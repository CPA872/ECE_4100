#include "MOESIF_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOESIF_protocol::MOESIF_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
    this->state = MOESIF_CACHE_I;
}

MOESIF_protocol::~MOESIF_protocol ()
{    
}

void MOESIF_protocol::dump (void)
{
    const char *block_states[9] = {"X","I","S","E","O","M","F"};
    fprintf (stderr, "MOESIF_protocol - state: %s\n", block_states[state]);
}

void MOESIF_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
    case MOESIF_CACHE_I:    do_cache_I(request);        break;
    case MOESIF_CACHE_S:    do_cache_S(request);        break;
    case MOESIF_CACHE_E:    do_cache_E(request);        break;
    case MOESIF_CACHE_O:    do_cache_O(request);        break;
    case MOESIF_CACHE_M:    do_cache_M(request);        break;
    case MOESIF_CACHE_ISE:  do_cache_ISE(request);      break;
    case MOESIF_CACHE_IM:   do_cache_IM(request);       break;
    case MOESIF_CACHE_OM:   do_cache_OM(request);       break;
    case MOESIF_CACHE_SM:   do_cache_SM(request);       break;
    case MOESIF_CACHE_F:    do_cache_F(request);        break;
    default:                fatal_error ("Invalid Cache State for MOESIF Protocol\n");
    }
}

void MOESIF_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
    case MOESIF_CACHE_I:    do_snoop_I(request);        break;
    case MOESIF_CACHE_S:    do_snoop_S(request);        break;
    case MOESIF_CACHE_E:    do_snoop_E(request);        break;
    case MOESIF_CACHE_O:    do_snoop_O(request);        break;
    case MOESIF_CACHE_M:    do_snoop_M(request);        break;
    case MOESIF_CACHE_ISE:  do_snoop_ISE(request);      break;
    case MOESIF_CACHE_IM:   do_snoop_IM(request);       break;
    case MOESIF_CACHE_OM:   do_snoop_OM(request);       break;
    case MOESIF_CACHE_SM:   do_snoop_SM(request);       break;
    case MOESIF_CACHE_F:    do_snoop_F(request);        break;
    default:                fatal_error ("Invalid SNOOPER State for MOESIF Protocol\n");
    }
}

inline void MOESIF_protocol::do_cache_I(Mreq *request) {
    switch (request->msg) {
    case LOAD:
        send_GETS(request->addr);
        this->state = MOESIF_CACHE_ISE;  
        Sim->cache_misses += 1;
        break;

    case STORE:
        send_GETM(request->addr);   // request to get data to modify 
        this->state = MOESIF_CACHE_IM;
        Sim->cache_misses += 1;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state CACHE shouldn't see this message\n");
        break;
    }
}

inline void MOESIF_protocol::do_snoop_I(Mreq *request) {

}

