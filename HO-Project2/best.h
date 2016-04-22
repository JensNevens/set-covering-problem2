//
//  best.h
//  HO-Project2
//
//  Created by Jens Nevens on 22/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef best_h
#define best_h

#include "data.h"

void allocBest(instance_t* inst, best_t* best);
void initBest(solution_t* sol, best_t* best);
void applyBest(instance_t* inst, solution_t* sol, best_t* best);
void freeBest(best_t* best);

#endif /* best_h */
