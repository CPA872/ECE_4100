#include <cstdio>
#include <cinttypes>
#include <iostream>
#include <deque>
#include <vector>
#include <assert.h>
#include "procsim.h"

using namespace std;

uint64_t GLOBAL_CLOCK     = 0;
uint64_t INSTRUCTION_ID   = 1;

bool RAISE_EXCEPTION      = false;
bool INTERRUPT_RESOLVED   = false;
bool ALL_DONE             = false;
//bool DEBUG_TRACE          = true;
bool DEBUG_TRACE          = false;

int SCHEDULING_QUEUE_SIZE = 0;

uint64_t    CONF_F;
uint64_t    CONF_P;
uint64_t    CONF_J;
uint64_t    CONF_K;
uint64_t    CONF_L;
uint64_t    CONF_R;
uint64_t    CONF_I;

deque<instruction*> DispQ;
deque<SQ_entry>     SchedQ;
int                 Scoreboard[5];
int*                RAT;
RF_entry*           RF;
deque<ROB_entry>    ROB;

vector<instruction*> instruction_holder;

bool instruction_empty = false;

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
    CONF_F = conf->F;  // F – Fetch rate (instructions per cycle)
    CONF_P = conf->P;  // P – Number of PRegs
    CONF_J = conf->J;  // J – Number of k0 function units # FU_ALU Units (Used for BRANCH as well)
    CONF_K = conf->K;  // K – Number of k1 function units # FU_MUL Units
    CONF_L = conf->L;  // L – Number of k2 function units # LOAD / STORE instruction
    CONF_R = conf->R;  // R – Number of ROB entries
    CONF_I = conf->I;  // I – Interrupt Interval (cycles)

    SCHEDULING_QUEUE_SIZE = 2 * (conf->J + conf->K + conf->L);

    // ---- initialize Scoreboard ----
    Scoreboard[0] = CONF_J;  // FU_ALU
    Scoreboard[1] = CONF_K;  // FU_MUL
    Scoreboard[2] = CONF_L;  // FU_LW
    Scoreboard[3] = 1;       // FU_ST
    Scoreboard[4] = INT32_MAX; // for NOT using a function unit

    // ---- initialize RAT ----
    RAT = new int[32];
    for (int i = 0; i < 32; i++) {
        RAT[i] = i;
    }

    // ---- initialize RF ----
    RF = new RF_entry[conf->P];
    for (int i = 0; i < conf->P; i++) {
        RF[i].ready = true;
        RF[i].ready_cycle = -1;
        if (i < 32)
            RF[i].free = false;  // architectural registers, always NOT free
        else
            RF[i].free = true;
    }
}


/*
 * Checks if an available register exists in register file
 * Returns the lowest numbered dest_preg, else -1
*/
int check_RF() {
    for (int i = 0; i < CONF_P; i++) {
        if (RF[i].free) {
            return i;
        }
    }
    return -1;
}

/*
 * Prints relevant information of an instruction
*/
void print_info(instruction* instruction) {
    if (instruction->opcode == OP_INT) {
        printf("\nINT ");
    } else {
        printf("\n%lu ", instruction->id);
    }
    printf("%lu %lu %lu %lu %lu", instruction->fetch_cycle,
           instruction->dispatch_cycle, instruction->schedule_cycle,
           instruction->execute_cycle, instruction->state_update_cycle);
}

string get_inst_string(int type)
{
    if (type == OP_NOP)     return " NOP ";
    if (type == OP_ADD)     return " ADD ";
    if (type == OP_MUL)     return " MUL ";
    if (type == OP_LOAD)    return " LD ";
    if (type == OP_STORE)   return " ST ";
    if (type == OP_BR)      return " BR ";
    if (type == OP_INT)     return " INT ";

    return std::string();
}

/*
Fetch:
    1. The fetch unit fetches up to F instructions from the trace into empty slots in the dispatch queue.
    Note that the dispatch queue is infinite in size. We assume that the frontend Fetch unit never misses
    in the Instruction Cache (which we don’t model).

    2. For simplicity, we assume that the branch predictor has a 100 percent accuracy and
    hence do not perform any explicit branch prediction. Branch instructions are scheduled like any
    other instruction.
*/
static void fetch(procsim_stats *stats)
{
    // Sample for how instructions are read from the trace
    for (int i = 0; i < CONF_F; i++) {
        instruction *new_inst = new instruction;
        instruction_empty = !read_instruction(new_inst);  // [TEMP LOGIC]  if end of trace, ALL_DONE will be asserted high
        instruction_holder.push_back(new_inst);

        if (instruction_empty) { // stopping at 100
            return;
        }

        new_inst->id = INSTRUCTION_ID;
        INSTRUCTION_ID += 1;

        // ---- Configuring newly fetched instruction ----
        new_inst->fetch_cycle = GLOBAL_CLOCK;
        DispQ.push_back(new_inst);  // Add the new instruction to the dispatch queue

//        if (DEBUG_TRACE) cout << "<" << GLOBAL_CLOCK << ">\tinst[" << new_inst->id << "]\t" << get_inst_string(new_inst->opcode) << "\tfetched\n";
    }
}

