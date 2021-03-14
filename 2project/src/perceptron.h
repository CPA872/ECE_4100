#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include "branchsim.h"
#include <vector>

// Perceptron predictor definition
class perceptron_predictor : public branch_predictor_base
{
private:
    uint64_t GHR;
    uint64_t G;
    uint64_t P;
    double theta;
    bool current_prediction;
    int current_y;

    int** perceptron_table;
    uint64_t* tag_store;
    std::pair<int, int> pc_range;
    std::pair<int, int> tag_range;


public:
    void init_predictor(branchsim_conf *sim_conf);
    
    // Return the prediction 
    bool predict(branch *branch, branchsim_stats *sim_stats);
    
    // Update the branch predictor state
    void update_predictor(branch *branch);

    // Cleanup any initialized memory here
    ~perceptron_predictor();
};


#endif
