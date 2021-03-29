/**
 * @file branchsim_driver.cpp
 * @brief Trace reader and driver for a branch prediction project assigned in
 * CS{4/6}290 and ECE{4/6}100 for the Spring 2021 semester
 *
 * DON"T MODIFY ANY CODE IN THIS FILE!
 *
 * @author Anirudh Jain
 */

#include <getopt.h>
#include <unistd.h>


#include <cstdarg>
#include <cinttypes>
#include <cstdio>
#include <cstdbool>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include "branchsim.h"
#include "gshare.h"
#include "perceptron.h"
#include "yeh_patt.h"



// Print message and exit from the application
static inline void print_error_exit(const char *msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);
    fprintf(stderr, msg, argptr);
    va_end(argptr);
    exit(EXIT_FAILURE);
}


// Print error usage
static void print_err_usage(const char *err)
{

    fprintf(stderr, "%s\n", err);
    fprintf(stderr, "./branchsim -i <trace file> [Options]\n");
    fprintf(stderr, "-O [Predictor {1:Gshare, 2:Yeh-Patt, 3:Perceptron}\n");
    fprintf(stderr, "-G [Size of History Reg (bits)]\n");
    fprintf(stderr, "-P [Num entries in Pattern / Perceptron Table]\n");
    fprintf(stderr, "-N [Num stages in modelled pipeline\n");
    fprintf(stderr, "-H <Print this message>\n");

    exit(EXIT_FAILURE);
}

// Function to print the run configuration
static void print_sim_config(branchsim_conf *sim_conf, FILE* data_file)
{
    fprintf(data_file, "SIMULATION CONFIGURATION\n");
    fprintf(data_file, "G:          %" PRIu64 "\n", sim_conf->G);
    fprintf(data_file, "P:          %" PRIu64 "\n", sim_conf->P);
    fprintf(data_file, "N:          %" PRIu64 "\n", sim_conf->N);
    fprintf(data_file, "Predictor:  %s\n", pred_to_string.at(sim_conf->predictor));
}

// Function to print the simulation output
static void print_sim_output(branchsim_stats *sim_stats, FILE* data_file)
{
    fprintf(data_file, "\nSIMULATION OUTPUT\n");
    fprintf(data_file, "Total Instructions:                 %" PRIu64 "\n", sim_stats->total_instructions);
    fprintf(data_file, "Total Branch Instructions:          %" PRIu64 "\n", sim_stats->num_branch_instructions);
    fprintf(data_file, "Branches Correctly Predicted:       %" PRIu64 "\n", sim_stats->num_branches_correctly_predicted);
    fprintf(data_file, "Branches Miss Predicted:            %" PRIu64 "\n", sim_stats->num_branches_miss_predicted);
    fprintf(data_file, "Misses Per Kilo Instructions:       %" PRIu64 "\n", sim_stats->misses_per_kilo_instructions);
    fprintf(data_file, "Number of Tag conflicts             %" PRIu64 "\n", sim_stats->num_tag_conflicts);

    fprintf(data_file, "Fraction Branch Instructions:       %.8f\n", sim_stats->fraction_branch_instructions);
    fprintf(data_file, "Branch Prediction accuracy:         %.8f\n", sim_stats->prediction_accuracy);

    fprintf(data_file, "Total Stages in the Pipeline:       %" PRIu64 "\n", sim_stats->N);
    fprintf(data_file, "Branch Miss Prediction Stalls:      %" PRIu64 "\n", sim_stats->stalls_per_miss_predicted_branch);
    fprintf(data_file, "Average CPI:                        %.8f\n", sim_stats->average_CPI);
}

// Functionality to print the MPKI of the branch predictor as it evolves
static void process_histogram(std::vector<std::pair<uint64_t, uint64_t>> &histogram, uint64_t interval)
{
    for (auto iter = histogram.end(); iter != (histogram.begin()); iter--) {
        iter->second -= (iter - 1)->second;
    }

    for (auto &iter : histogram) {
        iter.first /= interval;
        iter.second = iter.second * 1000 / interval;
    }
}

static void print_histogram(std::vector<std::pair<uint64_t, uint64_t>> &histogram, uint64_t interval)
{
    std::cout << std::endl << "DEBUG OUTPUT" << std::endl;
    std::cout << "Printing debug MPKI for [interval=" << interval << "]" << std::endl;
    std::cout << "Interval Number: Interval MPKI" << std::endl;
    for (auto iter : histogram) {
        std::cout << iter.first << ": " << iter.second << std::endl;
    }
}

