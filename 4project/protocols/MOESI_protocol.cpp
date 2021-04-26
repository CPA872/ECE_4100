#include "MOESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOESI_protocol::MOESI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
    this->state = MOESI_CACHE_I;
}

MOESI_protocol::~MOESI_protocol ()
{    
}

void MOESI_protocol::dump (void)
{
    const char *block_states[6] = {"X","I","S","E","O","M"};
    fprintf (stderr, "MOESI_protocol - state: %s\n", block_states[state]);
}

void MOESI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
    case MOESI_CACHE_I:     do_cache_I(request);    break;
    case MOESI_CACHE_S:     do_cache_S(request);    break;
    case MOESI_CACHE_E:     do_cache_E(request);    break;
    case MOESI_CACHE_O:     do_cache_O(request);    break;
    case MOESI_CACHE_M:     do_cache_M(request);    break;
    case MOESI_CACHE_ISE:   do_cache_ISE(request);  break;
    case MOESI_CACHE_IM:    do_cache_IM(request);   break;
    case MOESI_CACHE_OM:    do_cache_OM(request);   break;
    case MOESI_CACHE_SM:    do_cache_SM(request);   break;

    default:
        fatal_error ("Invalid Cache State for MOESI Protocol\n");
    }
}

void MOESI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
    case MOESI_CACHE_I:     do_snoop_I(request);    break;
    case MOESI_CACHE_S:     do_snoop_S(request);    break;
    case MOESI_CACHE_E:     do_snoop_E(request);    break;
    case MOESI_CACHE_O:     do_snoop_O(request);    break;
    case MOESI_CACHE_M:     do_snoop_M(request);    break;
    case MOESI_CACHE_ISE:   do_snoop_ISE(request);  break;
    case MOESI_CACHE_IM:    do_snoop_IM(request);   break;
    case MOESI_CACHE_OM:    do_snoop_OM(request);   break;
    case MOESI_CACHE_SM:    do_snoop_SM(request);   break;
    default:
    	fatal_error ("Invalid Cache State for MOESI Protocol\n");
    }
}

inline void MOESI_protocol::do_cache_I(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_I(Mreq *request) {

}

inline void MOESI_protocol::do_cache_S(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_S(Mreq *request) {

}

inline void MOESI_protocol::do_cache_E(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_E(Mreq *request) {

}

inline void MOESI_protocol::do_cache_O(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_O(Mreq *request) {

}

inline void MOESI_protocol::do_cache_M(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_M(Mreq *request) {

}

inline void MOESI_protocol::do_cache_ISE(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_ISE(Mreq *request) {

}

inline void MOESI_protocol::do_cache_IM(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_IM(Mreq *request) {

}

inline void MOESI_protocol::do_cache_OM(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_OM(Mreq *request) {

}

inline void MOESI_protocol::do_cache_SM(Mreq *request) {

}

inline void MOESI_protocol::do_snoop_SM(Mreq *request) {

}
