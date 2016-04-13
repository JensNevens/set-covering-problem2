//
//  data.h
//  HO-Project2
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef data_h
#define data_h

// Instance struct
struct Instance {
    int m;        // number of rows
    int n;        // number of columns
    int** row;    // row[i] contains rows that are covered by column i
    int** col;    // col[i] contains columns that cover row i
    int* nrow;    // nrow[i] contains number of rows that are covered by column i
    int* ncol;    // ncol[i] contains number of columns that cover row i
    int* cost;    // cost[i] contains cost of column i
};

typedef struct Instance instance_t;

// Solution struct
struct Ant {
    int* x;          // x[i] 0,1 if column i is selected
    int* y;          // y[i] 0,1 if row i covered by the actual solution
    int fx;          // sum of the cost of the columns selected in the solution
    int** col_cover; // col_colver[i] contains selected columns that cover row i
    int* ncol_cover; // ncol_cover[i] contains number of selected columns that cover row i
    int un_rows;     // the amount of un-covered rows
    float* pheromone;  // pheromone[i] contains the amount of pheromone for column i
    float* heuristic;  // heuristic[i] contains the heuristic value for column i
};

typedef struct Ant ant_t;

extern int ant_count;
extern float beta;
extern float ro;
extern float epsilon;
extern float tau_min;
extern float tau_max;


#endif /* data_h */
