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

void addSet(instance_t* inst, ant_t* ant, int col);
void removeSet(instance_t* inst, ant_t* ant, int col);
void shift(ant_t* ant, int row, int start);
int isSolution(ant_t* ant);

void allocAnt(instance_t* inst, ant_t* ant);
void freeAnt(instance_t* inst, ant_t* ant);
void copyAnt(instance_t* inst, ant_t* src, ant_t* dest);

#endif /* ant_h */
