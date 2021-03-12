
#include <iostream>

#include "branchsim.h"
#include "gshare.h"
#include "perceptron.h"
#include "yeh_patt.h"

// Don't modify this line -- Its to make the compiler happy
branch_predictor_base::~branch_predictor_base() {}


// ******* Student Code starts here *******

// Gshare Branch Predictor

void gshare::init_predictor(branchsim_conf *sim_conf)
{

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

}

bool yeh_patt::predict(branch *branch, branchsim_stats *sim_stats)
{
   return NOT_TAKEN;
}

void yeh_patt::update_predictor(branch *branch)
{

}

yeh_patt::~yeh_patt()
{

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

}

/**
 *  Function to cleanup branchsim statistic computations such as prediction rate, etc.
 *
 *  @param stats Pointer to the simulation statistics -- update in this function
*/
void branchsim_cleanup_stats(branchsim_stats *sim_stats)
{

}
