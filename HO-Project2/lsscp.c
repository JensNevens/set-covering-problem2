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
#include "ant.h"
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

int ant_count = 20;
float beta = 5.0;
float ro = 0.99;
float epsilon = 0.005;
float tau_min = 0;
float tau_max = 0;

instance_t* inst;
ant_t** colony;

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

/*** Prints diagnostic information about the solution **/
void diagnostics(ant_t* ant) {
    for (int i = 0; i < inst->m; i++) {
        if (ant->y[i]) {
            printf("ELEMENT %d COVERED BY %d SET(S)\n", i, ant->ncol_cover[i]);
            for (int j = 0; j < ant->ncol_cover[i]; j++) {
                if (ant->col_cover[i][j] < 0) {
                    printf("---\n");
                    break;
                } else {
                    printf("---SET %d\n", ant->col_cover[i][j]);
                }
            }
        } else {
            printf("ELEMENT %d NOT COVERED\n", i);
        }
    }
}

/*** Initialize other algorithm parameters ***/
void initialize() {
    // Compute tau_max ant tau_min
    int cost = totalCost(inst);
    tau_max = (float) 1 / (float) ((1 - ro) * cost);
    tau_min = epsilon * tau_max;
    
    // Initialize the ant colony
    colony = (ant_t**) mymalloc(ant_count * sizeof(ant_t*));
    for (int a = 0; a < ant_count; a++) {
        colony[a] = (ant_t*) mymalloc(sizeof(ant_t));
        ant_t* ant = colony[a];
        ant->fx = 0;
        ant->un_rows = inst->m;
        ant->x = (int*) mymalloc(inst->n * sizeof(int));
        ant->y = (int*) mymalloc(inst->m * sizeof(int));
        ant->col_cover = (int**) mymalloc(inst->m * sizeof(int*));
        ant->ncol_cover = (int*) mymalloc(inst->m * sizeof(int));
        ant->pheromone = (float*) mymalloc(inst->n * sizeof(float));
        for (int i = 0; i < inst->n; i++) {
            ant->x[i] = 0;
            ant->pheromone[i] = tau_max;
        }
        for (int i = 0; i < inst->m; i++) {
            ant->y[i] = 0;
            ant->ncol_cover[i] = 0;
            int k = inst->ncol[i];
            ant->col_cover[i] = (int*) mymalloc(k * sizeof(int));
            for (int j = 0; j < k; j++) {
                ant->col_cover[i][j] = -1;
            }
        }
    }
}

/*** Use this function to finalize execution **/
void finalize() {
    for (int a = 0; a < ant_count; a++) {
        ant_t* ant = colony[a];
        free((void*) ant->x);
        free((void*) ant->y);
        free((void*) ant->col_cover);
        free((void*) ant->ncol_cover);
        free((void*) ant->pheromone);
        free((void*) ant);
    }
    free((void*) colony);
}


/****************/
/** ACO-Method **/
/****************/

/** Heuristic information methods **/
float adaptiveCost(ant_t* ant, int col) {
    unsigned int covers = 0;
    for (int i = 0; i < inst->nrow[col]; i++) {
        if (!ant->y[inst->row[col][i]]) {
            covers += 1;
        }
    }
    return (float) covers / (float) inst->cost[col];
}

/** Constructive methods **/
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
int randomFromPDF(float* probabilities, int len) {
    float r = (float) rand() / (float) RAND_MAX;
    float cummulative = 0;
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

/*** Construct a solution ***/
// 4.1 SROM:
// Randomly select an uncovered row i
// From the set of columns covering row i,
// select column j with probability:
//   pheromone[j]*(heuristic[j]**beta) / Sum_q (pheromone[q]*(heuristic[q]**beta))
// and add column j to the solution.
void constructSolution(ant_t* ant) {
    int row = -1;
    while (row < 0) {
        int idx = pickRandom(0, inst->m-1);
        if (!ant->y[idx]) {
            row = idx;
        }
    }
    
    float denom = 0;
    for (int i = 0; i < inst->ncol[row]; i++) {
        int col = inst->col[row][i];
        denom += ant->pheromone[col] * powf(adaptiveCost(ant, col), beta);
    }
    float* probabilities = (float*) mymalloc(inst->n * sizeof(float));
    for (int i = 0; i < inst->n; i++) {
        if (columnCovers(inst, i, row)) {
            float nom = ant->pheromone[i] * powf(adaptiveCost(ant, i), beta);
            probabilities[i] = nom / denom;
        } else {
            probabilities[i] = 0;
        }
    }
    
    int col = -1;
    while (col < 0) {
        int idx = randomFromPDF(probabilities, inst->n);
        if (!ant->x[idx]) {
            col = idx;
        }
    }
    addSet(inst, ant, col);
    free((void*) probabilities);
}

/** Local Search methods **/
void initAnt(ant_t* new, ant_t* old) {
    new->fx = old->fx;
    new->un_rows = old->un_rows;
    new->x = (int*) mymalloc(inst->n * sizeof(int));
    new->y = (int*) mymalloc(inst->m * sizeof(int));
    new->col_cover = (int**) mymalloc(inst->m * sizeof(int*));
    new->ncol_cover = (int*) mymalloc(inst->m * sizeof(int));
    new->pheromone = (float*) mymalloc(inst->n * sizeof(float));
    for (int i = 0; i < inst->n; i++) {
        new->x[i] = old->x[i];
        new->pheromone[i] = old->pheromone[i];
    }
    for (int i = 0; i < inst->m; i++) {
        new->y[i] = old->x[i];
        new->ncol_cover[i] = old->ncol_cover[i];
        int k = inst->ncol[i];
        new->col_cover[i] = (int*) mymalloc(k * sizeof(int));
        for (int j = 0; j < k; j++) {
            new->col_cover[i][j] = old->col_cover[i][j];
        }
    }
}

void copyAnt(ant_t* from, ant_t* to) {
    to->fx = from->fx;
    to->un_rows = from->un_rows;
    for (int i = 0; i < inst->n; i++) {
        to->x[i] = from->x[i];
        to->pheromone[i] = from->pheromone[i];
    }
    for (int i = 0; i < inst->m; i++) {
        to->y[i] = from->y[i];
        to->ncol_cover[i] = from->ncol_cover[i];
        for (int j = 0; j < inst->ncol[i]; j++) {
            to->col_cover[i][j] = from->col_cover[i][j];
        }
    }
}

/*** First Improvement local search method ***/
void localSearch(ant_t* ant) {
    int improvement = 1;
    ant_t* antcpy = (ant_t*) mymalloc(sizeof(ant_t));
    initAnt(antcpy, ant);
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (antcpy->x[i]) {
                removeSet(inst, antcpy, i);
                while (!isSolution(antcpy)) {
                    constructSolution(antcpy);
                }
                if (antcpy->fx < ant->fx) {
                    copyAnt(antcpy, ant);
                    improvement = 1;
                    eliminate(ant);
                } else {
                    copyAnt(ant, antcpy);
                }
            }
        }
    }
    free((void*) antcpy);
}

