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


//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


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

// cache block, the most basic unit of a cach
struct cache_block_t
{
    bool valid;
    bool dirty;

    uint64_t tag;
    uint64_t vc_tag;
    uint64_t frequency;
    uint64_t timestamp;
    uint64_t first_timestamp; // timestamp when the block is FIRST brought in, not updated by subsequent accesses
};

// Secondary unit of a cache: cache set
struct cache_set_t
{
    cache_block_t* blocks;
    cache_block_t* most_recently_used;
};

struct victim_cache_t
{
    cache_block_t* vc_blocks;
};

struct cache_t
{
    cache_set_t* sets;
};

// Globals for convenience
uint64_t c;
uint64_t b;
uint64_t s;
uint64_t vc_size;
uint64_t tag_length;
uint64_t index_length;
uint64_t offset_length;
uint64_t num_sets;
uint64_t num_ways;
bool fully_associative_flag;
bool vc_hit_flag = false;

int      vc_hit_index;
int      vc_evict_index;
uint64_t global_clock = 0;
uint64_t counter1 = 0;
uint64_t counter2 = 0;
uint64_t counter3 = 0;

// INCLUSIVE bit ranges of each address fields, e.g. (3, 0) for the least 4 bits
pair<int, int> tag_range;
pair<int, int> index_range;
pair<int, int> offset_range;
pair<int, int> vc_tag_range;

// OH LAWD HE COMIN: THE CACHE
cache_t cache;
victim_cache_t victim_cache;


void print_vc(victim_cache_t* vc) {
    for (int i = 0; i < vc_size; ++i) {
        cache_block_t* block = &vc->vc_blocks[i];
        if (block->dirty) {
            if (DEBUG)
                printf(BOLDWHITE "<%d>[vc_tag=%ld  @%-7ld  %d|" BOLDRED "%d" BOLDWHITE "]  " RESET, i, block->vc_tag,
                       block->timestamp, block->valid, block->dirty);
        } else {
            if (DEBUG)
                printf(BOLDWHITE "<%d>[vc_tag=%ld  @%-7ld  %d|%d]  " RESET, i, block->vc_tag, block->timestamp,
                       block->valid, block->dirty);
        }
    }
    if (DEBUG) printf("\n");
}

void print_block(cache_block_t* block) {
    if (DEBUG) printf("tag=%ld  vc_tag=%ld  %d|%d" RESET, block->tag, block->vc_tag, block->valid, block->dirty);
}

/**
 * Function to initialize any data structures you might need for simulating the cache hierarchy. Use
 * the sim_conf structure for initializing dynamically allocated memory.
 *
 * @param sim_conf Pointer to simulation configuration structure
 *
 */
