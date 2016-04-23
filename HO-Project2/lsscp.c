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

#include "utils.h"
#include "aco.h"
#include "genetic.h"
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
clock_t start_time;
float runtime = 10;

instance_t* inst;
optimal_t* opt;

// Flags
int aco, ga, fi, rep, tour, prop, uniform, fusion, bi = 0;

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
        } else if (strcmp(argv[i], "--ac") == 0) {
            // Number of ants
            ant_count = atoi(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--beta") == 0) {
            // Value for beta
            beta = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--ro") == 0) {
            // Value for ro
            ro = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--epsilon") == 0) {
            // Value for epsilon
            epsilon = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--pops") == 0) {
            // Population size
            population_size = atoi(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--mf") == 0) {
            // M_f
            Mf = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--mc") == 0) {
            // M_c
            Mc = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--mg") == 0) {
            // M_g
            Mg = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--pool") == 0) {
            // Pool size
            pool_size = atoi(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--runtime") == 0) {
            runtime = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--aco") == 0) {
            aco = 1;
        } else if (strcmp(argv[i], "--ga") == 0) {
            ga = 1;
        } else if (strcmp(argv[i], "--fi") == 0) {
            fi = 1;
        } else if (strcmp(argv[i], "--rep") == 0) {
            rep = 1;
        } else if (strcmp(argv[i], "--tour") == 0) {
            tour = 1;
        } else if (strcmp(argv[i], "--prop") == 0) {
            prop = 1;
        } else if (strcmp(argv[i], "--uniform") == 0) {
            uniform = 1;
        } else if (strcmp(argv[i], "--fusion") == 0) {
            fusion = 1;
        } else if (strcmp(argv[i], "--bi") == 0) {
            bi = 1;
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
    inst = mymalloc(sizeof(instance_t));
    
    if (!fp)
        errorExit("ERROR: Error in opening the file.\n");
    if (fscanf(fp,"%d",&inst->m) != 1) /* number of rows */
        errorExit("ERROR: Error reading number of rows.\n");
    if (fscanf(fp,"%d",&inst->n) != 1) /* number of columns */
        errorExit("ERROR: Error reading number of columns.\n");
    // Cost of the n columns
    inst->cost = mymalloc(inst->n * sizeof(int));
    for (j = 0; j < inst->n; j++)
        if (fscanf(fp,"%d",&inst->cost[j]) != 1)
            errorExit("ERROR: Error reading cost.\n");
    
    // Info of columns that cover each row
    inst->col  = mymalloc(inst->m * sizeof(int*));
    inst->ncol = mymalloc(inst->m * sizeof(int));
    for (i = 0; i < inst->m; i++) {
        if (fscanf(fp,"%d",&inst->ncol[i]) != 1)
            errorExit("ERROR: Error reading number of columns.\n");
        inst->col[i] = mymalloc(inst->ncol[i] * sizeof(int));
        for (h = 0; h < inst->ncol[i]; h++) {
            if(fscanf(fp,"%d",&inst->col[i][h]) != 1)
                errorExit("ERROR: Error reading columns.\n");
            inst->col[i][h]--;
        }
    }
    // Info of rows that are covered by each column
    inst->row  = mymalloc(inst->n * sizeof(int*));
    inst->nrow = mymalloc(inst->n * sizeof(int));
    k = mymalloc(inst->n * sizeof(int));
    for (j = 0; j < inst->n; j++) inst->nrow[j] = 0;
    for (i = 0; i < inst->m; i++) {
        for (h = 0; h < inst->ncol[i]; h++)
            inst->nrow[inst->col[i][h]]++;
    }
    for (j = 0; j < inst->n; j++) {
        inst->row[j] = mymalloc(inst->nrow[j] * sizeof(int));
        k[j] = 0;
    }
    for (i = 0; i < inst->m; i++) {
        for (h = 0; h < inst->ncol[i]; h++) {
            inst->row[inst->col[i][h]][k[inst->col[i][h]]] = i;
            k[inst->col[i][h]]++;
        }
    }
    free(k);
}

/*** Exit from code with error message ***/
void errorExit(char* text) {
    printf("%s\n", text);
    exit(EXIT_FAILURE);
}

/*** Initialize other algorithm parameters ***/
void initialize() {
    // Initialize the optimal solution
    opt = mymalloc(sizeof(optimal_t));
    opt->fx = INT32_MAX;
    opt->x = mymalloc(inst->n * sizeof(int));
    for (int i = 0; i < inst->n; i++) opt->x[i] = 0;
    
    if (aco) {
        ACOinitialize(inst);
    } else if (ga) {
        GENinitialize(inst);
    }
}

/*** Use this function to finalize execution **/
void freeInstance(instance_t* inst) {
    for (int i = 0; i < inst->m; i++) free(inst->col[i]);
    for (int i = 0; i < inst->n; i++) free(inst->row[i]);
    free(inst->col);
    free(inst->row);
    free(inst->ncol);
    free(inst->nrow);
    free(inst->cost);
}

void finalize() {
    if (aco) {
        ACOfinalize(inst);
    } else if (ga) {
        GENfinalize(inst);
    }
    free(opt->x);
    free(opt);
    freeInstance(inst);
    free(inst);
}

/*** Sorting functions ***/
// These functions need to be in the main file
// since they require the instance, but cannot
// take it as an additional parameter.
int sortDesc(const void* a, const void* b) {
    int val;
    int cost_a = inst->cost[*(int*) a];
    int cost_b = inst->cost[*(int*) b];
    if (cost_a != cost_b) {
        val = cost_b - cost_a;
    } else {
        int cover_a = inst->nrow[*(int*) a];
        int cover_b = inst->nrow[*(int*) b];
        val = cover_a - cover_b;
    }
    return val;
}

int sortAsc(const void* a, const void* b) {
    int val;
    int cost_a = inst->cost[*(int*) a];
    int cost_b = inst->cost[*(int*) b];
    if (cost_a != cost_b) {
        val = cost_a - cost_b;
    } else {
        int cover_a = inst->nrow[*(int*) a];
        int cover_b = inst->nrow[*(int*) b];
        val = cover_b - cover_a;
    }
    return val;
}

/*** MAIN ***/
int main(int argc, char* argv[]) {
    start_time = clock();
    
    readParameters(argc, argv);
    readSCP(instance_file);
    srand(seed);
    initialize();
    
    if (aco) {
        ACOsolve(inst, opt);
    } else if (ga) {
        GENsolve(inst, opt);
    }
    printf("%d, %f\n", opt->fx, opt->time);
    
    finalize();
    return 0;
}
