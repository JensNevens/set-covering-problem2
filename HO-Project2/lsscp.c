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
double beta = 5.0;
double ro = 0.99;
double epsilon = 0.005;
double tau_min = 0;
double tau_max = 0;

instance_t* inst;
ant_t** colony;
optimal_t* opt;

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
            ant_count = atoi(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--beta") == 0) {
            beta = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--ro") == 0) {
            ro = atof(argv[i+1]);
            i += 1;
        } else if (strcmp(argv[i], "--epsilon") == 0) {
            epsilon = atof(argv[i+1]);
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
    // Compute tau_max ant tau_min
    int cost = totalCost(inst);
    tau_max = (double) 1 / (double) ((1 - ro) * cost);
    tau_min = epsilon * tau_max;
    
    // Initialize the optimal solution
    opt = mymalloc(sizeof(optimal_t));
    opt->fx = INT32_MAX;
    opt->x = mymalloc(inst->n * sizeof(int));
    for (int i = 0; i < inst->n; i++) {
        opt->x[i] = 0;
    }
    
    // Initialize the ant colony
    colony = mymalloc(ant_count * sizeof(ant_t*));
    for (int a = 0; a < ant_count; a++) {
        colony[a] = mymalloc(sizeof(ant_t));
        ant_t* ant = colony[a];
        allocAnt(inst, ant);
        ant->fx = 0;
        ant->un_rows = inst->m;
        for (int i = 0; i < inst->n; i++) {
            ant->x[i] = 0;
            ant->pheromone[i] = tau_max;
        }
        for (int i = 0; i < inst->m; i++) {
            ant->y[i] = 0;
            ant->ncol_cover[i] = 0;
            int k = inst->ncol[i];
            for (int j = 0; j < k; j++) {
                ant->col_cover[i][j] = -1;
            }
        }
    }
}

/*** Use this function to finalize execution **/
void freeInstance(instance_t* inst) {
    for (int i = 0; i < inst->m; i++) {
        free(inst->col[i]);
    }
    for (int i = 0; i < inst->n; i++) {
        free(inst->row[i]);
    }
    free(inst->col);
    free(inst->row);
    free(inst->ncol);
    free(inst->nrow);
    free(inst->cost);
}

void finalize() {
    for (int a = 0; a < ant_count; a++) {
        ant_t* ant = colony[a];
        freeAnt(inst, ant);
        free(ant);
    }
    free(colony);
    freeInstance(inst);
    free(inst);
    free(opt->x);
    free(opt);
}


/****************/
/** ACO-Method **/
/****************/

/** Heuristic information methods **/
double adaptiveCost(ant_t* ant, int col) {
    unsigned int covers = 0;
    for (int i = 0; i < inst->nrow[col]; i++) {
        if (!ant->y[inst->row[col][i]]) {
            covers += 1;
        }
    }
    return (double) covers / (double) inst->cost[col];
}

/** Constructive methods **/
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
    
    double denom = 0;
    for (int i = 0; i < inst->ncol[row]; i++) {
        int col = inst->col[row][i];
        denom += ant->pheromone[col] * powf(adaptiveCost(ant, col), beta);
    }
    double* probabilities = mymalloc(inst->n * sizeof(double));
    for (int i = 0; i < inst->n; i++) {
        if (columnCovers(inst, i, row)) {
            double nom = ant->pheromone[i] * powf(adaptiveCost(ant, i), beta);
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
    free(probabilities);
}

/** Local Search methods **/
/*** First Improvement local search method ***/
void localSearch(ant_t* ant) {
    int improvement = 1;
    ant_t* antcpy = mymalloc(sizeof(ant_t));
    allocAnt(inst, antcpy);
    copyAnt(inst, ant, antcpy);
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (antcpy->x[i]) {
                removeSet(inst, antcpy, i);
                while (!isSolution(antcpy)) {
                    constructSolution(antcpy);
                }
                if (antcpy->fx < ant->fx) {
                    copyAnt(inst, antcpy, ant);
                    improvement = 1;
                    eliminate(ant);
                } else {
                    copyAnt(inst, ant, antcpy);
                }
            }
        }
    }
    freeAnt(inst, antcpy);
    free(antcpy);
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
    
    int* sortedCols = mymalloc(inst->n * sizeof(int));
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
    free(sortedCols);
}

/** Check if there is a new best solution **/
void updateOptimal(ant_t* ant) {
    opt->fx = ant->fx;
    for (int i = 0; i < inst->n; i++) {
        opt->x[i] = ant->x[i];
    }
    printf("New optimal cost is %d\n", opt->fx);
}

void updateTau(optimal_t* opt) {
    tau_max = (double) 1 / (double) ((1 - ro) * opt->fx);
    tau_min = (double) epsilon * tau_max;
}

void updateBest() {
    int bestAnt = -1;
    int bestAntCost = INT32_MAX;
    for (int a = 0; a < ant_count; a++) {
        ant_t* ant = colony[a];
        if (ant->fx < bestAntCost) {
            bestAnt = a;
            bestAntCost = ant->fx;
        }
    }
    if (bestAntCost < opt->fx) {
        updateOptimal(colony[bestAnt]);
        updateTau(opt);
    }
}

/*** Pheromone update methods **/
void updatePheromone() {
    for (int a = 0; a < ant_count; a++) {
        ant_t* ant = colony[a];
        for (int i = 0; i < inst->n; i++) {
            ant->pheromone[i] = (double) ro * ant->pheromone[i];
            if (opt->x[i]) {
                ant->pheromone[i] += (double) 1 / (double) opt->fx;
            }
            if (ant->pheromone[i] < tau_min) {
                ant->pheromone[i] = tau_min;
            }
            if (ant->pheromone[i] > tau_max) {
                ant->pheromone[i] = tau_max;
            }
        }
    }
}

/*** Clear all ant's solutions ***/
// Clear the ant's solution, except the pheromone!
void clearColony() {
    for (int a = 0; a < ant_count; a++) {
        ant_t* ant = colony[a];
        ant->fx = 0;
        ant->un_rows = inst->m;
        for (int i = 0; i < inst->n; i++) {
            ant->x[i] = 0;
        }
        for (int i = 0; i < inst->m; i++) {
            ant->y[i] = 0;
            ant->ncol_cover[i] = 0;
            for (int j = 0; j < inst->ncol[i]; j++) {
                ant->col_cover[i][j] = -1;
            }
        }
    }
}


/*** General methods ***/
void solve() {
    int iterCount = 0;
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
        clearColony();
        printf("Iteration %d, time elapsed: %f\n",
               iterCount,
               computeTime(start_time, clock()));
        iterCount++;
    }
}

// Update test for colony, in stead of single ant
void test() {
    int ctr = 0;
    while (ctr < 20) {
        for (int a = 0; a < ant_count; a++) {
            ant_t* ant = colony[a];
            while(!isSolution(ant)) {
                constructSolution(ant);
            }
            eliminate(ant);
            localSearch(ant);
        }
        updateBest();
        updatePheromone();
        clearColony();
        ctr++;
    }
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
// -
// For each row i:
//   if only 1 column covers row i:
//      Add this column to the solution of each ant

// TODO: Modulerize code
// TODO: Implement preprocess + add flag (--pre)
// TODO: Also add BI (and local search from paper)

int main(int argc, char* argv[]) {
    readParameters(argc, argv);
    readSCP(instance_file);
    srand(seed);
    initialize();
    // Do some stuff here!
    start_time = clock();
    solve();
        
    finalize();
    return 0;
}
