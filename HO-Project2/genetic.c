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
int Mf = 10;
int Mc = 200;
int Mg = 2;
int offspring_count;

inidividual_t** population;

/********************/
/** init and final **/
/********************/
void GENinitialize(instance_t* inst) {
    offspring_count = 0;
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
void constructSolution(instance_t* inst, inidividual_t* indv) {}

/*** Select suitable parents from the population ***/

/*** General methods ***/
void GENsolve(instance_t* inst, optimal_t* opt) {
    int iterCount = 0;
    int constructed = 0;
    while (computeTime(start_time, clock()) < 10) {
        if (!constructed) {
            for (int c = 0; c < population_size; c++) {
                inidividual_t* indv = population[c];
                constructSolution(inst, indv);
            }
            constructed = 1;
        }
        int* parents = mymalloc(2 * sizeof(int));
        selectParents(inst, parents);
        int par1 = parents[0];
        int par2 = parents[1];
        inidividual_t* child = crossover(inst, par1, par2);
        mutate(inst, child);
        feasible(inst, child);
        replaceIndvs(inst, child);
        iterCount++;
    }
}










