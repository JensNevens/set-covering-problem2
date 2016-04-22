//
//  iterative.h
//  HO-Project2
//
//  Created by Jens Nevens on 21/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef iterative_h
#define iterative_h

#include "data.h"

void firstImprovement(instance_t* inst, solution_t* sol);

void findAddedSets(instance_t* inst, solution_t* sol, solution_t* solcpy, best_t* best, int removed);
void bestImprovement(instance_t* inst, solution_t* sol);

#endif /* iterative_h */
