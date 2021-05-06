#include "MSI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MSI_protocol::MSI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
    this->state = MSI_CACHE_I;  // initialize cache state to invalid at start
}

MSI_protocol::~MSI_protocol ()
{    

}

void MSI_protocol::dump (void)
{
    const char *block_states[4] = {"X","I","S","M"};
    fprintf (stderr, "MSI_protocol - state: %s\n", block_states[state]);
}

void MSI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
    case MSI_CACHE_I:   do_cache_I(request);    break;  // invalid state
    case MSI_CACHE_IM:  do_cache_IM(request);   break;  // transient state from I to M, waiting for DATA response
    case MSI_CACHE_IS:  do_cache_IS(request);   break;  // ^
    case MSI_CACHE_S:   do_cache_S(request);    break;  // Shared state
    case MSI_CACHE_SM:  do_cache_SM(request);   break;  // transient state from S to M, waiting for DATA response as confirmation
    case MSI_CACHE_M:   do_cache_M(request);    break;  // modified state
    default:            fatal_error ("Invalid Cache State for MSI Protocol\n");
    }
}

void MSI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
    case MSI_CACHE_I:   do_snoop_I(request);    break;
    case MSI_CACHE_IM:  do_snoop_IM(request);   break;
    case MSI_CACHE_IS:  do_snoop_IS(request);   break;
    case MSI_CACHE_S:   do_snoop_S(request);    break;
    case MSI_CACHE_SM:  do_snoop_SM(request);   break;
    case MSI_CACHE_M:   do_snoop_M(request);    break;
    default:            fatal_error ("Invalid Cache State for MSI Protocol\n");
    }
}

// invalid cache block behavior
inline void MSI_protocol::do_cache_I (Mreq *request)  
{
    switch (request->msg) {
    case LOAD:
        send_GETS(request->addr);   // request to get data on bus
        this->state = MSI_CACHE_IS; // set state to transient from I to S
        Sim->cache_misses += 1;     // misses ++
        break;

    case STORE:
        send_GETM(request->addr);   // request to get data to modify 
        this->state = MSI_CACHE_IM;
        Sim->cache_misses += 1;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_cache_IM (Mreq *request) 
{
    switch (request->msg) {
    case LOAD:
    case STORE:  
    case DATA:
        break;      // wait for snooper snooping DATA as response to previous request

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IM state shouldn't see this message\n");
    }
}  

inline void MSI_protocol::do_cache_IS (Mreq *request) 
{
    switch (request->msg) {
    case LOAD:
    case STORE:  
    case DATA:
        break;      // wait for snooper snooping DATA as response to previous request

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IS state shouldn't see this message\n");
    }
}  

inline void MSI_protocol::do_cache_S (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        send_DATA_to_proc(request->addr);  
        break;

    case STORE:
        send_GETM(request->addr);
        this->state = MSI_CACHE_SM;
        Sim->cache_misses += 1;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: S state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_cache_M (Mreq *request)  // same as MI_protocol::do_cache_M()
{
    switch (request->msg) {
    /* The M state means we have the data and we can modify it.  Therefore any request
     * from the processor (read or write) can be immediately satisfied.
     */
    case LOAD:
    case STORE:
    	// This is how you send data back to the processor to finish the request
    	// Note: There was no need to send anything on the bus on a hit.
    	send_DATA_to_proc(request->addr);
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: M state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_cache_SM (Mreq *request)  // SM state taken care of by snooper, not cache
{
    switch (request->msg) {
    case LOAD:
    case STORE:
    case DATA:
    	break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: M state shouldn't see this message\n");
    }
}  

inline void MSI_protocol::do_snoop_I (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
    case DATA:
        break;  // inaction for snooping in invalid state

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IS state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_snoop_S (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        break;

    case GETM:
        this->state = MSI_CACHE_I;  // invalidate own copy upon a GETM
        break;

    case DATA:
        break;     

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IM state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_snoop_M (Mreq *request)
{
    switch (request->msg) {
    case GETS:  // another core requesting READ, intervene, supply data on bus, write back to MEM
        send_DATA_on_bus(request->addr, request->src_mid);  
        this->state = MSI_CACHE_S;
        break;

    case GETM:
        send_DATA_on_bus(request->addr, request->src_mid);  // another core requesting WRITE, invalidate own copy, writeback
        this->state = MSI_CACHE_I;
        break;

    case DATA:
        break;   

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IM state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_snoop_IS (Mreq *request) 
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;   

    case DATA:
        send_DATA_to_proc(request->addr);   // send DATA from bus to core
        this->state = MSI_CACHE_S;          // Set cache block to S state;
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IS state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_snoop_IM (Mreq *request) 
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;      

    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MSI_CACHE_M;  
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: IM state shouldn't see this message\n");

    }
}

inline void MSI_protocol::do_snoop_SM (Mreq *request) 
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;      

    // Wait for 100 cycles to receive DATA from memory as confirmation that
    // other cores' copies are indeed INVALID -- Bobin Deng
    case DATA:
        send_DATA_to_proc(request->addr);
        this->state = MSI_CACHE_M;  
        break;

    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: SM state shouldn't see this message\n");
    }
}
