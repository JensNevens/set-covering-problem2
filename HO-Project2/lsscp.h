//
//  lsscp.h
//  HO-Project2
//
//  Created by Jens Nevens on 12/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef lsscp_h
#define lsscp_h

#include "data.h"

// SCP methods
void readParameters(int argc, char* argv[]);
void readSCP(char* filename);
void errorExit(char* text);
void diagnostics(ant_t* ant);
void initialize();
void finalize();

// ACO-Methods
//    Heuristic information
double adaptiveCost(ant_t* ant, int col);

//    Constructive methods
unsigned int pickRandom(unsigned int min, unsigned int max);
int randomFromPDF(double* probabilities, int len);
void constructSolution(ant_t* ant);

//    Local Search methods
void localSearch(ant_t* ant);

//    Redudancy elimination
int cmp(const void* a, const void* b);
void eliminate(ant_t* ant);

//    Update best solution
void updateOptimal(ant_t* ant);
void updateTau(optimal_t* opt);
void updateBest();

//    Update pheromone trails
void updatePheromone();

// General methods
void solve();
int main(int argc, char* argv[]);


#endif /* lsscp_h */