/*
Dispatch:
    1. The dispatcher attempts to dispatch as many instructions as it can from the dispatch queue into
    empty slots in the scheduling queue/reservation stations, in program (trace) order, every cycle.
    When there are no more slots in the scheduling queue/reservation stations, it stalls.

    2. The source and destination register numbers are checked and the Physical Register File (PRF) is
    accessed along with the Register Alias Table (RAT) and Reorder Buffer (ROB). (See lecture
    notes for details)

    3. The lowest numbered free Preg is assigned as the destination register for an instruction whose
    destination register is not equal to -1. This is recorded in the RAT. The dispatch unit stalls if it
    is unable to assign a PReg to an instruction requiring one.4. A ROB entry is allocated for each instruction
    that is being dispatched. Each ROB entry must store the Areg number and the previously mapped Preg for that Areg.

    5. NOP instructions are never scheduled. This means if you encounter a NOP instruction at the
    head of the dispatch queue, you can ignore it and move on to servicing the next instruction.

    6. The First 32 physical registers of the PReg File are reserved as architectural registers (free bit is
    always false). All other physical registers can be free’ed and assigned as destination Pregs as
    required.
*/
static void dispatch(procsim_stats *stats)
{
    // dispatch until DispQ empty, ROB full, SchedQ full, or no more free_preg
    while (SchedQ.size() < SCHEDULING_QUEUE_SIZE  && ROB.size() < CONF_R && !DispQ.empty())
    // Clear to dispatch, OK to dispatch if no free preg but instruction doesn't need one
    {
        instruction* inst = DispQ.front();

        if (RAISE_EXCEPTION && inst->opcode != OP_INT) {  // stall dispatch upon exception
            return;
        }

        if (inst->dest_reg != -1 && check_RF() == -1) {  // instruction doesn't need writing to any register
            break;
        }

        inst->dispatch_cycle = GLOBAL_CLOCK;
        DispQ.pop_front();

        if (inst->opcode == OP_NOP) {  // discard NOP instructions
            continue;
        }

        // Allocate a new Scheduling Queue entry and a new ROB entry
        SQ_entry new_RS;
        ROB_entry rob_entry;

        int free_preg = check_RF();

//      Acquire Preg for source registers first, then assign new Preg for destnation register
//      Assign the reservation station's source registers based on Register Alias Table, or -1 (unused)
        for (int i = 0; i < 2; i++) {
            if (inst->src_reg[i] != -1) {
                new_RS.src_preg[i] = RAT[inst->src_reg[i]];
            } else {
                new_RS.src_preg[i] = -1;
            }
        }

        for (int i = 0; i < 2; ++i) {  // loop through all source registers
            if (inst->src_reg[i] == -1) {  // Doesn't need register, continue and set ready to true
                new_RS.src_ready[i] = true;
                continue;
            } else if (RF[RAT[inst->src_reg[i]]].ready) {
                new_RS.src_ready[i] = true;
            } else {
                new_RS.src_ready[i] = false;
            }
        }

//      Allocate a free PREG to the destination register of dispatching instruction
        if (inst->dest_reg != -1) { // If instruction Writes to anything
            new_RS.dest_preg    = free_preg;
            rob_entry.prev_preg = RAT[inst->dest_reg];
            rob_entry.dest_preg = free_preg;

            RAT[inst->dest_reg] = free_preg;
            RF[free_preg].free  = false;
            RF[free_preg].ready = false;
        } else {
            rob_entry.prev_preg = -1;
            rob_entry.dest_preg = -1;
            new_RS.dest_preg    = -1; // Else indicate this instruction will not write to register
        }

        new_RS.inst        = inst;     // SchedQ keeps track of the instruction
        new_RS.inst_id     = inst->id;
        new_RS.fired       = false;
        new_RS.completed   = false;

        // -------- Allocate a ROB entry -------
        rob_entry.ready     = false;
        rob_entry.inst      = inst;
        rob_entry.inst_id   = inst->id;

        switch (inst->opcode) {
            case OP_NOP:    // 1  NOP
                new_RS.function_unit = FU_NONE;
                new_RS.cycles_remaining = 0;
                break;

            case OP_ADD:    // 2  ADD
                new_RS.function_unit = FU_ALU;
                new_RS.cycles_remaining = 1;
                break;

            case OP_MUL:    // 3  FU_MUL
                new_RS.function_unit = FU_MUL;
                new_RS.cycles_remaining = 3;
                break;

            case OP_LOAD:   // 4  LOAD
                new_RS.function_unit = FU_LW;
                stats->load_instructions += 1;
                new_RS.cycles_remaining = 2;
                break;

            case OP_STORE:  // 5  STORE
                new_RS.function_unit = FU_ST;   // uses load-store queue, finishes in 1 cycle
                stats->store_instructions += 1;
                new_RS.cycles_remaining = 1;
                break;

            case OP_BR:     // 6  BRANCH
                new_RS.function_unit = FU_ALU;
                stats->branch_instructions += 1;
                new_RS.cycles_remaining = 1;
                break;

            case OP_INT:
                new_RS.function_unit = FU_ALU;
                new_RS.cycles_remaining = 1;
                break;

            default:
                break;
        }

        SchedQ.push_back(new_RS);
        ROB.push_back(rob_entry);

        if (DEBUG_TRACE) cout << "<" << GLOBAL_CLOCK << ">\tinst[" << inst->id << "]\t" << get_inst_string(inst->opcode) << "\tDispatched\n";

        assert(!(new_RS.dest_preg != -1 && (new_RS.dest_preg == new_RS.src_preg[0] || new_RS.dest_preg == new_RS.src_preg[1])));
    } // end while
}

