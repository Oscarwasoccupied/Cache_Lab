/**
 * @ File Name: csim.c
 * @ Author: Xianwei Zou
 * @ AndrewID: xianweiz
 * @ Version: 1.0.0
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
int find_max_LRU (int set_bits);
int update_time(int idx, int set_bits);

int main(int argc, char **argv) {
    /* set the parameter s E b t from the command line input */
    getCli(argc, argv, &s, &E, &b, traceFile);

    /* initialize the cache */
    malloc_cache();

    /* read the trace file from traceFile */
    readTrace();

    cache_stats.dirty_bytes = cache->B * cache_stats.dirty_bytes;
    cache_stats.dirty_evictions = cache->B * cache_stats.dirty_evictions;

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
            // printf("s=%d\n", *s);
            break;
        case 'E':
            *E = atoi(optarg); /* convert E from string to int */
            // printf("E=%d\n", *E);
            break;
        case 'b':
            *b = atoi(optarg); /* convert b from string to int */
            // printf("b=%d\n", *b);
            break;
        case 't':
            strcpy(traceFile, optarg); /* copy the trace file path to t */
            // printf("traceFile=%s\n", traceFile);
            break;
        case 'v':
            verbose = 1;
            break;
        case 'h':
            /* ******** to do help ************ */
            // printf("wrong argument\n");
            break;
        default:
            /* ******** to do default ********* */
            // printf("wrong argument\n");
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
    cache->set = (CacheLine **)malloc(sizeof(CacheLine *) * (cache->S));

    int i, j;
    for (i = 0; i < (cache->S); i++) {
        cache->set[i] = (CacheLine *)malloc(sizeof(CacheLine) * (cache->E));
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
 *     and  update data in cache.
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
        // int set_bits = (address >> b) & ((unsigned)(-1) >> (8 * sizeof(unsigned) - s));
        printf("op=%c\n", opIdentifier);
        printf("address=%lx\n", address);
        // printf("tag_bits=%lx\n", tag_bits);
        // printf("tag_bits=%d\n", tag_bits);
        // printf("set_bits=%lx\n\n", set_bits);
        // printf("set_bits=%d\n\n", set_bits);

        /* For Load operation */
        if (opIdentifier == 'L') {
            /* set selection and line match*/
            int i;
            bool hit_flag = false;
            for (i = 0; i < cache->E; i++) {
                /* Hit: when the set bits and the valid bits both match */
                if ((cache->set[set_bits][i].tag == tag_bits) &&
                    (cache->set[set_bits][i].valid == 1)) {
                    /* count this hit */
                    cache_stats.hits++;
                    if(verbose)
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
                if(verbose)
                    printf("Miss\n");
                /* find the cache line has the least LRU number */
                int max_idx = find_max_LRU (set_bits);

                /* put the value into a cache line which has the least LRU
                 * number */
                if (cache->set[set_bits][max_idx].valid == 1) {
                    cache_stats.evictions++;
                    if(verbose)
                        printf("Evictions\n\n");
                    if (cache->set[set_bits][max_idx].dirty == 1) {
                        cache_stats.dirty_evictions++;
                        cache->set[set_bits][max_idx].dirty = 0;
                        cache_stats.dirty_bytes--;
                    }
                }
                /* read data from low memory write to cache line */
                cache->set[set_bits][max_idx].valid = 1;
                cache->set[set_bits][max_idx].tag = tag_bits;
                /* update time */
                update_time(max_idx, set_bits);
            }
        }

        if (opIdentifier == 'S') {
            /* set selection and line match*/
            int i;
            bool hit_flag = false;
            for (i = 0; i < cache->E; i++) {
                /* Hit: when the set bits and the valid bits both match */
                if ((cache->set[set_bits][i].tag == tag_bits) &&
                    (cache->set[set_bits][i].valid == 1)) {
                    /* count this hit */
                    cache_stats.hits++;
                    if(verbose)
                        printf("Hit\n");
                    /* update time, valid bit and tag bit */
                    update_time(i, set_bits);
                    /* set the dirty bit */
                    if (cache->set[set_bits][i].dirty == 0)
                    {
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
                if(verbose)
                    printf("Miss\n");

                /* find the cache line has the least LRU number */
                int max_idx = find_max_LRU (set_bits);

                /* put the value into a cache line which has the least LRU
                 * number */
                if (cache->set[set_bits][max_idx].valid == 1) {
                    cache_stats.evictions++;
                    if(verbose)
                        printf("Evictions\n\n");
                    if (cache->set[set_bits][max_idx].dirty == 1) {
                        cache_stats.dirty_evictions++;
                        cache->set[set_bits][max_idx].dirty = 0;
                        cache_stats.dirty_bytes--;
                    }
                }
                /* update time, valid bit and tag bit */
                cache->set[set_bits][max_idx].valid = 1;
                cache->set[set_bits][max_idx].tag = tag_bits;
                update_time(max_idx, set_bits);

                /* set the dirty bits after write */
                cache->set[set_bits][max_idx].dirty = 1;
                cache_stats.dirty_bytes++;
            }
        }
    }

    fclose(pFile);
    return 0;
}

int update_time(int idx, int set_bits) {
    int i;
    for(i = 0; i < cache->E; i++){
        /* increase all time */
        cache->set[set_bits][i].LRU_time_stamp++;
    }
    /* only set the most recent hit time to 0 */
    cache->set[set_bits][idx].LRU_time_stamp = 0;
    return 0;
}


/**
 * @brief Given the set number, return the setline index of the line,
 * that has the minimum LRU value.
 *
 * @param set_bits
 */
int find_max_LRU (int set_bits) {
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
 *     free cache line, cache set and cache space
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

