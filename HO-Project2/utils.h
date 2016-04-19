//
//  utils.h
//  HO-Project2
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef utils_h
#define utils_h

#include "data.h"

void* mymalloc(size_t size);
double computeTime(clock_t start, clock_t end);
int totalCost(instance_t* inst);
int columnCovers(instance_t* inst, int col, int row);
unsigned int pickRandom(unsigned int min, unsigned int max);
int randomFromPDF(double* probabilities, int len);
void eliminate(instance_t* inst, inidividual_t* indv);

#endif /* utils_h */
