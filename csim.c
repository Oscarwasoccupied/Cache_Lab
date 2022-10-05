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

char traceFile[200]; /* trace file path*/
int s; /* s: S=2^s is the set number */
int E; /* E: num of lines in each set */
int b; /* b: B=2^b is the size of each block in bytes */

/* store num of hits, miss, eviction miss, dirty bits and dirty evictions */
csim_stats_t cache_stats = {0,0,0,0,0};

int getCli(int argc, char **argv, int *s, int *E, int *b, char *traceFile);

int main(int argc, char *argv[]) {
    /* set the parameter s E b t from the command line input */
    getCli(argc, argv, &s, &E, &b, traceFile);
    
    /* ************ TO DO ************ */
    /* check if the s,B,b parameter is valid */

    /* ************ TO DO ************ */
    /* initialize the cache */

    /* ************ TO DO ************ */
    /* read the trace file from t */

    /* ************ TO DO ************ */
    /* free the cache */

    /* ************ TO DO ************ */
    /* print summary about hit miss eviction */
    printSummary(&cache_stats);

}

/**
 * Description: 
 *     Read command from command line,
 *     and then set parameter for the cache simulator
 * @param 
 * @return 
 */
int getCli(int argc, char **argv, int *s, int *E, int *b, char *traceFile){
    int opt;
    while(-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
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
                /* ******** to do verbose ********* */
            case 'h':
                /* ******** to do help ************ */
                break;
            default:
                /* ******** to do default ********* */
                break;
        }
        return 0;
    }
}
