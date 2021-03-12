#ifndef PERCEPTRON_H
#define PERCEPTRON_H

#include "branchsim.h"
#include <vector>

// Perceptron predictor definition
class perceptron_predictor : public branch_predictor_base
{

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
