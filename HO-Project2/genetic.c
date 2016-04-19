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

// !!! Algorithm in papers assumed columns are ordered

int offspring_size = 1000; // This can also be a time-limit!
int population_size = 100;
double Mf = 10;
double Mc = 200;
double Mg = 2;
int offspring_count = 0;
int pool_size = 2;

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
        indv->fx = 0;
        indv->un_rows = inst->m;
        for (int i = 0; i < inst->n; i++) indv->x[i] = 0;
        for (int i = 0; i < inst->m; i++) {
            indv->y[i] = 0;
            indv->ncol_cover[i] = 0;
            int k = inst->ncol[i];
            for (int j = 0; j < k; j++) indv->col_cover[i][j] = -1;
        }
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
    int colCost = INT32_MAX;
    for (int i = 0; i < inst->ncol[row]; i++) {
        int currCol = inst->col[row][i];
        if (!indv->x[currCol]) {
            if (inst->cost[currCol] < colCost) {
                col = currCol;
                colCost = inst->cost[currCol];
            }
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
    
    double probSum = 0;
    for (int c = 0; c < population_size; c++) {
        probSum += probabilities[c];
    }
    
    for (int i = 0; i < 2; i++) {
        parents[i] = randomFromPDF(probabilities, population_size);
    }
    free(probabilities);
}

/*** Crossover operators ***/
// TODO: Also extend to 1-point or 2-point crossover?
//       Problem is, these methods generate 2 children
//       Solution: singleCrossover (uni, fusion) and doubleCrossover (1-p, 2-p)?
void crossover(instance_t* inst, int parent1, int parent2, int* child) {
    if (uniform) {
        uniformCross(inst, parent1, parent2, child);
    } else if (fusion) {
        fusionCross(inst, parent1, parent2, child);
    }
}

/*** Uniform crossover ***/
void uniformCross(instance_t* inst, int parent1, int parent2, int* child) {
    inidividual_t* par1 = population[parent1];
    inidividual_t* par2 = population[parent2];
    
    for (int i = 0; i < inst->n; i++) {
        int idx = pickRandom(0, 1);
        if (!idx) {
            child[i] = par1->x[i];
        } else {
            child[i] = par2->x[i];
        }
    }
}

/*** Fusion crossover ***/
void fusionCross(instance_t* inst, int parent1, int parent2, int* child) {
    inidividual_t* par1 = population[parent1];
    inidividual_t* par2 = population[parent2];
    double* probabilities = mymalloc(2 * sizeof(double));
    
    for (int i = 0; i < inst->n; i++) {
        if (par1->x[i] == par2->x[i]) {
            child[i] = par1->x[i];
        } else {
            probabilities[0] = (double) par2->fx / (double) (par1->fx + par2->fx);
            probabilities[1] = (double) 1 - probabilities[0];
            int idx = randomFromPDF(probabilities, 2);
            if (!idx) {
                child[i] = par1->x[i];
            } else {
                child[i] = par2->x[i];
            }
        }
    }
    free(probabilities);
}

/*** Mutation operator ***/
void mutate(instance_t* inst, int* child) {
    int mutateCount = (int) ceil((double) Mf / (double) (1 + exp(-4 * Mg * ((offspring_count - Mc)/Mf))));
    printf("Mutating %d columns\n", mutateCount);
    for (int i = 0; i < mutateCount; i++) {
        int col = pickRandom(0, inst->n-1);
        if (child[col]) {
            child[col] = 0;
        } else {
            child[col] = 1;
        }
    }
}

/*** Create an Individual from the generated child ***/
void createIndv(instance_t* inst, int* child, inidividual_t* indv) {
    indv->fx = 0;
    indv->un_rows = inst->m;
    for (int i = 0; i < inst->n; i++) indv->x[i] = 0;
    for (int i = 0; i < inst->m; i++) {
        indv->y[i] = 0;
        indv->ncol_cover[i] = 0;
        int k = inst->ncol[i];
        for (int j = 0; j < k; j++) indv->col_cover[i][j] = -1;
    }
    
    for (int i = 0; i < inst->n; i++) {
        if (child[i]) {
            addSet(inst, indv, i);
        }
    }
}

/*** Make the Individual a valid solution ***/
double coverCost(instance_t* inst, inidividual_t* indv, int col) {
    int covers = 0;
    for (int i = 0; i < inst->nrow[col]; i++) {
        if (!indv->y[inst->row[col][i]]) {
            covers += 1;
        }
    }
    return (double) inst->cost[col] / (double) covers;
}

void makeFeasible(instance_t* inst, inidividual_t* indv) {
    for (int i = 0; i < inst->m; i++) {
        if (!indv->y[i]) {
            int col = -1;
            double colCost = (double) INT32_MAX;
            for (int j = 0; j < inst->ncol[i]; j++) {
                int currCol = inst->col[i][j];
                int currColCost = coverCost(inst, indv, currCol);
                if (currColCost < colCost) {
                    col = currCol;
                    colCost = currColCost;
                }
            }
            addSet(inst, indv, col);
        }
    }
}

/*** Replace a bad individual with the newly created one ***/
void replaceIndv(instance_t* inst, inidividual_t* newIndv) {
    int totalCost = 0;
    for (int c = 0; c < population_size; c++) {
        totalCost += population[c]->fx;
    }
    double avgCost = (double) totalCost / (double) population_size;
    printf("Population avg is %f\n", avgCost);
    
    for (int c = 0; c < population_size; c++) {
        inidividual_t* oldIndv = population[c];
        if (oldIndv->fx > avgCost && newIndv->fx < oldIndv->fx) {
            printf("Indv %d (cost %d) is replaced with child (cost %d)\n",
                   c, oldIndv->fx, newIndv->fx);
            copySolution(inst, newIndv, oldIndv);
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
    // parents contains 2 idx's of individuals in the population
    // child is binary representation of an individual
    int* parents = mymalloc(2 * sizeof(int));
    int* child = mymalloc(inst->n * sizeof(int));
    inidividual_t* indv = mymalloc(sizeof(inidividual_t));
    allocSolution(inst, indv);
    
    //while (computeTime(start_time, clock()) < 10) {
    while (offspring_count < 10) {
        if (!constructed) {
            for (int c = 0; c < population_size; c++) {
                inidividual_t* indv = population[c];
                while (!isSolution(indv)) {
                    constructIndv(inst, indv);
                }
                eliminate(inst, indv);
            }
            constructed = 1;
        }
        // Main loop of the algorithm:
        selectParents(parents);
        int par1 = parents[0];
        int par2 = parents[1];
        for (int i = 0; i < inst->n; i++) child[i] = 0;
        printf("Parents %d and %d selected\n", par1, par2);
        crossover(inst, par1, par2, child);
        mutate(inst, child);
        createIndv(inst, child, indv);
        printf("Initial child cost is %d\n", indv->fx);
        if (!isSolution(indv)) {
            makeFeasible(inst, indv);
        }
        printf("Completed child cost is %d\n", indv->fx);
        eliminate(inst, indv);
        printf("Child cost after RE is %d\n", indv->fx);
        // TODO: Check for duplicates!!
        offspring_count++;
        replaceIndv(inst, indv);
        updateBestIndv(inst, opt);
        printf("Iteration: %d - time elapsed: %f - optimal cost: %d\n\n\n",
               iterCount,
               computeTime(start_time, clock()),
               opt->fx);
        iterCount++;
    }
    // Free resources
    free(parents);
    free(child);
    freeSolution(inst, indv);
    free(indv);
}










