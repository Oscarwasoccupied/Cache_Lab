/**
 * @ File Name: csim.c
 * @ Author: Xianwei Zou
 * @ AndrewID: xianweiz
 * @ Version: 1.1.0
 * @ Description: Simulate the behavior of a cache
 */

#include "cachelab.h" /* contains printSummary() */
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* contains getopt() */

#define MACHINEBITS 64

char traceFile[200]; /* trace file path*/
int s;               /* s: S=2^s is the set number */
int E;               /* E: num of lines in each set */
int b;               /* b: B=2^b is the size of each block in bytes */
int verbose = 0;

/* structure for a cache line */
typedef struct {
    int valid;
    int dirty;
    unsigned long tag;
    int LRU_time_stamp;
} CacheLine;

/* structure for a cache */
typedef struct {
    int S; /* Set number */
    int E; /* num of lines in each set */
    int B; /* block size */
    CacheLine **set;
} Cache;

Cache *cache = NULL;

/* store num of hits, miss, eviction miss, dirty bits and dirty evictions */
csim_stats_t cache_stats = {0, 0, 0, 0, 0};

int getCli(int argc, char **argv, int *s, int *E, int *b, char *traceFile);
int readTrace(void);
int malloc_cache(void);
int free_cache(void);
int load_op(unsigned long set_bits, unsigned long tag_bits);
int store_op(unsigned long set_bits, unsigned long tag_bits);
int find_max_LRU(unsigned long set_bits);
int eviction_effect(int idx, unsigned long set_bits);
int update_bits(int idx, unsigned long set_bits, unsigned long tag_bits);
int update_time(int idx, unsigned long set_bits);
int print_help(void);

int main(int argc, char **argv) {
    /* set the parameter s E b t from the command line input */
    getCli(argc, argv, &s, &E, &b, traceFile);

    /* initialize the cache */
    malloc_cache();

    /* read the trace file from traceFile */
    readTrace();

    /* calculate the dirty bytes in cache in the end */
    cache_stats.dirty_bytes = (unsigned long)cache->B * cache_stats.dirty_bytes;
    /* dirty bytes evicted in the process */
    cache_stats.dirty_evictions =
        (unsigned long)cache->B * cache_stats.dirty_evictions;

    /* free the cache */
    free_cache();

    /* print summary about hit miss eviction */
    printSummary(&cache_stats);
    return 0;
}

/**
 * Description:
 *     Read command from command line,
 *     and then set parameter for the cache simulator.
 * @param argv is an array of strings, each accessible as a char*
 * @return
 */
int getCli(int argc, char **argv, int *s, int *E, int *b, char *traceFile) {
    int opt;
    while (-1 != (opt = getopt(argc, argv, "vs:E:b:t:"))) {
        switch (opt) {
        case 's':
            *s = atoi(optarg); /* convert s from string to int */
            break;
        case 'E':
            *E = atoi(optarg); /* convert E from string to int */
            break;
        case 'b':
            *b = atoi(optarg); /* convert b from string to int */
            break;
        case 't':
            strcpy(traceFile, optarg); /* copy the trace file path to t */
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
            print_help();
            break;
        default:
            printf("wrong argument\n");
            break;
        }
    }
    return 0;
}

/**
 * Description:
 *     Initialize parameter S, E, B for the cache,
 *     and use malloc to create space for cache and each cache line.
 */
int malloc_cache(void) {
    cache = (Cache *)malloc(sizeof(Cache));
    if (s == 0) {
        cache->S = 1;
    } else {
        cache->S = (1 << s); /* S = 2^s */
    }
    cache->E = E;
    if (b == 0) {
        cache->B = 1;
    } else {
        cache->B = (1 << b); /* B = 2^b */
    }
    cache->set =
        (CacheLine **)malloc(sizeof(CacheLine *) * (unsigned long)(cache->S));

    int i, j;
    for (i = 0; i < (cache->S); i++) {
        cache->set[i] =
            (CacheLine *)malloc(sizeof(CacheLine) * (unsigned long)(cache->E));
        for (j = 0; j < (cache->E); j++) {
            cache->set[i][j].valid = 0;
            cache->set[i][j].dirty = 0;
            cache->set[i][j].tag = 0;
            cache->set[i][j].LRU_time_stamp = 0;
        }
    }
    return 0;
}

/**
 * Description:
 *     Read and execute each line of instruction from the trace file,
 *     and update bits in cache.
 */
int readTrace(void) {
    FILE *pFile;
    char opIdentifier;
    unsigned long address; /* unsigned long: 8 bytes */
    int size;

    pFile = fopen(traceFile, "r");
    if (pFile == NULL) {
        printf("\"%s\" does not exit in the directory\n", traceFile);
        exit(1);
    }

    while (fscanf(pFile, " %c %lx,%d", &opIdentifier, &address, &size) > 0) {
        /* get tag field length */
        int t = MACHINEBITS - s - b;
        /* get opcode set bits and tag bytes */
        unsigned long tag_bits = address >> (b + s);
        unsigned long set_bits;
        if (s == 0) {
            set_bits = 0;
        } else {
            set_bits = ((address << t) >> (t + b));
        }
        /* For Load operation */
        if (opIdentifier == 'L') {
            load_op(set_bits, tag_bits);
        }
        /* For Store operation */
        if (opIdentifier == 'S') {
            store_op(set_bits, tag_bits);
        }
    }

    fclose(pFile);
    return 0;
}

/**
 * @brief Operations to cache when the opcode is Load.
 * @param set_bits set bits in the memory address
 * @param tag_bits tag bits of the memory address
 */
