#ifndef GSHARE_H
#define GSHARE_H

#include "branchsim.h"

// Gshare predictor definition
class gshare : public branch_predictor_base
{

private:
    uint64_t GHR;  // shift registers
    uint8_t*  pattern_table;  // smith counters, initialized to 01
    uint64_t  width;
    std::pair<uint64_t, uint64_t> pc_range;
    std::pair<uint64_t, uint64_t> tag_range;

public:
    void init_predictor(branchsim_conf *sim_conf);
    
    // Return the prediction ({taken/not-taken}, target-address)
    bool predict(branch *branch, branchsim_stats *sim_stats);
    
    // Update the branch predictor state
    void update_predictor(branch *branch);

    // Cleanup any initialized memory here
    ~gshare();
};

#endif
