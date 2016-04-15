//
//  utils.c
//  HO-Project2
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "utils.h"

/*** Allocate memory ***/
void* mymalloc(size_t size) {
    void *s;
    if ((s=malloc(size)) == NULL) {
        fprintf(stderr, "malloc : Not enough memory.\n");
        exit(EXIT_FAILURE);
    }
    return s;
}

/*** Computes runtime of algorithm ***/
double computeTime(clock_t start, clock_t end) {
    return (double) (end - start)/CLOCKS_PER_SEC;
}

/*** Computes cost of all columns in the instance ***/
int totalCost(instance_t* inst) {
    int total = 0;
    for (int i = 0; i < inst->n; i++) {
        total += inst->cost[i];
    }
    return total;
}

/*** Checks if column 'col' covers element 'row' ***/
int columnCovers(instance_t* inst, int col, int row) {
    for (int i = 0; i < inst->nrow[col]; i++) {
        if (row == inst->row[col][i]) {
            return 1;
        }
    }
    return 0;
}

/*** Pick a random number in range [min,max] ***/
unsigned int pickRandom(unsigned int min, unsigned int max) {
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;
    
    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do {
        r = rand();
    } while (r >= limit);
    
    return min + (r / buckets);
}

/*** Select a random element according to a probability density function ***/
int randomFromPDF(double* probabilities, int len) {
    double r = (double) rand() / (double) RAND_MAX;
    double cummulative = 0;
    int idx;
    for (int i = 0; i < len; i++) {
        cummulative += probabilities[i];
        if (r <= cummulative) {
            idx = i;
            break;
        }
    }
    return idx;
}
