/**
 * @file cache.c
 * @brief CS{4/6}290 / ECE{4/6}100 Spring 2021 Project 1 cache simulator
 *
 * Fill in the functions to complete the cache simulator
 *
 * @author <Yue Pan>
 */

#include <cstdarg>
#include <cinttypes>
#include <cstdio>
#include <cstdbool>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <cmath>
#include <vector>
#include "cache.hpp"

#define DEBUG 0

using namespace std;

// Use this for printing errors while debugging your code
// Most compilers support the __LINE__ argument with a %d argument type
// Example: print_error_exit("Error: Could not allocate memory %d", __LINE__);
static inline void print_error_exit(const char *msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);
    fprintf(stderr, msg, argptr);
    va_end(argptr);
    exit(EXIT_FAILURE);
}


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


// cache block, the most basic unit of a cache
// size = 16
struct cache_block_t
{
    bool valid;
    bool dirty;

    uint64_t tag;
    uint64_t frequency;
    uint64_t timestamp;
    uint64_t first_timestamp; // timestamp when the block is FIRST brought in, not updated by subsequent accesses

    void ident()
    {
        printf("struct cache_block_t, size=%lu, valid=%d, dirty=%d\n", sizeof(cache_block_t), valid, dirty);
    }
};

struct victim_cache_block_t
{
    uint64_t tag;  // Here timestamp is tag + index
    uint64_t timestamp;
};

// Secondary unit of a cache: cache set
// size = 8
struct cache_set_t
{
    cache_block_t* blocks;
    cache_block_t* most_recently_used;

    void ident()
    {
        printf("struct cache_set_t, size=%lu\n", sizeof(cache_set_t));
    }
};

struct victim_cache_t
{
    victim_cache_block_t* vc_blocks;

    void ident()
    {
        printf("struct victim_cache_t, size=%lu\n", sizeof(victim_cache_block_t));
    }
};

struct cache_t
{
    cache_set_t* sets;

    void ident()
    {
        printf("struct cache_t, size=%lu\n", sizeof(cache_t));
    }
};


uint64_t c;
uint64_t b;
uint64_t s;
uint64_t vc_size;
uint64_t tag_length;
uint64_t index_length;
uint64_t offset_length;
uint64_t num_sets;
uint64_t num_ways;
uint64_t global_clock = 0;
bool fully_associative_flag;

// INCLUSIVE bit ranges of each address fields, e.g. (3, 0) for the least 4 bits
pair<int, int> tag_range;
pair<int, int> index_range;
pair<int, int> offset_range;
pair<int, int> vc_tag_range;

// OH LORD HE COMING: THE CACHE
cache_t cache;
victim_cache_t victum_cache;

/**
 * Function to initialize any data structures you might need for simulating the cache hierarchy. Use
 * the sim_conf structure for initializing dynamically allocated memory.
 *
 * @param sim_conf Pointer to simulation configuration structure
 *
 */
