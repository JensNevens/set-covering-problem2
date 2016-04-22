//
//  iterative.c
//  HO-Project2
//
//  Created by Jens Nevens on 21/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "solution.h"
#include "utils.h"
#include "aco.h"
#include "genetic.h"
#include "best.h"
#include "iterative.h"

/*** First Improvement local search method ***/
void firstImprovement(instance_t* inst, solution_t* sol) {
    int improvement = 1;
    solution_t* solcpy = mymalloc(sizeof(solution_t));
    allocSolution(inst, solcpy);
    copySolution(inst, sol, solcpy);
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (solcpy->x[i]) {
                removeSet(inst, solcpy, i);
                while (!isSolution(solcpy)) {
                    if (aco) {
                        constructAnt(inst, solcpy);
                    } else if (ga) {
                        constructIndv(inst, solcpy);
                    }
                }
                if (solcpy->fx < sol->fx) {
                    copySolution(inst, solcpy, sol);
                    improvement = 1;
                    eliminate(inst, sol);
                } else {
                    copySolution(inst, sol, solcpy);
                }
            }
        }
    }
    freeSolution(inst, solcpy);
    free(solcpy);
}

void findAddedSets(instance_t* inst, solution_t* sol, solution_t* solcpy, best_t* best, int removed) {
    best->addedPtr = 0;
    for (int i = 0; i < inst->n; i++) {
        if (sol->x[i] != solcpy->x[i] && i != removed) {
            best->added[best->addedPtr] = i;
            best->addedPtr++;
        }
    }
}

/*** Best Improvement local search method ***/
void bestImprovement(instance_t* inst, solution_t* sol) {
    int improvement = 1;
    
    solution_t* solcpy = (solution_t*) mymalloc(sizeof(solution_t));
    best_t* best = (best_t*) mymalloc(sizeof(best_t));
    allocSolution(inst, solcpy);
    allocBest(inst, best);
    initSolution(inst, solcpy);
    initBest(solcpy, best);
    
    while (improvement) {
        improvement = 0;
        for (int i = 0; i < inst->n; i++) {
            if (solcpy->x[i]) {
                removeSet(inst, solcpy, i);
                while (!isSolution(solcpy)) {
                    if (aco) {
                        constructAnt(inst, solcpy);
                    } else if (ga) {
                        constructIndv(inst, solcpy);
                    }
                }
                if (solcpy->fx < best->fx) {
                    improvement = 1;
                    best->fx = solcpy->fx;
                    best->removed = i;
                    findAddedSets(inst, sol, solcpy, best, i);
                }
                copySolution(inst, sol, solcpy);
            }
        }
        if (improvement) {
            applyBest(inst, sol, best);
            eliminate(inst, sol);
            copySolution(inst, sol, solcpy);
            best->removed = -1;
            best->addedPtr = 0;
        }
    }
    freeSolution(inst, solcpy);
    freeBest(best);
    free(solcpy);
    free(best);
}