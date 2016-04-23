//
//  genetic.c
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
#include "iterative.h"
#include "genetic.h"

// Algorithm outline
// 1. Generate initial population
// 2. Evaluate fitness of population
// 3. Loop until satisfactory solution found:
//     3.1 Select suitable parents from population
//     3.2 Recombine parents to produce children
//         3.2.1 Crossover operators
//         3.2.2 Mutation operators
//         3.2.3 Feasibility operators
//     3.3 Evaluate fitness of children
//     3.4 Replace (some or all) population by children

// Detailed comments can be find in the corresponding .h file
// (genetic.h)

int offspring_size = 1000;
int population_size = 100;
double Mf = 10;
double Mc = 200;
double Mg = 2;
int offspring_count = 0;
int pool_size = 4;

inidividual_t** population;

/********************/
/** init and final **/
/********************/
void GENinitialize(instance_t* inst) {
    population = mymalloc(population_size * sizeof(inidividual_t*));
    for (int c = 0; c < population_size; c++) {
        population[c] = mymalloc(sizeof(inidividual_t));
        inidividual_t* indv = population[c];
        allocSolution(inst, indv);
        initSolution(inst, indv);
    }
}

void GENfinalize(instance_t* inst) {
    for (int c = 0; c < population_size; c++) {
        inidividual_t* indv = population[c];
        freeSolution(inst, indv);
        free(indv);
    }
    free(population);
}

/****************/
/** GA Methods **/
/****************/

/*** Construct a single solution ***/
void constructIndv(instance_t* inst, inidividual_t* indv) {
    int row = -1;
    while (row < 0) {
        int idx = pickRandom(0, inst->m-1);
        if (!indv->y[idx]) {
            row = idx;
        }
    }
    int col = -1;
    while (col < 0) {
        int idx = pickRandom(0, inst->ncol[row]-1);
        int tmp = inst->col[row][idx];
        if (!indv->x[tmp]) {
            col = tmp;
        }
    }
    addSet(inst, indv, col);
}

/*** Parent selection ***/
void selectParents(int* parents) {
    if (tour) {
        tournamentSelection(parents);
    } else if (prop) {
        proportionateSelection(parents);
    }
}

/*** Parent selection (tournament) ***/
void tournamentSelection(int* parents) {
    int* candidates = mymalloc(2 * pool_size * sizeof(int));
    for (int i = 0; i < 2 * pool_size; i++) {
        // TODO: There is no check for duplicates
        candidates[i] = pickRandom(0, population_size-1);
    }
    
    for (int i = 0; i < 2; i++) {
        int indv = -1;
        int indvCost = INT32_MAX;
        for (int j = i * pool_size; j < i+1 * pool_size; j++) {
            if (population[candidates[j]]->fx < indvCost) {
                indv = candidates[j];
                indvCost = population[candidates[j]]->fx;
            }
        }
        parents[i] = indv;
    }
    free(candidates);
}

/*** Parent selection (proportionate) ***/
void proportionateSelection(int* parents) {
    double* probabilities = mymalloc(population_size * sizeof(double));
    
    int denom = 0;
    for (int c = 0; c < population_size; c++) {
        denom += population[c]->fx;
    }
    
    for (int c = 0; c < population_size; c++) {
        probabilities[c] = (double) population[c]->fx / (double) denom;
    }
    
    for (int i = 0; i < 2; i++) {
        parents[i] = randomFromPDF(probabilities, population_size);
    }
    free(probabilities);
}

/*** Crossover operators ***/
void crossover(instance_t* inst, int parent1, int parent2, inidividual_t* child) {
    if (uniform) {
        uniformCross(inst, parent1, parent2, child);
    } else if (fusion) {
        fusionCross(inst, parent1, parent2, child);
    }
}

/*** Uniform crossover ***/
void uniformCross(instance_t* inst, int parent1, int parent2, inidividual_t* child) {
    inidividual_t* par1 = population[parent1];
    inidividual_t* par2 = population[parent2];
    
    for (int i = 0; i < inst->n; i++) {
        int bit = pickRandom(0, 1);
        if (!bit && par1->x[i]) {
            addSet(inst, child, i);
        } else if (bit && par2->x[i]) {
            addSet(inst, child, i);
        }
    }
}

/*** Fusion crossover ***/
void fusionCross(instance_t* inst, int parent1, int parent2, inidividual_t* child) {
    inidividual_t* par1 = population[parent1];
    inidividual_t* par2 = population[parent2];
    double* probabilities = mymalloc(2 * sizeof(double));
    probabilities[0] = (double) par2->fx / (double) (par1->fx + par2->fx);
    probabilities[1] = (double) 1 - probabilities[0];
    
    for (int i = 0; i < inst->n; i++) {
        if (par1->x[i] == par2->x[i] && par1->x[i]) {
            addSet(inst, child, i);
        } else {
            int bit = randomFromPDF(probabilities, 2);
            if (!bit && par1->x[i]) {
                addSet(inst, child, i);
            } else if (bit && par2->x[i]) {
                addSet(inst, child, i);
            }
        }
    }
    free(probabilities);
}

