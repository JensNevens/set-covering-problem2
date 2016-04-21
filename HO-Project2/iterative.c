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

/*** Best Improvement local search method ***/
void bestImprovement(instance_t* inst, solution_t* sol) {
    
}