void sim_init(struct sim_config_t *sim_conf)
{
//    cout << "\n\n---- Function sim_init ------\n";

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

    if (DEBUG) printf("    Extract cache dimensions: C=%lu  B=%lu  S=%lu, %lu sets, %lu ways\n", c, b, s, num_sets, num_ways);
    if (DEBUG) printf("    Address breakdown: [Tag=%lub] [Index=%lub] [Offset=%lub] \n", tag_length, index_length, offset_length);
    if (DEBUG) printf("                       [%d Tag %d] [%d Index %d] [%d Offset %d]\n", tag_range.first, tag_range.second, index_range.first, index_range.second, offset_range.first, offset_range.second);
    if (DEBUG) printf("                       [%d VC_Tag %d] [%d Offset %d]\n", vc_tag_range.first, vc_tag_range.second, offset_range.first, offset_range.second);

    // Initialize L1
    auto* sets = new cache_set_t[num_sets];
    for (int i = 0; i < num_sets; i++) {
        sets[i].blocks = new cache_block_t[num_ways];
        sets[i].most_recently_used = nullptr;
        for (int j = 0; j < num_ways; ++j) {
            sets[i].blocks[j].tag               = 0;
            sets[i].blocks[j].vc_tag            = 0;
            sets[i].blocks[j].frequency         = 0;
            sets[i].blocks[j].timestamp         = 0;
            sets[i].blocks[j].first_timestamp   = 0;
            sets[i].blocks[j].valid             = false;
            sets[i].blocks[j].dirty             = false;
        }
    }
    cache.sets = sets;

    // Initialize VC
    if (vc_size > 0) {
        victim_cache.vc_blocks = new cache_block_t[vc_size];
        for (int i = 0; i < vc_size; ++i) {
            victim_cache.vc_blocks[i].tag               = 0;
            victim_cache.vc_blocks[i].vc_tag            = 0;
            victim_cache.vc_blocks[i].frequency         = 0;
            victim_cache.vc_blocks[i].timestamp         = 0;
            victim_cache.vc_blocks[i].first_timestamp   = 0;
            victim_cache.vc_blocks[i].valid             = false;
            victim_cache.vc_blocks[i].dirty             = false;
        }
    }

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
void cache_access(uint64_t addr, char type, struct sim_stats_t *sim_stats, struct sim_config_t *sim_conf) {

    global_clock += 1;

    if (DEBUG) {
        if (global_clock == 1000) print_error_exit("1000 cycles");
    }


    sim_stats->l1data_num_accesses += 1;
    if (type == 'L') {
        sim_stats->l1data_num_accesses_loads += 1;
    }
    if (type == 'S') {
        sim_stats->l1data_num_accesses_stores += 1;
    }

    uint64_t tag = bit_extract(addr, tag_range);
    uint64_t vc_tag = bit_extract(addr, vc_tag_range);
    uint64_t index = fully_associative_flag ? 0 : bit_extract(addr, index_range); // fully associative always get index 0

    cache_set_t *current_set = &cache.sets[index];

    if (DEBUG) printf(BOLDWHITE"(%7ld) ----- ", global_clock);
    print_vc(&victim_cache);
    if (DEBUG) printf("            " BOLDBLUE "<%c>" RESET " 0x%lX  index=%3d  Tag=%ld  VC_tag=%ld\n", type, addr, index, tag, vc_tag);
    if (DEBUG) printf("            [CURRENT_SET]       index=%3d  tag=%ld  vc_tag=%ld  %d|%d\n", index, current_set->blocks[0].tag, current_set->blocks[0].vc_tag, current_set->blocks[0].valid, current_set->blocks[0].dirty);

    for (int i = 0; i < num_ways; ++i) {
        cache_block_t *current_block = &current_set->blocks[i];
        //  ===================
        //  ==    L1 HIT     ==
        //  ===================
        if (current_block->tag == tag && current_block->valid) {
            // update frequency and access timestamp
            current_block->timestamp = global_clock;
            current_block->frequency += 1;
            current_set->most_recently_used = current_block;  // only matters for LFU-notMRU

            // if writing to a block in L1, mark it dirty
            if (type == 'S') {
                current_block->dirty = true;
                if (DEBUG) printf(BOLDCYAN "            Marking L1 block dirty\n");
            }
            if (DEBUG) printf("            " BOLDGREEN "L1 HIT\n" RESET);
            if (DEBUG) printf("\n");
            return;
        }
    }

//  ===================
//  ==    L1 MISS    ==    94926  94745  direct_mapped, gcc.trace
//  ===================
    // update miss stats
    sim_stats->l1data_num_misses += 1;
    if (type == 'L') {
        sim_stats->l1data_num_misses_loads += 1;
    }
    if (type == 'S') {
        sim_stats->l1data_num_misses_stores += 1;
    }


    // This switch is to find the L1 l1_evict_block under different replacement policies
    cache_block_t *l1_evict_block = nullptr;
    switch (sim_conf->rp) {
        case LRU: {
            uint64_t min_timestamp = UINT64_MAX;
            // For loop to determine the least/earliest timestamp for LRU to evict
            for (int i = 0; i < num_ways; ++i) {
                auto *current_block = &current_set->blocks[i];
                if (!current_block->valid) { // if a block is invalid (empty), we "evict" it directly
                    l1_evict_block = current_block;
                    break;
                }
                if (current_block->valid && current_block->timestamp < min_timestamp) {
                    min_timestamp = current_block->timestamp;
                    l1_evict_block = current_block;
                }
            }
            break;
        }

        case LFU: { // LFU-notMRU
            uint64_t min_frequency = UINT64_MAX;
            uint64_t max_tag = 0;

            // find min_frequency
            for (int i = 0; i < num_ways; ++i) {
                cache_block_t *current_block = &current_set->blocks[i];
                if (current_block->valid && current_block->frequency < min_frequency &&
                    current_block != current_set->most_recently_used) {
                    min_frequency = current_block->frequency;
                }
            }

            // for those blocks with min_frequency, find the one with max tag
            for (int i = 0; i < num_ways; ++i) {
                cache_block_t *current_block = &current_set->blocks[i];
                if (!current_block->valid) { // if a block is invalid (empty), we "evict" it directly
                    l1_evict_block = current_block;
                    break;
                }
                if (current_block->valid && current_block->frequency == min_frequency &&
                    current_block != current_set->most_recently_used) {
                    if (current_block->tag > max_tag) {
                        max_tag = current_block->tag;
                        l1_evict_block = current_block;
                    }
                }
            }
            break;
        }

        case FIFO: {
            uint64_t least_first_timestamp = UINT64_MAX;
            // For loop to determine the least/earliest first_timestamp for LRU to evict
            for (int i = 0; i < num_ways; ++i) {
                auto *current_block = &current_set->blocks[i];
                if (!current_block->valid) { // if a block is invalid (empty), we "evict" it directly
                    l1_evict_block = current_block;
                    break;
                }
                if (current_block->valid && current_block->first_timestamp < least_first_timestamp) {
                    least_first_timestamp = current_block->first_timestamp;
                    l1_evict_block = current_block;
                }
            }
            break;
        }

        default: {
            print_error_exit("[ERROR] Invalid replacement policy\n");
            break;
        }
    }

    if (l1_evict_block == nullptr) {
        print_error_exit("[ERROR] Did not find block to evict\n");
    }


//    if (DEBUG) printf("            [L1 EVICT BLOCK]    index=%3d  tag=%ld  vc_tag=%ld  %d|%d\n", index, l1_evict_block->tag, l1_evict_block->vc_tag, l1_evict_block->valid, l1_evict_block->dirty);
    cache_block_t* vc_evict_block = nullptr;
    // Search for VC Hit
    if (vc_size > 0) {
        vc_hit_flag = false;
        cache_block_t *vc_hit_block = nullptr;
        sim_stats->victim_cache_accesses += 1;

        for (int i = 0; i < vc_size; ++i) {
            cache_block_t *current_vc_block = &victim_cache.vc_blocks[i];
//          ===================
//          ==    VC HIT     ==
//          ===================
            if (current_vc_block->vc_tag == vc_tag) {
                vc_hit_block = current_vc_block;
                vc_hit_flag = true;
                vc_hit_index = i;
                if (type == 'S') {
                    vc_hit_block->dirty = true;
                    if (DEBUG) printf(BOLDCYAN "            Marking vc_hit_block dirty\n");
                }
                break;
            }
        }
        if (vc_hit_flag)
            sim_stats->victim_cache_hits += 1;
        else {
            sim_stats->victim_cache_misses += 1;
        }

//      =====================================================
//      ==  L1 MISS, VC HIT: swap block between L1 and VC  ==
//      =====================================================
        if (vc_hit_flag && l1_evict_block->valid) {
            counter1++;
            if (DEBUG) printf("            " BOLDYELLOW "L1 MISS, VC HIT. Action: Swap(vc_hit_block, l1_evict_block)\n" RESET);
            // Swap the following two using memcpy
            // VC: vc_hit_block
            // L1: l1_evict_block
            cache_block_t *temp = new cache_block_t;

            if (DEBUG) printf("            BEFORE: [l1_evict_block]  "); if (DEBUG) print_block(l1_evict_block);
            if (DEBUG) printf("    [vc_hit_block %d]  ", vc_hit_index); if (DEBUG) print_block(vc_hit_block); if (DEBUG) printf("\n");

            memcpy(temp, l1_evict_block, sizeof(cache_block_t));
            memcpy(l1_evict_block, vc_hit_block, sizeof(cache_block_t));
            memcpy(vc_hit_block, temp, sizeof(cache_block_t));
            delete temp;

//            if (DEBUG) printf("                                                            -------- >\n");
            if (DEBUG) printf("             AFTER: [l1_evict_block]  "); if (DEBUG) print_block(l1_evict_block);
            if (DEBUG) printf("    [vc_hit_block %d]  ", vc_hit_index); if (DEBUG) print_block(vc_hit_block); if (DEBUG) printf("\n");

            vc_hit_block->timestamp = global_clock;
            vc_hit_block->first_timestamp = global_clock;
            l1_evict_block->timestamp = global_clock;
            l1_evict_block->first_timestamp = global_clock;
            l1_evict_block->frequency = 1;
            current_set->most_recently_used = l1_evict_block;

            sim_stats->l1data_num_evictions += 1;
            if (DEBUG) printf("\n");

            return;
        }

//      ==========================================================================
//      ==  L1 MISS, VC MISS : L1 block to VC, VC evict block, new block to L1  ==
//      ==========================================================================
        if (!vc_hit_flag) {
            uint64_t min_timestamp = UINT64_MAX;
//            cache_block_t *vc_evict_block = nullptr;

            // find the VC block to evict under FIFO policy
            for (int i = 0; i < vc_size; ++i) {
                // if VC has empty (invalid) block, use it first
                if (!victim_cache.vc_blocks[i].valid) {
                    vc_evict_block = &victim_cache.vc_blocks[i];
                    vc_evict_index = i;
                    break;
                }
                if (victim_cache.vc_blocks[i].timestamp < min_timestamp) {
                    min_timestamp = victim_cache.vc_blocks[i].timestamp;
                    vc_evict_block = &victim_cache.vc_blocks[i];
                    vc_evict_index = i;
                }
            }

            if (DEBUG) printf("            " BOLDRED "L1 MISS, VC MISS. Action: Move L1 block to VC, Evict block from VC, Bring new block to L1\n" RESET);
            if (DEBUG) printf("            Before: [l1_evict_block  ] "); if (DEBUG) print_block(l1_evict_block);  // if (DEBUG) printf("\n");
            if (DEBUG) printf("  [vc_evict_block %d] ", vc_evict_index); if (DEBUG) print_block(vc_evict_block); if (DEBUG) printf("\n");

            if (vc_evict_block == nullptr) {
                print_error_exit("Did not find VC block to evict");
            }

            // if evicting a dirty block from VC, must update writeback
            if (vc_evict_block->valid && l1_evict_block->valid && vc_evict_block->dirty) {
                //            if (DEBUG) printf("[%ld] %c 0x%lX\n", global_clock, type, addr);
                sim_stats->num_writebacks += 1;
                sim_stats->bytes_transferred_to_mem += (int) pow(2, b);
                if (DEBUG) printf(BOLDCYAN "  num_writeback += 1" RESET); if (DEBUG) printf("\n");
            }

            // move valid block from L1 to VC
            if (l1_evict_block->valid) {
                memcpy(vc_evict_block, l1_evict_block, sizeof(cache_block_t));
                vc_evict_block->timestamp = global_clock;
                vc_evict_block->first_timestamp = global_clock;
                vc_evict_block->frequency = 1;

                sim_stats->l1data_num_evictions += 1;
            }
        } // end if (L1 + VC MISS)
    }

//      =========================================================
//      ==  L1 MISS (no VC) : L1 evict block, new block to L1  ==
//      =========================================================
    if (vc_size == 0) {
        if (l1_evict_block->dirty) { // write back
            sim_stats->num_writebacks += 1;
            sim_stats->bytes_transferred_to_mem += (int) pow(2, b);
        }
        // If previous block was valid, update eviction
        if (l1_evict_block->valid) {
            sim_stats->l1data_num_evictions += 1;
        }
    } // end if (entire cache MISS)

    // Bring in a new block to L1, applies to with VC and without VC
    l1_evict_block->valid           = true;
    if (!vc_hit_flag)
        l1_evict_block->dirty           = (type == 'S');  if (DEBUG && type == 'S') printf(BOLDGREEN "            Bring in dirty L1 block \n" RESET);
    l1_evict_block->tag             = tag;
    l1_evict_block->vc_tag          = vc_tag;
    l1_evict_block->timestamp       = global_clock;
    l1_evict_block->first_timestamp = global_clock;
    l1_evict_block->frequency       = 1;
    current_set->most_recently_used = l1_evict_block;

    sim_stats->bytes_transferred_from_mem += (int) pow(2, b);

    if (DEBUG) printf("             After: [l1_evict_block  ] "); if (DEBUG) print_block(l1_evict_block);
    if (DEBUG) printf("  [vc_evict_block %d] ", vc_evict_index); if (DEBUG) print_block(vc_evict_block); if (DEBUG) printf("\n\n");

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
    delete [] victim_cache.vc_blocks;

    sim_stats->l1data_miss_penalty = 60;
    sim_stats->l1data_miss_rate = (double) sim_stats->l1data_num_misses / sim_stats->l1data_num_accesses;

    if (vc_size > 0) {
        sim_stats->victim_cache_miss_rate = (double) sim_stats->victim_cache_misses / sim_stats->victim_cache_accesses;
        sim_stats->avg_access_time = sim_stats->l1data_hit_time + sim_stats->l1data_miss_rate * sim_stats->victim_cache_miss_rate * sim_stats->l1data_miss_penalty;
    } else {
        sim_stats->avg_access_time = sim_stats->l1data_hit_time + sim_stats->l1data_miss_rate * sim_stats->l1data_miss_penalty;
    }
    if (DEBUG) printf("\n\n\n counter 1: %ld", counter1);
}