inline void MOESIF_protocol::do_cache_S(Mreq *request) {
    switch (request->msg) {
    case LOAD:
        send_DATA_to_proc(request->addr);
        break;

    case STORE:
        send_GETM(request->addr);
        this->state = MOESIF_CACHE_SM;  // redundant use of state SM, same as IM, will rename later
        Sim->cache_misses += 1;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state CACHE shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_S(Mreq *request) {
    switch (request->msg) {
    case GETS:
        set_shared_line();
        break;

    case GETM:
        set_shared_line();
        // send_DATA_on_bus(request->addr, request->src_mid);
        this->state = MOESIF_CACHE_I;
        break;

    case DATA:
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_E(Mreq *request) {
    switch (request->msg) {
    case LOAD:
        send_DATA_to_proc(request->addr);
        break;

    case STORE:  // silent_upgrade here: S->M without broadcast
        send_DATA_to_proc(request->addr);
        this->state = MOESIF_CACHE_M;
        Sim->silent_upgrades += 1;  
        break;

    case DATA:
        break;
        
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: E state CACHE shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_E(Mreq *request) {
    switch (request->msg) {
    case GETS:  // discards exclusiveness upon other cores' requests
        send_DATA_on_bus(request->addr, request->src_mid);
        set_shared_line();
        this->state = MOESIF_CACHE_S;
        break;

    case GETM:
        send_DATA_on_bus(request->addr, request->src_mid);
        set_shared_line();
        this->state = MOESIF_CACHE_I;
        break;

    case DATA:
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: E state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_O(Mreq *request) {
    switch (request->msg) {
    case LOAD:
        send_DATA_to_proc(request->addr);
        break;

    case STORE:
        this->state = MOESIF_CACHE_OM;
        Sim->cache_misses += 1;
        send_GETM(request->addr);
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: O state CACHE shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_O(Mreq *request) {
    switch (request->msg) {
    case GETS:
        set_shared_line();
        send_DATA_on_bus(request->addr, request->src_mid);
        break;

    case GETM:
        set_shared_line();
        send_DATA_on_bus(request->addr, request->src_mid);
        this->state = MOESIF_CACHE_I;
        break;

    case DATA:
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: O state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_M(Mreq *request) {
    switch (request->msg) {
    case LOAD:
    case STORE:
        send_DATA_to_proc(request->addr);
        break;
        
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: M state CACHE shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_M(Mreq *request) {
    switch (request->msg) {
    case GETS:
        set_shared_line();
        send_DATA_on_bus(request->addr, request->src_mid);
        this->state = MOESIF_CACHE_O;
        break;

    case GETM:
        set_shared_line();
        send_DATA_on_bus(request->addr, request->src_mid);
        this->state = MOESIF_CACHE_I;
        break;

    case DATA:
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: M state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_ISE(Mreq *request) { }

inline void MOESIF_protocol::do_snoop_ISE(Mreq *request) {
    switch (request->msg) {
    case GETS:
    case GETM:
        break;

    case DATA:
        send_DATA_to_proc(request->addr);

        // Depending on whether the line is shared, set to either S or E
        if (get_shared_line()) {  // if line indicates this block is shared
            this->state = MOESIF_CACHE_S;
        } else {
            this->state = MOESIF_CACHE_E;
        }
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: ISE state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_IM(Mreq *request) { }

inline void MOESIF_protocol::do_snoop_IM(Mreq *request) {
    switch (request->msg) {
    case GETS:
    case GETM:
        break;

    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MOESIF_CACHE_M;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IM state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_OM(Mreq *request) { }

inline void MOESIF_protocol::do_snoop_OM(Mreq *request) {
    switch (request->msg) {
    case GETS:
    // if unique copy, can serve other's READ request many times
        if (!get_shared_line()) {  
            send_DATA_on_bus(request->addr, request->src_mid);
            set_shared_line();
        }
        break;

    case GETM:
    // if unique copy, can serve other's WRITE request but only once,
    // then enter IM state to complete O->M transition
        if (!get_shared_line()) {  
            send_DATA_on_bus(request->addr, request->src_mid);
            set_shared_line();
            this->state = MOESIF_CACHE_IM;
        }
        break;

    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MOESIF_CACHE_M;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: OM state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_SM(Mreq *request) { }

inline void MOESIF_protocol::do_snoop_SM(Mreq *request) {
    switch (request->msg) {
    case GETS:
        set_shared_line();
        break;
        
    case GETM:
        break;

    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MOESIF_CACHE_M;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: SM state SNOOPER shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_F(Mreq *request) {
    switch (request->msg) {
    case LOAD:  
        send_DATA_to_proc(request->addr);
        break;

    case STORE:
        send_DATA_to_proc(request->addr);
        break;
        
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: M state CACHE shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_F(Mreq *request) {
    switch (request->msg) {
    case GETS:
        set_shared_line();
        send_DATA_on_bus(request->addr, request->src_mid);
        break;
        
    case GETM:
        set_shared_line();
        send_DATA_on_bus(request->addr, request->src_mid);
        this->state = MOESIF_CACHE_I;
        break;

    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MOESIF_CACHE_M;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: SM state SNOOPER shouldn't see this message\n");
    }
}

