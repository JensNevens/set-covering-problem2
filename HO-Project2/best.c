//
//  best.c
//  HO-Project2
//
//  Created by Jens Nevens on 22/04/16.
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
#include "best.h"

void allocBest(instance_t* inst, best_t* best) {
    best->added = (int*) mymalloc(inst->n * sizeof(int));
}

void initBest(solution_t* sol, best_t* best) {
    best->removed = -1;
    best->addedPtr = 0;
    best->fx = sol->fx;
}

void applyBest(instance_t* inst, solution_t* sol, best_t* best) {
    for (int i = 0; i < best->addedPtr; i++) {
        addSet(inst, sol, best->added[i]);
    }
    removeSet(inst, sol, best->removed);
}

void freeBest(best_t* best) {
    free(best->added);
}