int load_op(unsigned long set_bits, unsigned long tag_bits) {
    /* set selection and line match*/
    int i;
    bool hit_flag = false;
    for (i = 0; i < cache->E; i++) {
        /* Hit: when the set bits and the valid bits both match */
        if ((cache->set[set_bits][i].tag == tag_bits) &&
            (cache->set[set_bits][i].valid == 1)) {
            /* count this hit */
            cache_stats.hits++;
            if (verbose)
                printf("Hit\n");
            /* update time, valid bit and tag bit */
            update_time(i, set_bits);
            hit_flag = true;
        }
    }

    /* Miss: when hits nothing in the cache */
    if (hit_flag == false) {
        /* count the miss */
        cache_stats.misses++;
        if (verbose)
            printf("Miss\n");
        /* find the cache line has the least LRU number */
        int max_idx = find_max_LRU(set_bits);

        /* Eviction miss when the cache set is full,
         * which means the line has the max LRU has valid bit = 1
         */
        eviction_effect(max_idx, set_bits);

        /* update time, valid bit and tag bit */
        update_bits(max_idx, set_bits, tag_bits);
        update_time(max_idx, set_bits);
    }
    return 0;
}

/**
 * @brief Operations to cache when the opcode is Store.
 * @param set_bits set bits in the memory address
 * @param tag_bits tag bits of the memory address
 */
int store_op(unsigned long set_bits, unsigned long tag_bits) {
    /* set selection and line match*/
    int i;
    bool hit_flag = false;
    for (i = 0; i < cache->E; i++) {
        /* Hit: when the set bits and the valid bits both match */
        if ((cache->set[set_bits][i].tag == tag_bits) &&
            (cache->set[set_bits][i].valid == 1)) {
            /* count this hit */
            cache_stats.hits++;
            if (verbose)
                printf("Hit\n");
            /* update time, valid bit and tag bit */
            update_time(i, set_bits);
            /* set the dirty bit */
            if (cache->set[set_bits][i].dirty == 0) {
                cache->set[set_bits][i].dirty = 1;
                cache_stats.dirty_bytes++;
            }
            hit_flag = true;
        }
    }

    /* Miss: when hits nothing in the cache */
    if (hit_flag == false) {
        /* count the miss */
        cache_stats.misses++;
        if (verbose)
            printf("Miss\n");

        /* find the cache line has the least LRU number */
        int max_idx = find_max_LRU(set_bits);

        /* Eviction miss when the cache set is full,
         * which means the line has the max LRU has valid bit = 1
         */
        eviction_effect(max_idx, set_bits);

        /* update time, valid bit and tag bit */
        update_bits(max_idx, set_bits, tag_bits);
        update_time(max_idx, set_bits);

        /* set the dirty bits after write */
        cache->set[set_bits][max_idx].dirty = 1;
        cache_stats.dirty_bytes++;
    }
    return 0;
}

/**
 * @brief Change bits and count dirty bytes after determing if
 *      eviction miss happens
 * @param idx index of the set
 * @param set_bits set bits in the memory address
 */
int eviction_effect(int idx, unsigned long set_bits) {
    /* if eviction happens */
    if (cache->set[set_bits][idx].valid == 1) {
        cache_stats.evictions++;
        if (verbose)
            printf("Evictions\n\n");
        if (cache->set[set_bits][idx].dirty == 1) {
            cache_stats.dirty_evictions++;
            cache->set[set_bits][idx].dirty = 0;
            cache_stats.dirty_bytes--;
        }
    }
    return 0;
}

/**
 * @brief After each hit or miss,
 *      update the time in LRU time stamp.
 * @param idx index of the set
 * @param set_bits set bits in the memory address
 */
int update_time(int idx, unsigned long set_bits) {
    int i;
    for (i = 0; i < cache->E; i++) {
        /* increase all time */
        cache->set[set_bits][i].LRU_time_stamp++;
    }
    /* only set the most recent hit time to 0 */
    cache->set[set_bits][idx].LRU_time_stamp = 0;
    return 0;
}

/**
 * @brief Update the bits in each cache line after miss
 *
 * @param idx index of the set
 * @param set_bits set bits in the memory address
 * @param tag_bits tag bits in the memory address
 */
int update_bits(int idx, unsigned long set_bits, unsigned long tag_bits) {
    cache->set[set_bits][idx].valid = 1;
    cache->set[set_bits][idx].tag = tag_bits;
    return 0;
}

/**
 * @brief Given the set number, return the setline index of the line,
 * that has the minimum LRU value.
 *
 * @param set_bits set bits in the memory address
 */
int find_max_LRU(unsigned long set_bits) {
    int i;
    int max_LRU = 0;
    int max_idx = 0;
    for (i = 0; i < cache->E; i++) {
        if (cache->set[set_bits][i].LRU_time_stamp > max_LRU) {
            max_idx = i;
            max_LRU = cache->set[set_bits][i].LRU_time_stamp;
        }
    }
    return max_idx;
}

/**
 * Description:
 *     free cache line, cache set and cache space.
 *     created by malloc in malloc_cache() function
 */
int free_cache(void) {
    int i;
    for (i = 0; i < (cache->S); i++) {
        free(cache->set[i]); /* free cache line */
    }
    free(cache->set); /* free cache set */
    free(cache);      /* free whole cache */
    return 0;
}

/**
 * Description:
 *     print help when entering command in the cli.
 */
int print_help() {
    printf("Format: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    printf("Options:\n");
    printf("-s <num>   Number of set index bits.\n");
    printf("-E <num>   Number of lines per set.\n");
    printf("-b <num>   Number of block offset bits.\n");
    printf("-t <file>  Trace file path name.\n\n");
    printf("-h         OPTIONAL: Print help.\n");
    printf("-v         OPTIONAL: verbose flag.\n");
    return 0;
}
