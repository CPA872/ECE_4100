#include "MESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MESI_protocol::MESI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
    this->state = MESI_CACHE_I;
}

MESI_protocol::~MESI_protocol ()
{    
}

void MESI_protocol::dump (void)
{
    const char *block_states[5] = {"X","I","S","E","M"};
    fprintf (stderr, "MESI_protocol - state: %s\n", block_states[state]);
}

void MESI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
    case MESI_CACHE_I:      do_cache_I(request);       break;
    case MESI_CACHE_S:      do_cache_S(request);       break;
    case MESI_CACHE_E:      do_cache_E(request);       break;
    case MESI_CACHE_M:      do_cache_M(request);       break;
    case MESI_CACHE_ISE:    do_cache_ISE(request);     break;
    case MESI_CACHE_IM:     do_cache_IM(request);      break;
    case MESI_CACHE_SM:     do_cache_SM(request);      break;
    default           :     fatal_error ("Invalid Cache State for MESI Protocol\n");
    }
}

void MESI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
    case MESI_CACHE_I:      do_snoop_I(request);       break;
    case MESI_CACHE_S:      do_snoop_S(request);       break;
    case MESI_CACHE_E:      do_snoop_E(request);       break;
    case MESI_CACHE_M:      do_snoop_M(request);       break;
    case MESI_CACHE_ISE:    do_snoop_ISE(request);     break;
    case MESI_CACHE_IM:     do_snoop_IM(request);      break;
    case MESI_CACHE_SM:     do_snoop_SM(request);      break;
    default:                fatal_error ("Invalid Cache State for MESI Protocol\n");
    }
}

inline void MESI_protocol::do_cache_I (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        send_GETS(request->addr);
        this->state = MESI_CACHE_ISE;  // may change either to S or E
        Sim->cache_misses += 1;
        break;

    case STORE:
        send_GETM(request->addr);   // request to get data to modify 
        this->state = MESI_CACHE_IM;
        Sim->cache_misses += 1;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state CACHE shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_I (Mreq *request) {}  // inaction

inline void MESI_protocol::do_cache_ISE (Mreq *request) {}  // handled by snooper

inline void MESI_protocol::do_snoop_ISE (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;

    case DATA:
        send_DATA_to_proc(request->addr);

        // Depending on whether the line is shared, set to either S or E
        if (get_shared_line()) {  // if line indicates this block is shared
            this->state = MESI_CACHE_S;
        } else {
            this->state = MESI_CACHE_E;
        }
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: ISE state SNOOPER shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_IM (Mreq *request) {}  // handled by snooper

inline void MESI_protocol::do_snoop_IM (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;

    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MESI_CACHE_M;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IM state SNOOPER shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_S (Mreq *request)
{
    switch (request->msg) {
    case LOAD:  // Shared write-hit, send data to processor
        send_DATA_to_proc(request->addr);
        break;

    case STORE: // shared write-hit: broadcast to invalidate other copies
        send_GETM(request->addr);
        this->state = MESI_CACHE_SM;
        Sim->cache_misses += 1;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state CACHE shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_S (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        set_shared_line();
        break;

    case GETM:
        this->state = MESI_CACHE_I;
        break;

    case DATA:
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state SNOOPER shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_SM (Mreq *request)  // snooper handles this state
{
    switch (request->msg) {
    case LOAD:
    case STORE:
    case DATA:
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: SM state CACHE shouldn't see this message\n");
    }
}

// handles a S->M transition, listen for confirmation and change to state M
inline void MESI_protocol::do_snoop_SM (Mreq *request)  
{
    switch (request->msg) {
    case GETS:
        set_shared_line();
        break;

    case GETM:
        break;

    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MESI_CACHE_M;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: SM state SNOOPER shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_E (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        send_DATA_to_proc(request->addr);
        break;

    case STORE:  // silent_upgrade here: S->M without broadcast
        send_DATA_to_proc(request->addr);
        this->state = MESI_CACHE_M;
        Sim->silent_upgrades += 1;  
        break;

    case DATA:
        break;
        
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: E state CACHE shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_E (Mreq *request)
{
    switch (request->msg) {
    case GETS:  // discards exclusiveness upon other cores' requests
        send_DATA_on_bus(request->addr, request->src_mid);
        set_shared_line();
        this->state = MESI_CACHE_S;
        break;

    case GETM:
        send_DATA_on_bus(request->addr, request->src_mid);
        set_shared_line();
        this->state = MESI_CACHE_I;
        break;

    case DATA:
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: E state SNOOPER shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_M (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
    case STORE:  // both simply send data to core
        send_DATA_to_proc(request->addr);
        break;

    case DATA:
        break;
        
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: M state CACHE shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_M (Mreq *request)
{
    switch (request->msg) {
    case GETS:  // another core requesting READ, intervene, supply data on bus, write back to MEM
        send_DATA_on_bus(request->addr, request->src_mid);  
        set_shared_line();
        this->state = MESI_CACHE_S;
        break;

    case GETM:
        send_DATA_on_bus(request->addr, request->src_mid);  // another core requesting WRITE, invalidate own copy, writeback
        set_shared_line();
        this->state = MESI_CACHE_I;
        break;

    case DATA:
        break;   

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: M state SNOOPER shouldn't see this message\n");
    }
}