// Main function for the simulator driver
int main(int argc, char *const argv[])
{

    FILE* data_file;

    if (argc < 2) {
        print_err_usage("Input configuration file not provided");
    }

    // Trace file
    FILE *trace = NULL;

    // FILE *temp = fopen("temp.trace", "w+");

    branchsim_stats sim_stats;
    memset(&sim_stats, 0, sizeof(branchsim_stats));

    branchsim_conf sim_conf;
    bool debug = false;

    std::string trace_name = "";

    int opt;
    int pred_num;
    while (-1 != (opt = getopt(argc, argv, "o:O:g:G:n:N:p:P:i:I:dDhH"))) {
        switch (opt) {
            case 'o':
            case 'O':
                pred_num = std::atoi(optarg);
                if (pred_num < 1 || pred_num > 3) { print_err_usage("Invalid predictor option"); }
                sim_conf.predictor = (branchsim_conf::PREDICTOR) pred_num;
                break;

            case 'g':
            case 'G':
                sim_conf.G = (uint64_t) std::atoi(optarg);
                break;

            case 'p':
            case 'P':
                sim_conf.P = (uint64_t) std::atoi(optarg);
                break;

            case 'n':
            case 'N':
                sim_conf.N = (uint64_t) std::atoi(optarg);
                break;

            case 'i':
            case 'I':
                trace = std::fopen(optarg, "r");
                trace_name = optarg;
                if (trace == NULL) {
                    print_err_usage("Could not open the input trace file");
                }
                break;

            case 'd':
            case 'D':
                debug = true;
                break;

            case 'h':
            case 'H':
                print_err_usage("");
                break;
            default:
                print_err_usage("Invalid argument to program");
                break;
        }
    }

    char filename[100];
    int a = trace_name.find("/") +1, b = trace_name.find("br") - 1;
    std::cout << a << "  " << b << trace_name << std::endl;
    trace_name = trace_name.substr(a, b);
    const char *cstr = trace_name.c_str();
    printf("%s", cstr);
    sprintf(filename, "/home/cx872/Documents/ECE_4100/2project/data/O%d_G%ld_P%ld_%s.txt", sim_conf.predictor, sim_conf.G, sim_conf.P, cstr);
    data_file = fopen(filename, "w");
    printf("%s\n", filename);

    print_sim_config(&sim_conf, data_file);
    fprintf(data_file, "SETUP COMPLETE - STARTING SIMULATION\n");

    // Allocate the predictor
    branch_predictor_base *predictor;
    switch (sim_conf.predictor) {

        case branchsim_conf::YEH_PATT:
            predictor = new yeh_patt();
            break;

        case branchsim_conf::PERCEPTRON:
            predictor = new perceptron_predictor();
            break;

        case branchsim_conf::GSHARE:
        default:
            predictor = new gshare();
            break;
    }



    // Initialize the predictor
    predictor->init_predictor(&sim_conf);
    size_t MPKI_INTERVAL = 100000;

    // Histogram for debug mode
    std::vector<std::pair<uint64_t, uint64_t>> histogram;

    branch branch;
    size_t num_branches = 0;
    while (!feof(trace)) {
        int ret = std::fscanf(trace, "%" PRIx64 " %d %" PRIu64 "\n", &branch.ip, (int *) &branch.is_taken, &branch.inst_num);

        // Read branch from the input trace
        if (ret == 3) {
            bool prediction = predictor->predict(&branch, &sim_stats);
            branchsim_update_stats(prediction, &branch, &sim_stats);
            predictor->update_predictor(&branch);

            num_branches++;
        }

        if ((num_branches % MPKI_INTERVAL) == 0) {
            if (debug) {
                histogram.push_back(std::pair<uint64_t, uint64_t>(num_branches, sim_stats.num_branches_miss_predicted));
            }
        }
    }

    // Function cleanup final statistics
    sim_stats.N = sim_conf.N;
    branchsim_cleanup_stats(&sim_stats);

    // This calls the destructor of the predictor before freeing the allocated memory
    delete predictor;

    fclose(trace);

    if (debug) {
        process_histogram(histogram, MPKI_INTERVAL);
        print_histogram(histogram, MPKI_INTERVAL);
    }

    print_sim_output(&sim_stats, data_file);

    fclose(data_file);
    return 0;
}
