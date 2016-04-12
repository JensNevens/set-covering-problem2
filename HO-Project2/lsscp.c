//
//  lsscp.c
//  HO-Project2
//
//  Created by Jens Nevens on 12/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "lsscp.h"

/***********************/
/** For this code:    **/
/**   rows = elements **/
/**   cols = subsets  **/
/***********************/

/********************
 Algorithm parameters
 *******************/
int seed = 1234567;
char* instance_file = "";
char* output_file = "output.txt";
instance_t* inst;

/*** Read parameters from command line ***/
void readParameters(int argc, char* argv[]) {
    int i;
    if (argc <= 1) {
        exit(EXIT_FAILURE);
    }
    for(i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0) {
            seed = atoi(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--instance") == 0) {
            instance_file = argv[i+1];
            i += 1;
        } else if (strcmp(argv[i], "--output") == 0) {
            output_file = argv[i+1];
            i += 1;
        } else {
            printf("ERROR: parameter %s not recognized.\n", argv[i]);
            exit(EXIT_FAILURE);
        }
    }
    if((instance_file == NULL) || ((instance_file != NULL) && (instance_file[0] == '\0'))) {
        exit(EXIT_FAILURE);
    }
}

/*** Read instance in the OR-LIBRARY format ***/
void readSCP(char* filename) {
    int h,i,j;
    int* k;
    FILE* fp = fopen(filename, "r");
    inst = (instance_t*) mymalloc(sizeof(instance_t));
    
    if (!fp)
        errorExit("ERROR: Error in opening the file.\n");
    if (fscanf(fp,"%d",&inst->m) != 1) /* number of rows */
        errorExit("ERROR: Error reading number of rows.\n");
    if (fscanf(fp,"%d",&inst->n) != 1) /* number of columns */
        errorExit("ERROR: Error reading number of columns.\n");
    // Cost of the n columns
    inst->cost = (int*) mymalloc(inst->n * sizeof(int));
    for (j = 0; j < inst->n; j++)
        if (fscanf(fp,"%d",&inst->cost[j]) != 1)
            errorExit("ERROR: Error reading cost.\n");
    
    // Info of columns that cover each row
    inst->col  = (int**) mymalloc(inst->m * sizeof(int*));
    inst->ncol = (int*) mymalloc(inst->m * sizeof(int));
    for (i = 0; i < inst->m; i++) {
        if (fscanf(fp,"%d",&inst->ncol[i]) != 1)
            errorExit("ERROR: Error reading number of columns.\n");
        inst->col[i] = (int *) mymalloc(inst->ncol[i] * sizeof(int));
        for (h = 0; h < inst->ncol[i]; h++) {
            if(fscanf(fp,"%d",&inst->col[i][h]) != 1)
                errorExit("ERROR: Error reading columns.\n");
            inst->col[i][h]--;
        }
    }
    // Info of rows that are covered by each column
    inst->row  = (int**) mymalloc(inst->n * sizeof(int*));
    inst->nrow = (int*) mymalloc(inst->n * sizeof(int));
    k    = (int*) mymalloc(inst->n * sizeof(int));
    for (j = 0; j < inst->n; j++) inst->nrow[j] = 0;
    for (i = 0; i < inst->m; i++) {
        for (h = 0; h < inst->ncol[i]; h++)
            inst->nrow[inst->col[i][h]]++;
    }
    for (j = 0; j < inst->n; j++) {
        inst->row[j] = (int *) mymalloc(inst->nrow[j] * sizeof(int));
        k[j] = 0;
    }
    for (i = 0; i < inst->m; i++) {
        for (h = 0; h < inst->ncol[i]; h++) {
            inst->row[inst->col[i][h]][k[inst->col[i][h]]] = i;
            k[inst->col[i][h]]++;
        }
    }
    free((void*) k);
}

/*** Exit from code with error message ***/
void errorExit(char* text) {
    printf("%s\n", text);
    exit(EXIT_FAILURE);
}

/*** Allocate memory ***/
void* mymalloc(size_t size) {
    void *s;
    if ((s=malloc(size)) == NULL) {
        fprintf(stderr, "malloc : Not enough memory.\n");
        exit(EXIT_FAILURE);
    }
    return s;
}

/*** Initialize other algorithm parameters ***/
void initialize() {}

/*** Use this function to finalize execution **/
void finalize() {
    free((void*) inst);
}

int main(int argc, char* argv[]) {
    readParameters(argc, argv);
    readSCP(instance_file);
    initialize();
    srand(seed);
    // Do some stuff here!
    finalize();
    return 0;
}