/*** Redundancy elimination ***/
int cmp(const void* a, const void* b) {
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

void eliminate(ant_t* ant) {
    int redundant = 1;
    int improvement = 1;
    
    int* sortedCols = (int*) mymalloc(inst->n * sizeof(int));
    for (int i = 0; i < inst->n; i++) {
        sortedCols[i] = i;
    }
    qsort(sortedCols, inst->n, sizeof(int), cmp);
    
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            int col = sortedCols[i];
            if (ant->x[col]) {
                redundant = 1;
                for (int j = 0; j < inst->nrow[col]; j++) {
                    int row = inst->row[col][j];
                    if (ant->ncol_cover[row] <= 1) {
                        redundant = 0;
                        break;
                    }
                }
                if (redundant) {
                    removeSet(inst, ant, col);
                    improvement = 1;
                }
            }
        }
    }
    free((void*) sortedCols);
}

/** Check if there is a new best solution **/
void updateBest() {}

/** Pheromone update methods **/
void updatePheromone() {}

void solve() {
    while (computeTime(start_time, clock()) < 120.0) {
        for (int a = 0; a < ant_count; a++) {
            ant_t* ant = colony[a];
            while (!isSolution(ant)) {
                constructSolution(ant);
            }
            eliminate(ant);
            localSearch(ant);
        }
        updateBest();
        updatePheromone();
    }
}

void test() {
    ant_t* ant = mymalloc(sizeof(ant_t));
    ant->fx = 0;
    ant->un_rows = inst->m;
    ant->x = (int*) mymalloc(inst->n * sizeof(int));
    ant->y = (int*) mymalloc(inst->m * sizeof(int));
    ant->col_cover = (int**) mymalloc(inst->m * sizeof(int*));
    ant->ncol_cover = (int*) mymalloc(inst->m * sizeof(int));
    ant->pheromone = (float*) mymalloc(inst->n * sizeof(float));
    for (int i = 0; i < inst->n; i++) {
        ant->x[i] = 0;
        ant->pheromone[i] = tau_max;
    }
    for (int i = 0; i < inst->m; i++) {
        ant->y[i] = 0;
        ant->ncol_cover[i] = 0;
        int k = inst->ncol[i];
        ant->col_cover[i] = (int*) mymalloc(k * sizeof(int));
        for (int j = 0; j < k; j++) {
            ant->col_cover[i][j] = -1;
        }
    }
    while (!isSolution(ant)) {
        constructSolution(ant);
    }
    printf("Cost of solution: %d\n", ant->fx);
    eliminate(ant);
    printf("Cost of solution after RE: %d\n", ant->fx);
    localSearch(ant);
    printf("Cost of solution after FI: %d\n", ant->fx);
}

// 1. (Preprocess SCP instance)
// 2. (Langrangian multiplier with subgradient method)
// 3. Initiliaze pheromone trails and related parameters
// 4. While not termination:
//      For each ant:
//        4.1 Construct a solution (SROM)
//        4.2 Apply local search
//      4.3 Update pheromones
//      (If best solution not improved for p iterations:
//        4.4 New Langrangian multiplier with subgradient method)
// 5. Return the best solution

// 1. Preprocess:
// For each column j:
//   lows = [] (size = #rows covered by j)
//   For each row i covered by column j:
//      For each column k that covers row i:
//         Get the lowest column k, i.e. low_i
//         Add low_i to lows
//   If sum(cost(lows)) < cost(j), delete column j
//   Re-initialize lows

int main(int argc, char* argv[]) {
    start_time = clock();
    readParameters(argc, argv);
    readSCP(instance_file);
    srand(seed);
    initialize();
    // Do some stuff here!
    test();
        
    finalize();
    return 0;
}
