//
//  ant.h
//  HO-Project2
//
//  Created by Jens Nevens on 14/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef ant_h
#define ant_h

#include "data.h"

void addSet(instance_t* inst, solution_t* sol, int col);
void removeSet(instance_t* inst, solution_t* sol, int col);
void shift(solution_t* sol, int row, int start);
int isSolution(solution_t* sol);

void allocSolution(instance_t* inst, solution_t* sol);
void freeSolution(instance_t* inst, solution_t* sol);
void copySolution(instance_t* inst, solution_t* src, solution_t* dest);

#endif /* ant_h */