void sim_init(struct sim_config_t *sim_conf)
{
    cout << "\n\n---- Function sim_init ------\n";

    // setting relevant globals
    c = sim_conf->l1data.c;
    b = sim_conf->l1data.b;
    s = sim_conf->l1data.s;
    vc_size = sim_conf->V;

    index_length  = c - b - s;
    offset_length = b;
    tag_length    = 64 - index_length - offset_length;
    num_sets      = (int) pow(2, index_length);
    num_ways      = (int) pow(2, s);
    tag_range     = make_pair<int, int>(63, 64 - tag_length);
    vc_tag_range  = make_pair<int, int>(63, offset_length);
    index_range   = make_pair<int, int>(63 - tag_length, 64 - tag_length - index_length);
    offset_range  = make_pair<int, int>(offset_length - 1, 0);
    if (c == b + s) // fully associative
        fully_associative_flag = true;
    else // non-fully associative
        fully_associative_flag = false;

    printf("    Extract cache dimensions: C=%lu  B=%lu  S=%lu, %lu sets, %lu ways\n", c, b, s, num_sets, num_ways);
    printf("    Address breakdown: [Tag=%lub] [Index=%lub] [Offset=%lub] \n", tag_length, index_length, offset_length);
    printf("                       [%d Tag %d] [%d Index %d] [%d Offset %d]\n", tag_range.first, tag_range.second, index_range.first, index_range.second, offset_range.first, offset_range.second);

    // Initialization of structs
    auto* sets = new cache_set_t[num_sets];
    for (int i = 0; i < num_sets; i++) {
        sets[i].blocks = new cache_block_t[num_ways];
        sets[i].most_recently_used = nullptr;
        for (int j = 0; j < num_ways; ++j) {
            sets[i].blocks[j].tag = 0;
            sets[i].blocks[j].frequency = 0;
            sets[i].blocks[j].timestamp = 0;
            sets[i].blocks[j].first_timestamp = 0;
            sets[i].blocks[j].valid = false;
            sets[i].blocks[j].dirty= false;
        }
    }
    cache.sets = sets;

    if (vc_size > 0) {
        victum_cache.vc_blocks = new victim_cache_block_t[vc_size];
        for (int i = 0; i < vc_size; ++i) {
            victum_cache.vc_blocks[i].tag = 0;
            victum_cache.vc_blocks[i].timestamp = 0;
        }
    }

    // check valid initialization
//    for (int i = 0; i < num_sets; ++i) {
//        cout << i << " ";
//        cache.sets[i].ident();
//        for (int j = 0; j < num_ways; ++j) {
//            cout << "  " << j << "  ";
//            cache.sets[i].blocks[j].ident();
//        }
//    }
}


/**
 * Function to perform cache accesses, one access at a time. The print_debug function should be called
 * if the debug flag is true
 *
 * @param addr The address being accessed by the processor
 * @param type The type of access - Load (L) or Store (S)
 * @param sim_stats Pointer to simulation statistics structure - Should be populated here
 * @param sim_conf Pointer to the simulation configuration structure - Don't modify it in this function
 */
