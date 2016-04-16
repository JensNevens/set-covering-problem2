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
double* pheromone;

int fi, rep = 0;

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
        } else if (strcmp(argv[i], "--fi") == 0) {
            fi = 1;
        } else if (strcmp(argv[i], "--rep") == 0) {
            rep = 1;
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
    
    // Initialize pheromone trail
    pheromone = mymalloc(inst->n * sizeof(double));
    
    // Initialize the optimal solution
    opt = mymalloc(sizeof(optimal_t));
    opt->fx = INT32_MAX;
    opt->x = mymalloc(inst->n * sizeof(int));
    for (int i = 0; i < inst->n; i++) {
        opt->x[i] = 0;
        pheromone[i] = tau_max;
    }
    
    // Initialize the ant colony
    colony = mymalloc(ant_count * sizeof(ant_t*));
    for (int a = 0; a < ant_count; a++) {
        colony[a] = mymalloc(sizeof(ant_t));
        ant_t* ant = colony[a];
        allocAnt(inst, ant);
        ant->fx = 0;
        ant->un_rows = inst->m;
        for (int i = 0; i < inst->n; i++) ant->x[i] = 0;
        for (int i = 0; i < inst->m; i++) {
            ant->y[i] = 0;
            ant->ncol_cover[i] = 0;
            int k = inst->ncol[i];
            for (int j = 0; j < k; j++) ant->col_cover[i][j] = -1;
        }
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
    free(pheromone);
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
        denom += pheromone[col] * powf(adaptiveCost(ant, col), beta);
    }
    double* probabilities = mymalloc(inst->n * sizeof(double));
    for (int i = 0; i < inst->n; i++) {
        if (columnCovers(inst, i, row)) {
            double nom = pheromone[i] * powf(adaptiveCost(ant, i), beta);
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
// FI needs more time to find optimal solution
// but less iterations, since REP iterates faster
void localSearch(ant_t* ant) {
    if (fi) {
        eliminate(ant);
        firstImprovement(ant);
    } else if (rep) {
        replaceColumns(ant);
    }
}

/*** Local Search method described in Ren et al.'s paper ***/
int computeWj(ant_t* ant, int* Wj, int col) {
    int idx = 0;
    for (int i = 0; i < inst->m; i++) {
        int ncol_cover = ant->ncol_cover[i];
        int thecol = ant->col_cover[i][0];
        if (ncol_cover == 1 && thecol == col) {
            Wj[idx] = i;
            idx += 1;
        }
    }
    return idx;
}

int getLow(int row) {
    int col = -1;
    int colCost = INT32_MAX;
    for (int i = 0; i < inst->ncol[row]; i++) {
        if (inst->cost[inst->col[row][i]] < colCost) {
            col = inst->col[row][i];
            colCost = inst->cost[col];
        }
    }
    return col;
}

// Sort columns with high cost first
// For each column j:
//     Compute Wj (Wj contains the rows that are only covered by column j)
//     If |Wj| = 0:
//         Column j is redundant, remove it;
//     If |Wj| = 1 and cost(j) > cost(low_q): (low_q is the lowest cost column covering row q)
//         Replace j with low_q;
//     If |Wj| = 2 and j != low_q1 == low_q2:
//         Replace j with low_q1 (or low_q2, since they are equal)
//     If |Wj] = 2 and low_q1 != low_q2 and cost(low_q1) + cost(low_q2) < cost(j):
//         Replace j with low_q1 and low_q2
void replaceColumns(ant_t* ant) {
    // Sort columns
    int* sortedCols = mymalloc(inst->n * sizeof(int));
    for (int i = 0; i < inst->n; i++) sortedCols[i] = i;
    qsort(sortedCols, inst->n, sizeof(int), sortDesc);
    
    // Initiliaze Wj
    int* Wj = mymalloc(inst->m * sizeof(int));
    for (int i = 0; i < inst->m; i++) Wj[i] = -1;
    
    // Local Search
    for (int j = 0; j < inst->n; j++) {
        int col = sortedCols[j];
        if (ant->x[col]) {
            int WjCount = computeWj(ant, Wj, col);
            if (WjCount == 0) {
                removeSet(inst, ant, col);
            } else if (WjCount == 1) {
                int low = getLow(Wj[0]);
                if (inst->cost[col] > inst->cost[low]) {
                    removeSet(inst, ant, col);
                    addSet(inst, ant, low);
                }
            } else if (WjCount == 2) {
                int low1 = getLow(Wj[0]);
                int low2 = getLow(Wj[1]);
                if (low1 == low2 && low1 != col) {
                    removeSet(inst, ant, col);
                    addSet(inst, ant, low1);
                } else if (low1 != low2 &&
                           inst->cost[low1] + inst->cost[low2] < inst->cost[col]) {
                    removeSet(inst, ant, col);
                    addSet(inst, ant, low1);
                    addSet(inst, ant, low2);
                }
            }
            // Empty Wj
            for (int i = 0; i < inst->m; i++) Wj[i] = -1;
        }
    }
    // Free memory
    free(sortedCols);
    free(Wj);
}

/*** First Improvement local search method ***/
void firstImprovement(ant_t* ant) {
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

void eliminate(ant_t* ant) {
    int redundant = 1;
    int improvement = 1;
    
    int* sortedCols = mymalloc(inst->n * sizeof(int));
    for (int i = 0; i < inst->n; i++) sortedCols[i] = i;
    qsort(sortedCols, inst->n, sizeof(int), sortDesc);
    
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
    opt->time = computeTime(start_time, clock());
    opt->fx = ant->fx;
    for (int i = 0; i < inst->n; i++) opt->x[i] = ant->x[i];
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
    for (int i = 0; i < inst->n; i++) {
        pheromone[i] = (double) ro * pheromone[i];
        if (opt->x[i]) {
            pheromone[i] += (double) 1 / (double) opt->fx;
        }
        if (pheromone[i] < tau_min) {
            pheromone[i] = tau_min;
        }
        if (pheromone[i] > tau_max) {
            pheromone[i] = tau_max;
        }
    }
}

/*** Clear all ant's solutions ***/
void clearColony() {
    for (int a = 0; a < ant_count; a++) {
        ant_t* ant = colony[a];
        ant->fx = 0;
        ant->un_rows = inst->m;
        for (int i = 0; i < inst->n; i++) ant->x[i] = 0;
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
// 1. Preprocess SCP instance
// 2. Initiliaze pheromone trails and related parameters
// 3. While not termination:
//      For each ant:
//        3.1 Construct a solution
//        3.2 Apply local search
//      3.3 Update optimal solution
//      3.4 Update pheromones
void solve() {
    int iterCount = 0;
    while (computeTime(start_time, clock()) < 10) {
        for (int a = 0; a < ant_count; a++) {
            ant_t* ant = colony[a];
            while (!isSolution(ant)) {
                constructSolution(ant);
            }
            localSearch(ant);
        }
        updateBest();
        updatePheromone();
        clearColony();
        /*printf("Iteration: %d - time elapsed: %f - optimal cost: %d\n",
               iterCount,
               computeTime(start_time, clock()),
               opt->fx);*/
        iterCount++;
    }
}

void test() {
    int ctr = 0;
    while (ctr < 20) {
        for (int a = 0; a < ant_count; a++) {
            ant_t* ant = colony[a];
            while(!isSolution(ant)) {
                constructSolution(ant);
            }
            localSearch(ant);
        }
        updateBest();
        updatePheromone();
        clearColony();
        ctr++;
    }
}

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

// TODO: Implement preprocess?
// TODO: Modulerize code
// TODO: Add additional local search methods (BI)?
// TODO: Add additional constructive methods (ch1, ch2, ch3, ch4)?

int main(int argc, char* argv[]) {
    readParameters(argc, argv);
    readSCP(instance_file);
    srand(seed);
    initialize();
    
    start_time = clock();
    solve();
    printf("%d-%f\n", opt->fx, opt->time);
    
    finalize();
    return 0;
}
