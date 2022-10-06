/** 
 * @ File Name: csim.c
 * @ Author: Xianwei Zou
 * @ AndrewID: xianweiz
 * @ Version: 0.0.0
 * @ Description: Simulate the behavior of a cache
 */

#include "cachelab.h" /* contains printSummary() */
#include <unistd.h>   /* contains getopt() */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MACHINEBITS 64

char traceFile[200]; /* trace file path*/
int s; /* s: S=2^s is the set number */
int E; /* E: num of lines in each set */
int b; /* b: B=2^b is the size of each block in bytes */

/* structure for a cache line */
typedef struct {
    int valid;
    int tag;
    int LRU_time_stamp;
} CacheLine;

/* structure for a cache */
typedef struct {
    int S;
    int E;
    int B;
    CacheLine **set ;
} Cache;

Cache *cache = NULL;

/* store num of hits, miss, eviction miss, dirty bits and dirty evictions */
csim_stats_t cache_stats = {0,0,0,0,0};

int getCli(int argc, char **argv, int *s, int *E, int *b, char *traceFile);
int readTrace();
int malloc_cache();
int free_cache();

int main(int argc, char **argv) {
    /* set the parameter s E b t from the command line input */
    getCli(argc, argv, &s, &E, &b, traceFile);
    
    /* ************ TO DO ************ */
    /* check if the s,B,b parameter is valid */

    /* ************ TO DO ************ */
    /* initialize the cache */
    malloc_cache();

    /* ************ TO DO ************ */
    /* read the trace file from t */
    readTrace();

    /* ************ TO DO ************ */
    /* free the cache */

    /* ************ TO DO ************ */
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
int getCli(int argc, char **argv, int *s, int *E, int *b, char *traceFile){
    int opt;
    while(-1 != (opt = getopt(argc, argv, "s:E:b:t:"))) {
        switch (opt) {
            case 's':
                *s = atoi(optarg); /* convert s from string to int */
                printf("s=%d\n", *s);
                break;
            case 'E':
                *E = atoi(optarg); /* convert E from string to int */
                printf("E=%d\n", *E);
                break;
            case 'b':
                *b = atoi(optarg); /* convert b from string to int */
                printf("b=%d\n", *b);
                break;
            case 't':
                strcpy(traceFile, optarg); /* copy the trace file path to t */
                printf("traceFile=%s\n", traceFile);
                break;
            case 'v':
                /* ******** to do verbose ********* */
                printf("wrong argument\n");
            case 'h':
                /* ******** to do help ************ */
                printf("wrong argument\n");
                break;
            default:
                /* ******** to do default ********* */
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
int malloc_cache() {
    cache = (Cache *) malloc(sizeof(Cache));
    cache->S = (1 << s); /* S = 2^s */
    cache->E = E;
    cache->B = (1 << b); /* B = 2^b */
    cache->set = (CacheLine **) malloc(sizeof(CacheLine *) * cache->S);

    for(int i = 0; i < cache->S; i++) {
        cache->set[i] = (CacheLine *) malloc(sizeof(CacheLine) * E);
        for(int j = 0; j < E; j++) {
            cache->set[i][j].valid = 0;
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
int readTrace() {
    FILE *pFile;
    char opIdentifier;
    unsigned long address; /* unsigned long: 8 bytes */
    int size;   

    pFile = fopen(traceFile, "r");
    if(pFile == NULL) {
        printf("\"%s\" does not exit in the directory\n", traceFile);
        exit(1);
    }

    while(fscanf(pFile, " %c %x,%d", &opIdentifier, &address, &size) > 0) {
        /* get tag field length */
        int t = MACHINEBITS - s - b; 
        /* get opcode set bits and tag bytes */
        unsigned long tag_bits = address >> (b+s);
        unsigned long set_bits = (address << t) >> (t + b);
        printf("op=%c\n",opIdentifier);
        printf("address=%lx\n",address);
        printf("tag_bits=%x\n",tag_bits);
        printf("set_bits=%x\n\n",set_bits);
    }

    fclose(pFile);
    return 0;
}