void cache_access(uint64_t addr, char type, struct sim_stats_t *sim_stats, struct sim_config_t *sim_conf)
{
    if (DEBUG) printf("[%ld] %c 0x%lX", global_clock, type, addr);

    global_clock += 1;
    sim_stats->l1data_num_accesses += 1;
    if (type == 'L')
        sim_stats->l1data_num_accesses_loads += 1;
    if (type == 'S')
        sim_stats->l1data_num_accesses_stores += 1;

    uint64_t tag    = bit_extract(addr, tag_range);
    uint64_t index  = fully_associative_flag ? 0 : bit_extract(addr, index_range); // fully associative always get index 0
    uint64_t offset = bit_extract(addr, offset_range);

    bool miss_flag = true;

    cache_set_t* current_set = &cache.sets[index];

    for (int i = 0; i < num_ways; ++i)
    {
        cache_block_t* current_block = &current_set->blocks[i];
        if (current_block->tag == tag && current_block->valid) { // L1 HIT
            // update frequency and access timestamp
            current_block->timestamp = global_clock;
            current_block->frequency += 1;
            current_set->most_recently_used = current_block;

            if (type == 'S') {
                current_block->dirty = true;
            }

            miss_flag = false;
        }
    }
//  ===================
//  **    L1 MISS    **
//  ===================
    if (miss_flag) {
        sim_stats->l1data_num_misses += 1;
        if (vc_size > 0)
            sim_stats->victim_cache_accesses += 1;
        if (type == 'L')
            sim_stats->l1data_num_misses_loads += 1;
        if (type == 'S')
            sim_stats->l1data_num_misses_stores += 1;

        cache_block_t* block_to_evict = nullptr;

        switch (sim_conf->rp)
        {
            case LRU: {
                uint64_t min_timestamp = UINT64_MAX;
                // For loop to determine the least/earliest timestamp for LRU to evict
                for (int i = 0; i < num_ways; ++i) {
                    auto* current_block = &current_set->blocks[i];
                    if (!current_block->valid) { // if a block is invalid (empty), we "evict" it directly
                        block_to_evict = current_block;
                        break;
                    }
                    if (current_block->valid && current_block->timestamp < min_timestamp) {
                        min_timestamp = current_block->timestamp;
                        block_to_evict = current_block;
                    }
                }
                break;
            }

            case LFU: { // LFU-notMRU
                uint64_t min_frequency = UINT64_MAX;
                uint64_t max_tag = 0;
                // find min_frequency
                for (int i = 0; i < num_ways; ++i) {
                    cache_block_t* current_block = &current_set->blocks[i];
                    if (current_block->valid && current_block->frequency < min_frequency && current_block != current_set->most_recently_used) {
                        min_frequency = current_block->frequency;
                    }
                }
                // for those blocks with min_frequency, find the one with max tag
                for (int i = 0; i < num_ways; ++i) {
                    cache_block_t* current_block = &current_set->blocks[i];
                    if (!current_block->valid) { // if a block is invalid (empty), we "evict" it directly
                        block_to_evict = current_block;
                        break;
                    }
                    if (current_block->valid && current_block->frequency == min_frequency && current_block != current_set->most_recently_used) {
                        if (current_block->tag > max_tag) {
                            max_tag = current_block->tag;
                            block_to_evict = current_block;
                        }
                    }
                }
                break;
            }

            case FIFO: {
                uint64_t least_first_timestamp = UINT64_MAX;
                // For loop to determine the least/earliest timestamp for LRU to evict
                for (int i = 0; i < num_ways; ++i) {
                    auto* current_block = &current_set->blocks[i];
                    if (!current_block->valid) { // if a block is invalid (empty), we "evict" it directly
                        block_to_evict = current_block;
                        break;
                    }
                    if (current_block->valid && current_block->first_timestamp < least_first_timestamp) {
                        least_first_timestamp = current_block->first_timestamp;
                        block_to_evict = current_block;
                    }
                }
                break;
            }

            default: {
                print_error_exit("[ERROR] Invalid replacement policy\n");
                break;
            }
        }

        if (block_to_evict == nullptr) {
            print_error_exit("[ERROR] Did not find block to evict\n");
        }

        // Kick out old block
        if (block_to_evict->dirty) { // write back
            sim_stats->num_writebacks += 1;
            sim_stats->bytes_transferred_to_mem += (int) pow(2, b);
        }
        // If previous block was valid, update eviction
        if (block_to_evict->valid) {
            sim_stats->l1data_num_evictions += 1;
        }
        // Bring in new block
        block_to_evict->valid = true;
        block_to_evict->dirty = (type != 'L');
        block_to_evict->tag = tag;
        block_to_evict->timestamp = global_clock;
        block_to_evict->first_timestamp = global_clock;
        block_to_evict->frequency = 1;

        current_set->most_recently_used = block_to_evict;
        sim_stats->bytes_transferred_from_mem += (int) pow(2, b);
    } // end if (cache MISS)

    if (DEBUG) cout << endl;
}


/**
 * Function to cleanup dynamically allocated simulation memory, and perform any calculations
 * that might be required
 *
 * @param stats Pointer to the simulation structure - Final calculations should be performed here
 */
void sim_cleanup(struct sim_stats_t *sim_stats, struct sim_config_t *sim_conf)
{
    for (int i = 0; i < num_sets; i++) {
        delete [] cache.sets[i].blocks;
    }
    delete [] cache.sets;

    sim_stats->l1data_miss_penalty = 60;
    sim_stats->l1data_miss_rate = (double) sim_stats->l1data_num_misses / sim_stats->l1data_num_accesses;
    sim_stats->avg_access_time = sim_stats->l1data_hit_time + sim_stats->l1data_miss_rate * sim_stats->l1data_miss_penalty;
}
