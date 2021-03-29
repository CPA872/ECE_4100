
#include <cstdio>
#include <cinttypes>
#include <iostream>

#include "procsim.h"

uint64_t GLOBAL_CLOCK = 0;
bool RAISE_EXCEPTION = false;
bool ALL_DONE = false;

// You may define any global variables here

// We suggest using std::deque for the dispatch queue since instructions are both added and removed
// from it. Using an std::vector is really inefficient for popping from the front


/**
 * Subroutine for initializing the processor. You many add and initialize any global or heap
 * variables as needed.
 *
 * @param conf Pointer to the configuration. Read only in this function
 */
void setup_proc(const procsim_conf *conf)
{
    GLOBAL_CLOCK = 0;
    RAISE_EXCEPTION = false;
    ALL_DONE = false;

    // Student code should go below this line

    // Note - The schedule queue can hold a maximum of 2 * (J + K + L) instructions

}


// Write helper methods for each superscalar operation here

static void fetch(procsim_stats *stats)
{
    // Sample for how instructions are read from the trace
    instruction *new_inst = new instruction;
    read_instruction(new_inst);

}

static void dispatch(procsim_stats *stats)
{

}

static void schedule(procsim_stats *stats)
{

}

static void execute(procsim_stats *stats)
{

}

static void state_update(procsim_stats *stats)
{

}


/**
 * Subroutine that simulates the processor. The processor should fetch instructions as
 * appropriate, until all instructions have executed
 *
 * @param stats Pointer to the statistics structure
 * @param conf Pointer to the configuration. Read only in this function
 */
void run_proc(procsim_stats *stats, const procsim_conf *conf)
{
    do {
        state_update(stats);
        execute(stats);
        schedule(stats);
        dispatch(stats);
        fetch(stats);

        GLOBAL_CLOCK++; // Clock the processor

        // Raise an exception/interrupt every 'I' clock cycles
        // When the RAISE_EXCEPTION FLAG is raised -- an Interrupt instruction is added
        // to the front of the dispatch queue while the schedule queue and ROB are flushed
        // Execution resumes starting from the Interrupt Instruction. The flushed instructions
        // are re-executed
        if ( (GLOBAL_CLOCK % conf->I) == 0 ) {
            RAISE_EXCEPTION = true;
        }

        // Run the loop until all the instructions in the trace have retired
        // Feel free to replace the condition of the do-while loop
    } while (!ALL_DONE);
}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC, average fire rate etc.
 *
 * @param stats Pointer to the statistics structure
 */
void complete_proc(procsim_stats *stats)
{

}
