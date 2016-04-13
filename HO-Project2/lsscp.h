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

void readParameters(int argc, char* argv[]);
void readSCP(char* filename);
void errorExit(char* text);
void initialize();
void finalize();

// ACO-Methods
unsigned int pickRandom(unsigned int min, unsigned int max);
int randomFromPDF(float* probabilities, int len);
void addSet(ant_t* ant, int col);
void constructSolution(ant_t* ant);

void localSearch(ant_t* ant);

void updatePheromone();

float adaptiveCost(ant_t* ant, int col);
void updateHeuristic(ant_t* ant);

int isSolution(ant_t* ant);
void solve();
int main(int argc, char* argv[]);


#endif /* lsscp_h */
