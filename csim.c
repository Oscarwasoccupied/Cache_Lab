/* *************************************************
 * @ File Name: csim.c
 * @ Author: Xianwei Zou
 * @ Version: 0.0.0
 * @ Description: Simulate the behavior of a cache
 * *************************************************/

#include "cachelab.h"
#include <unistd.h>  /* getopt() */
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char t[200]; /* trace file path*/
int s; /* s: S=2^s is the set number */
int E; /* E: num of lines in each set */
int b; /* b: B=2^b is the size of each block in bytes */

int getCli(int argc, char **argv, int *s, int *E, int *b, char *t);

int main(int argc, char *argv[]) {
    /* set the parameter s E b t from the command line input*/
    getCli(argc, argv, &s, &E, &b, t); 

    /* ************ TO DO ************ */
    /* check if the s,B,b parameter is valid*/

    /*
}

/* ***************************************************
 * Description: 
 *     Read command from command line,
 *     and then set parameter for the cache simulator
 * @param 
 * @return 
 * ***************************************************/
int getCli(int argc, char **argv, int *s, int *E, int *b, char *t){
    int opt;
    while(-1 != (opt = optget(argc, argv, "hvs:E:b:t:"))) {
        switch (opt) {
            case 's':
                s = atoi(optarg); /* convert s from string to int*/
                break;
            case 'E':
                E = atoi(optarg); /* convert E from string to int*/
                break;
            case 'b':
                b = atoi(optarg); /* convert b from string to int*/
                break;
            case 't':
                strcpy(t, optarg); /* copy the trace file path to t*/
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
