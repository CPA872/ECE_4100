
#include <iostream>
#include <cstring>

#include "branchsim.h"
#include "gshare.h"
#include "perceptron.h"
#include "yeh_patt.h"

using namespace std;

// Don't modify this line -- Its to make the compiler happy
branch_predictor_base::~branch_predictor_base() {}


// ******* Student Code starts here *******

uint64_t PIPELINE_STAGES = 0;

/**
 * Extract bit a to bit b of an address
 * @param addr Address to extract
 * @param range Bit range: pair<int, int>(high, low) to extract
 * @return Extracted section
 */
uint64_t bit_extract(uint64_t addr, pair<int, int> range)
{
    int high = range.first;
    int low  = range.second;

    uint64_t mask = high == 63 ? 0xFFFFFFFFFFFFFFFF : (1ULL << (high + 1)) - 1;
    mask = mask - ((1ULL << low) - 1);
    return ((mask & addr) >> low);
}

/**
 * Function for updating smith counters
 * @param counter       smith counter to be updated
 * @param branch_taken  whether branch is taken
 * @return              updated smith counter
 */
uint8_t smith_counter_update(uint8_t counter, bool branch_taken) {
    uint8_t result = 0;
    if (branch_taken) {
        result = (counter < 3) ? static_cast<uint8_t>(counter + 1) : 3;
    } else {
        result = (counter > 0) ? static_cast<uint8_t>(counter - 1) : 0;
    }
    return result;
}

/**
 * Function for updating shift register
 * @param reg           shift register to be updated
 * @param branch_taken  whether branch is taken
 * @param length        length of shift register
 * @return              shifted history register
 */
uint64_t shift_register_update(uint64_t reg, bool branch_taken, uint64_t length) {
    reg = static_cast<uint64_t>(reg << 1);
    reg = branch_taken ? (reg | 1ULL) : (reg | 0ULL);
    reg = reg & static_cast<uint64_t>((1 << length) - 1);
    return reg;
}


// Gshare Branch Predictor

void gshare::init_predictor(branchsim_conf *sim_conf)
{
    PIPELINE_STAGES = sim_conf->N;

    this->GHR = 0;
    this->pattern_table = new uint8_t[1 << sim_conf->P];
    this->pc_range      = make_pair<>(2 + sim_conf->G - 1, 2);
    this->tag_range     = make_pair(63, sim_conf->G + 2);
    this->width         = sim_conf->G;  // history register width

    for (uint64_t i = 0; i < (1ULL << sim_conf->P); i++) {
        this->pattern_table[i] = 1;
    }
}

bool gshare::predict(branch *branch, branchsim_stats *sim_stats)
{
    return NOT_TAKEN;
}

void gshare::update_predictor(branch *branch)
{

}

gshare::~gshare()
{

}


// Yeh-Patt Branch Predictor
void yeh_patt::init_predictor(branchsim_conf *sim_conf)
{
    PIPELINE_STAGES = sim_conf->N;

    this->history_table = new uint64_t[1 << sim_conf->G];
    this->pattern_table = new uint8_t[1 << sim_conf->P];
    this->pc_range      = make_pair<>(2 + sim_conf->G - 1, 2);
    this->tag_range     = make_pair(63, sim_conf->G + 2);
    this->width         = sim_conf->P;

    for (uint64_t i = 0; i < (1ULL << sim_conf->G); i++) {
        this->history_table[i] = 0;
    }

    for (uint64_t i = 0; i < (1ULL << sim_conf->P); i++) {
        this->pattern_table[i] = 1;
    }
}

bool yeh_patt::predict(branch *branch, branchsim_stats *sim_stats)
{
    sim_stats->num_branch_instructions += 1;

    uint64_t  ht_index         = bit_extract(branch->ip, this->pc_range);
    uint64_t* history_register = &this->history_table[ht_index];
    uint8_t*  smith_counter    = &this->pattern_table[*history_register];

    bool prediction = *smith_counter >= 2;

    if (prediction == branch->is_taken) {
        sim_stats->num_branches_correctly_predicted += 1;
    } else {
        sim_stats->num_branches_miss_predicted += 1;
    }

    return prediction;
}

void yeh_patt::update_predictor(branch *branch)
{
    uint64_t  ht_index         = bit_extract(branch->ip, this->pc_range);
    uint64_t* history_register = &this->history_table[ht_index];
    uint8_t*  smith_counter    = &this->pattern_table[*history_register];

    *history_register = shift_register_update(*history_register, branch->is_taken, this->width);
    *smith_counter    = smith_counter_update(*smith_counter, branch->is_taken);
}

yeh_patt::~yeh_patt()
{
    delete [] history_table;
    delete [] pattern_table;
}


// Perceptron Branch Predictor

void perceptron_predictor::init_predictor(branchsim_conf *sim_conf)
{

}

bool perceptron_predictor::predict(branch *branch, branchsim_stats *sim_stats)
{
    return NOT_TAKEN;
}

void perceptron_predictor::update_predictor(branch *branch)
{

}

perceptron_predictor::~perceptron_predictor()
{

}


// Common Functions to update statistics and final computations, etc.

/**
 *  Function to update the branchsim stats per prediction
 *
 *  @param perdiction The prediction returned from the predictor's predict function
 *  @param branch Pointer to the branch that is being predicted -- contains actual behavior
 *  @param stats Pointer to the simulation statistics -- update in this function
*/
void branchsim_update_stats(bool prediction, branch *branch, branchsim_stats *sim_stats)
{
    sim_stats->total_instructions               = branch->inst_num;
    sim_stats->misses_per_kilo_instructions     = sim_stats->num_branches_miss_predicted / sim_stats->total_instructions * 1000;
    sim_stats->fraction_branch_instructions     = static_cast<double>(sim_stats->num_branch_instructions) / static_cast<double>(sim_stats->total_instructions);
    sim_stats->prediction_accuracy              = static_cast<double>(sim_stats->num_branches_correctly_predicted) / static_cast<double>(sim_stats->num_branch_instructions);
    sim_stats->stalls_per_miss_predicted_branch = (PIPELINE_STAGES <= 7) ? 2 : PIPELINE_STAGES / 2 - 1;
    sim_stats->average_CPI                      = 1 + static_cast<double>(sim_stats->stalls_per_miss_predicted_branch) * sim_stats->fraction_branch_instructions * (1 - sim_stats->prediction_accuracy);
}

/**
 *  Function to cleanup branchsim statistic computations such as prediction rate, etc.
 *
 *  @param stats Pointer to the simulation statistics -- update in this function
*/
void branchsim_cleanup_stats(branchsim_stats *sim_stats)
{

}
