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
    int m;      // number of rows
    int n;      // number of columns
    int** row;  // row[i] contains rows that are covered by column i
    int** col;  // col[i] contains columns that cover row i
    int* nrow;  // nrow[i] contains number of rows that are covered by column i
    int* ncol;  // ncol[i] contains number of columns that cover row i
    int* cost;  // cost[i] contains cost of column i
};

typedef struct Instance instance_t;

// Solution struct
struct Solution {
    int* x;             // x[i] 0,1 if column i is selected
    int* y;             // y[i] 0,1 if row i covered by the actual solution
    int fx;             // sum of the cost of the columns selected in the solution
    int** col_cover;    // col_colver[i] contains selected columns that cover row i
    int* ncol_cover;    // ncol_cover[i] contains number of selected columns that cover row i
    int un_rows;        // the amount of un-covered rows
};

typedef struct Solution solution_t;
typedef struct Solution ant_t;
typedef struct Solution inidividual_t;

// Optimal solution struct
struct Optimal {
    int* x;       // x[i] 0,1 if column i is selected
    int fx;       // sum of cost of columns selected in the solution
    double time;  // Number of seconds after which this solution has been found
};

typedef struct Optimal optimal_t;

// Best struct
struct Best {
    int removed;
    int* added;
    int addedPtr;
    int fx;
};

typedef struct Best best_t;

extern clock_t start_time;
extern float runtime;
extern int maxcost;

extern int ant_count;
extern double beta;
extern double ro;
extern double epsilon;
extern double tau_min;
extern double tau_max;
extern double* pheromone;

extern int offspring_size;
extern int population_size;
extern double Mf;
extern double Mc;
extern double Mg;
extern int offspring_count;
extern int pool_size;

extern int aco;
extern int ga;
extern int fi;
extern int rep;
extern int tour;
extern int prop;
extern int uniform;
extern int fusion;
extern int bi;
extern int qrtd;

#endif /* data_h */
