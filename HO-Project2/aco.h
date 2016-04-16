//
//  aco.h
//  HO-Project2
//
//  Created by Jens Nevens on 16/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef aco_h
#define aco_h

#include "data.h"

void ACOinitialize(instance_t* inst);
void ACOfinalize(instance_t* inst);

// ACO-Methods
//     Heuristic information
double adaptiveCost(instance_t* inst, ant_t* ant, int col);

//     Constructive methods
void constructSolution(instance_t* inst, ant_t* ant);

//     Local Search
void localSearch(instance_t* inst, ant_t* ant);
//         Method described by Ren et al.
int computeWj(instance_t* inst, ant_t* ant, int* Wj, int col);
int getLow(instance_t* inst, int row);
void replaceColumns(instance_t* inst, ant_t* ant);
//         First Improvement method
void firstImprovement(instance_t* inst, ant_t* ant);

//     Update best solution
void updateOptimal(instance_t* inst, optimal_t* opt, ant_t* ant);
void updateTau(optimal_t* opt);
void updateBest(instance_t* inst, optimal_t* opt);

//     Update pheromone trails
void updatePheromone(instance_t* inst, optimal_t* opt);
void clearColony(instance_t* inst);

// General method
void ACOsolve(instance_t* inst, optimal_t* opt);
void ACOtest(instance_t* inst, optimal_t* opt);

#endif /* aco_h */
