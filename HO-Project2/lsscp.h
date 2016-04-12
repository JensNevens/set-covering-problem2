//
//  lsscp.h
//  HO-Project2
//
//  Created by Jens Nevens on 12/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef lsscp_h
#define lsscp_h

// Instance struct
struct Instance {
    int m;        // number of rows
    int n;        // number of columns
    int** row;    // row[i] contains rows that are covered by column i
    int** col;    // col[i] contains columns that cover row i
    int* nrow;    // ncol[i] contains number of columns that cover row i
    int* ncol;    // nrow[i] contains number of rows that are covered by column i
    int* cost;    // cost[i] contains cost of column i
    float* ccost; // ccost[i] contains static cover cost of column i
};

typedef struct Instance instance_t;

void readParameters(int argc, char* argv[]);
void readSCP(char* filename);
void errorExit(char* text);
void finalize();
void* mymalloc(size_t size);
void initialize();


int main(int argc, char* argv[]);


#endif /* lsscp_h */