/*
Schedule:
    1. There are 2*(J + K + L) entries in the scheduling unit (schedule queue).

    2. If there are multiple independent instructions ready to fire during the same cycle in a reservation
    station, service them in program order, and based on the availability of function units.

    3. A fired instruction remains in the reservation station until it completes. The latency of each unit
    type is listed below

    4. The rules for firing follow the Tomasulo with Preg approach (Please look at class notes for
    details): Loads and Stores are handled as per the rules described in a later section.

    5. The FU_MUL and Load Function units are pipelined

    6. Only one store instruction (oldest, i.e., Earliest in program order) can be fired. Number of store
    unit is always 1.
*/
static void schedule(procsim_stats *stats)
{
    for (auto RS = SchedQ.begin(); RS != SchedQ.end(); RS++)
    {
        for (int i = 0; i < 2; i++) {  // update state (ready) of source registers
            if (RS->src_preg[i] != -1) {
                if (GLOBAL_CLOCK > RF[RS->src_preg[i]].ready_cycle) {
                    RS->src_ready[i] = RF[RS->src_preg[i]].ready;
                }
            } else {
                RS->src_ready[i] = true;
            }
        }

        // Both source registers and function units are ready, FEUER!
        if (RS->src_ready[0] && RS->src_ready[1] && Scoreboard[RS->function_unit] > 0 && !RS->fired)
        {
            if (!RS->fired) {  // only record schedule cycle when we fire the instruction
                RS->inst->schedule_cycle = GLOBAL_CLOCK;
            }

            RS->fired = true;         // Indicates this instruction is fired / starts executing

            if (RS->function_unit != FU_NONE) {      // decrement the availability of that function unit
                Scoreboard[RS->function_unit] -= 1;
            }
            if (DEBUG_TRACE) cout << "<" << GLOBAL_CLOCK << ">\tinst[" << RS->inst->id << "]\t" << get_inst_string(RS->inst->opcode) << "\tscheduled, cycles=" << RS->cycles_remaining << endl;
        }
    }
}


/*
Execute:
    1. The function units are present in this stage. When an instruction completes, it updates the
    reservation stations, the PREG, and the ROB. Those updates are visible in the following cycle.
    An instruction is considered “completed” after it spends the required cycles in execute.

    2. The ROB and PRF are accessed in this stage of the pipeline
*/
static void execute(procsim_stats *stats)
{
    for (auto RS = SchedQ.begin(); RS != SchedQ.end(); ) {

        if (RS->fired)  // looking for instructions that are fired
        {
            RS->cycles_remaining -= 1;
            assert(RS->inst->opcode != OP_NOP);
            assert(RS->cycles_remaining >= -1);

            // For LW and MUL pipelines, we set their function units to free after completing first stage
            if (RS->function_unit == FU_LW && RS->cycles_remaining == 1) {
                Scoreboard[RS->function_unit] += 1;
            }
            if (RS->function_unit == FU_MUL && RS->cycles_remaining == 2) {
                Scoreboard[RS->function_unit] += 1;
            }

            if (RS->cycles_remaining <= 0) // This Reservation Station completes execution
            {
                RS->inst->execute_cycle = GLOBAL_CLOCK;

                if (RS->function_unit != FU_LW && RS->function_unit != FU_MUL) {
                    Scoreboard[RS->function_unit] += 1;             // clears the function unit
                }


                if (RS->dest_preg != -1) {
                    RF[RS->dest_preg].ready = true;          // set register to ready in register file
                    RF[RS->dest_preg].ready_cycle = GLOBAL_CLOCK;
                }

                // scans entire ROB to update the ROB entry assigned to that instruction
                for (auto rob_entry = ROB.rbegin(); rob_entry != ROB.rend(); rob_entry++) {
                    if (rob_entry->inst == RS->inst) {
                        rob_entry->ready = true;  // Instruction in ROB sets to finished
                        break;
                    }
                }

//                RS = SchedQ.erase(RS);  // Removes this instruction from scheduling queue
                RS->completed = true;
                RS++;

                if (DEBUG_TRACE) cout << "<" << GLOBAL_CLOCK << ">\tinst[" << RS->inst_id << "]\t" << get_inst_string(RS->inst->opcode) << "\tfinished execution\n";

            } else {
                RS++;
            }

        } else {
            RS++;
        }
    }
}


