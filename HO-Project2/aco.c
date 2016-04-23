//
//  aco.c
//  HO-Project2
//
//  Created by Jens Nevens on 16/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "solution.h"
#include "lsscp.h"
#include "iterative.h"
#include "aco.h"

/********************/
/** ACO Parameters **/
/********************/
int ant_count = 20;
double beta = 5.0;
double ro = 0.99;
double epsilon = 0.005;
double tau_min = 0;
double tau_max = 0;

ant_t** colony;
double* pheromone;

/********************/
/** init and final **/
/********************/
void ACOinitialize(instance_t* inst) {
    // Compute tau_max ant tau_min
    int cost = totalCost(inst);
    tau_max = (double) 1 / (double) ((1 - ro) * cost);
    tau_min = epsilon * tau_max;
    
    // Initialize pheromone trail
    pheromone = mymalloc(inst->n * sizeof(double));
    for (int i = 0; i < inst->n; i++) pheromone[i] = tau_max;
    
    // Initialize the ant colony
    colony = mymalloc(ant_count * sizeof(ant_t*));
    for (int a = 0; a < ant_count; a++) {
        colony[a] = mymalloc(sizeof(ant_t));
        ant_t* ant = colony[a];
        allocSolution(inst, ant);
        initSolution(inst, ant);
    }
}

void ACOfinalize(instance_t* inst) {
    for (int a = 0; a < ant_count; a++) {
        ant_t* ant = colony[a];
        freeSolution(inst, ant);
        free(ant);
    }
    free(colony);
    free(pheromone);
}

/*****************/
/** ACO-Methods **/
/*****************/

/** Heuristic information **/
double heuristicValue(instance_t* inst, ant_t* ant, int col) {
    int covers = 0;
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
void constructAnt(instance_t* inst, ant_t* ant) {
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
        denom += pheromone[col] * powf(heuristicValue(inst, ant, col), beta);
    }
    double* probabilities = mymalloc(inst->n * sizeof(double));
    for (int i = 0; i < inst->n; i++) {
        if (columnCovers(inst, i, row)) {
            double nom = pheromone[i] * powf(heuristicValue(inst, ant, i), beta);
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
void localSearchACO(instance_t* inst, ant_t* ant) {
    if (fi) {
        eliminate(inst, ant);
        firstImprovement(inst, ant);
    } else if (rep) {
        replaceColumns(inst, ant);
    }
}

/*** Local Search method described in Ren et al.'s paper ***/
int computeWj(instance_t* inst, ant_t* ant, int* Wj, int col) {
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

int getLow(instance_t* inst, int row) {
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
void replaceColumns(instance_t* inst, ant_t* ant) {
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
            int WjCount = computeWj(inst, ant, Wj, col);
            if (WjCount == 0) {
                removeSet(inst, ant, col);
            } else if (WjCount == 1) {
                int low = getLow(inst, Wj[0]);
                if (inst->cost[col] > inst->cost[low]) {
                    removeSet(inst, ant, col);
                    addSet(inst, ant, low);
                }
            } else if (WjCount == 2) {
                int low1 = getLow(inst, Wj[0]);
                int low2 = getLow(inst, Wj[1]);
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

/** Check if there is a new best solution **/
void updateTau(optimal_t* opt) {
    tau_max = (double) 1 / (double) ((1 - ro) * opt->fx);
    tau_min = (double) epsilon * tau_max;
}

void updateBestAnt(instance_t* inst, optimal_t* opt) {
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
        updateOptimal(inst, opt, colony[bestAnt]);
        updateTau(opt);
    }
}

/*** Pheromone update methods **/
void updatePheromone(instance_t* inst, optimal_t* opt) {
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
void clearColony(instance_t* inst) {
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
// TODO: Add additional local search methods (BI)?
// TODO: Add additional constructive methods (ch1, ch2, ch3, ch4)?

/*** General methods ***/
// 1. Preprocess SCP instance
// 2. Initiliaze pheromone trails and related parameters
// 3. While not termination:
//      For each ant:
//        3.1 Construct a solution
//        3.2 Apply local search
//      3.3 Update optimal solution
//      3.4 Update pheromones
void ACOsolve(instance_t* inst, optimal_t* opt) {
    int iterCount = 0;
    while (computeTime(start_time, clock()) < runtime) {
        for (int a = 0; a < ant_count; a++) {
            ant_t* ant = colony[a];
            while (!isSolution(ant)) {
                constructAnt(inst, ant);
            }
            localSearchACO(inst, ant);
        }
        updateBestAnt(inst, opt);
        updatePheromone(inst, opt);
        clearColony(inst);
        /*printf("Iteration: %d - time elapsed: %f - optimal cost: %d\n",
               iterCount,
               computeTime(start_time, clock()),
               opt->fx);*/
        iterCount++;
        if (qrtd) {
            if (opt->fx <= maxcost) {
                break;
            }
        }
    }
}




