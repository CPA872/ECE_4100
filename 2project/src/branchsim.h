#ifndef BRANCHSIM_H
#define BRANCHSIM_H

#include <cstdint>
#include <map>
#include <utility>

const bool TAKEN = true;
const bool NOT_TAKEN = false;

typedef struct branch_t {
    uint64_t ip; // ip = instruction pointer aka. address of the branch instruction
    uint64_t inst_num; // Instruction count of the branch

    bool is_taken;  // Is the branch actuall taken

    branch_t() { }

} branch;

// Structure to hold configuration details for a branchsim simulation run
typedef struct branchsim_conf_t {

    enum PREDICTOR {GSHARE = 1, YEH_PATT, PERCEPTRON};

    uint64_t G; // History Registers are G bits wide
    uint64_t P; // Pattern / Perceptron table addressing bits
    uint64_t N; // Number of stages in the pipeline

    enum PREDICTOR predictor;

    // Default constructor
    branchsim_conf_t() : G(10), P(10), N(5), predictor(PREDICTOR::GSHARE) {}

} branchsim_conf;

// static map to get the string for a predictor
static const std::map<branchsim_conf_t::PREDICTOR, const char *> pred_to_string = {
    {branchsim_conf::GSHARE, "Gshare"},
    {branchsim_conf::YEH_PATT, "Yeh-Patt"},
    {branchsim_conf::PERCEPTRON, "PERCEPTRON"}
};

// Structure to hold statistics for a branchsim simulation run
typedef struct branchsim_stats_t {
    uint64_t total_instructions;            // Use the inst num field of the final branch
    uint64_t num_branch_instructions;
    uint64_t num_branches_correctly_predicted;
    uint64_t num_branches_miss_predicted;
    uint64_t misses_per_kilo_instructions;  // Yes this is indeed being recorded as a uint64_t
                                            // Allows tracking various phases of program execution
                                            // and is a common metric in branch prediction papers

    uint64_t num_tag_conflicts;             // Number of tag conflicts in the first level
                                            // table of the branch predictor

    double fraction_branch_instructions;    // fraction of the entire program that is branches
    double prediction_accuracy;

    uint64_t N;                             // Number of stages in the pipeline
    uint64_t stalls_per_miss_predicted_branch; // set via equation in PDF

    double average_CPI;
} branchsim_stats;

// Base class for a branch predictor
class branch_predictor_base {

public:
    // Initialize the predictor state including any data structures you might need
    virtual void init_predictor(branchsim_conf *sim_conf) = 0;

    // Return the prediction ({taken/not-taken}, target-address)
    virtual bool predict(branch *branch, branchsim_stats *sim_stats) = 0;

    // Update the branch predictor state
    virtual void update_predictor(branch *branch) = 0;

    // Cleanup any initialized memory here
    virtual ~branch_predictor_base() = 0;
};


// Function to update branch prediction statistics after the prediction
void branchsim_update_stats(bool prediction, branch *branch, branchsim_stats *sim_stats);

// Function to perform cleanup and final statistics calculations, etc
void branchsim_cleanup_stats(branchsim_stats *sim_stats);

#endif