/*** Mutation operator ***/
void mutate(instance_t* inst, inidividual_t* child) {
    int mutateCount = (int) ceil((double) Mf / (double) (1 + exp(-4 * Mg * (offspring_count - Mc)/Mf)));
    for (int i = 0; i < (int) ceil(mutateCount); i++) {
        int col = pickRandom(0, inst->n-1);
        if (child->x[col]) {
            removeSet(inst, child, col);
        } else {
            addSet(inst, child, col);
        }
    }
}

/*** Make the Individual a valid solution ***/
double coverCost(instance_t* inst, inidividual_t* indv, int col) {
    int covers = 0;
    for (int i = 0; i < inst->nrow[col]; i++) {
        if (!indv->y[inst->row[col][i]]) {
            covers++;
        }
    }
    return (double) inst->cost[col] / (double) covers;
}

void makeFeasible(instance_t* inst, inidividual_t* child) {
    for (int i = 0; i < inst->m; i++) {
        if (!child->y[i]) {
            int col = -1;
            double colCost = (double) INT32_MAX;
            for (int j = 0; j < inst->ncol[i]; j++) {
                int currCol = inst->col[i][j];
                int currColCost = coverCost(inst, child, currCol);
                if (currColCost < colCost) {
                    col = currCol;
                    colCost = currColCost;
                }
            }
            addSet(inst, child, col);
        }
    }
}

/*** Local Search methods ***/
void localSearchGEN(instance_t* inst, inidividual_t* indv) {
    if (fi) {
        firstImprovement(inst, indv);
    } else if (bi) {
        bestImprovement(inst, indv);
    }
}

/*** Check for duplicates ***/
int isDuplicate(instance_t* inst, inidividual_t* child) {
    int result = 0;
    for (int c = 0; c < population_size; c++) {
        inidividual_t* indv = population[c];
        if (indv->fx == child->fx) {
            int duplicate = 1;
            for (int i = 0; i < inst->n; i++) {
                if (indv->x[i] != child->x[i]) {
                    duplicate = 0;
                    break;
                }
            }
            if (duplicate) {
                result = 1;
                break;
            }
        }
    }
    return result;
}

/*** Replace a bad individual with the newly created one ***/
void replaceIndv(instance_t* inst, inidividual_t* child) {
    int totalCost = 0;
    for (int c = 0; c < population_size; c++) {
        totalCost += population[c]->fx;
    }
    double avgCost = (double) totalCost / (double) population_size;
    
    for (int c = 0; c < population_size; c++) {
        inidividual_t* indv = population[c];
        if (indv->fx > avgCost && child->fx < indv->fx) {
            copySolution(inst, child, indv);
            break;
        }
    }
}

/*** Update the current best solution ***/
void updateBestIndv(instance_t* inst, optimal_t* opt) {
    int bestIndv = -1;
    int bestIndvCost = INT32_MAX;
    for (int c = 0; c < population_size; c++) {
        inidividual_t* indv = population[c];
        if (indv->fx < bestIndvCost) {
            bestIndv = c;
            bestIndvCost = indv->fx;
        }
    }
    if (bestIndvCost < opt->fx) {
        updateOptimal(inst, opt, population[bestIndv]);
    }
}

/*** General methods ***/
void GENsolve(instance_t* inst, optimal_t* opt) {
    int iterCount = 0;
    int constructed = 0;
    int* parents = mymalloc(2 * sizeof(int));
    inidividual_t* child = mymalloc(sizeof(inidividual_t));
    allocSolution(inst, child);
    
    while (computeTime(start_time, clock()) < runtime) {
        if (!constructed) {
            for (int c = 0; c < population_size; c++) {
                inidividual_t* indv = population[c];
                while (!isSolution(indv)) {
                    constructIndv(inst, indv);
                }
                eliminate(inst, indv);
                localSearchGEN(inst, indv);
            }
            constructed = 1;
        }
        int acceptChild = 0;
        int trials = 0;
        while (!acceptChild) {
            selectParents(parents);
            int par1 = parents[0];
            int par2 = parents[1];
            
            initSolution(inst, child);
            crossover(inst, par1, par2, child);
            if (!isSolution(child)) makeFeasible(inst, child);
            eliminate(inst, child);
            localSearchGEN(inst, child);
            
            mutate(inst, child);
            if (!isSolution(child)) makeFeasible(inst, child);
            eliminate(inst, child);
            localSearchGEN(inst, child);
            
            trials++;
            if (!isDuplicate(inst, child)) {
                acceptChild = 1;
            }
        }
        
        offspring_count++;
        replaceIndv(inst, child);
        updateBestIndv(inst, opt);
        /*printf("Iteration: %d - trails: %d - time elapsed: %f - optimal cost: %d\n",
               iterCount,
               trials,
               computeTime(start_time, clock()),
               opt->fx);*/
        iterCount++;
        if (qrtd) {
            if (opt->fx <= maxcost) {
                break;
            }
        }
    }
    free(parents);
    freeSolution(inst, child);
    free(child);
}










