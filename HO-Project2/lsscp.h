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
void initialize();
void finalize();

// Sorting functions
int sortDesc(const void* a, const void* b);
int sortAsc(const void* a, const void* b);

// General methods
int main(int argc, char* argv[]);


#endif /* lsscp_h */