/*
State Update:
    1. This stage performs in-order retirement from the reorder buffer. It checks if the oldest entry in
    the ROB is ready, and if so, the instruction is retired, freeing up the previous PReg.

    2. The result is written to the Areg in the ROB. This unit can retire as many instructions as possible
    until the head of the ROB is an instruction that has not completed yet.
*/
static void state_update(procsim_stats *stats)
{
    // retire as many instruction as possible, until hitting one that is not ready
    int inst_retired = 0;
    for (auto curr = ROB.begin(); curr != ROB.end(); ) {  // forward traverse, from head (oldest) to tail (earliest)
        if (curr->ready)
        {
            curr->inst->state_update_cycle = GLOBAL_CLOCK;
            stats->instructions_retired   += 1;

            if (curr->dest_preg != -1) {
                if (curr->prev_preg > 31) {
                    RF[curr->prev_preg].free = true;
                }
            }

//            print_info(curr->inst);

            if (curr->inst->opcode == OP_INT) {
                RAISE_EXCEPTION = false;
            }

            if (DEBUG_TRACE) cout << "<" << GLOBAL_CLOCK << ">\tinst[" << curr->inst_id << "]\t" << get_inst_string(curr->inst->opcode) << "\tretired\n";
            curr = ROB.erase(curr);
            inst_retired += 1;
        } else {
            curr++;
            break;
        }
    }

    for (auto RS = SchedQ.begin(); RS != SchedQ.end(); ) {
        if (RS->completed) {
            RS = SchedQ.erase(RS);
        } else {
            RS++;
        }
    }


    stats->average_ipc += inst_retired;
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

        stats->average_disp_queue_size += DispQ.size();
        if (DispQ.size() > stats->max_disp_queue_size) {
            stats->max_disp_queue_size = DispQ.size();
        }

        GLOBAL_CLOCK++; // Clock the processor

        // Raise an exception/interrupt every 'I' clock cycles
        // When the RAISE_EXCEPTION FLAG is raised -- an Interrupt instruction is added
        // to the front of the dispatch queue while the schedule queue and ROB are flushed
        // Execution resumes starting from the Interrupt Instruction. The flushed instructions
        // are re-executed
        if ( (GLOBAL_CLOCK % conf->I) == 0 ) {
            stats->num_exceptions += 1;
            RAISE_EXCEPTION = true;

            // push in the interrupt instruction
            instruction* interrupt = new instruction;
            interrupt->inst_addr   = 0;            // instruction address
            interrupt->opcode      = OP_INT;
            interrupt->src_reg[0]  = -1;           // Source registers
            interrupt->src_reg[1]  = -1;           // Source registers
            interrupt->dest_reg    = -1;           // Destination register
            interrupt->ld_st_addr  = 0;            // Effective address of Load or Store instruction
            interrupt->br_target   = 0;            // branch target if the instruction is a branch
            interrupt->br_taken    = false;
            interrupt->id          = 9999999999;
            interrupt->fetch_cycle = 0;
            DispQ.push_front(interrupt);
            instruction_holder.push_back(interrupt);
        }

        // Run the loop until all the instructions in the trace have retired
        // Feel free to replace the condition of the do-while loop
    } while (!(instruction_empty && DispQ.empty() && SchedQ.empty() && ROB.empty()));

    stats->cycle_count = GLOBAL_CLOCK;
}

/**
 * Subroutine for cleaning up any outstanding instructions and calculating overall statistics
 * such as average IPC, average fire rate etc.
 *
 * @param stats Pointer to the statistics structure
 */
void complete_proc(procsim_stats *stats)
{
    for (instruction* inst : instruction_holder)
        delete inst;
    delete [] RAT;
    delete [] RF;

    stats->average_ipc = stats->average_ipc / stats->cycle_count;
    stats->average_disp_queue_size = stats->average_disp_queue_size / stats->cycle_count;
}