//
//  ant.c
//  HO-Project2
//
//  Created by Jens Nevens on 14/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "utils.h"
#include "solution.h"

/*** Add set 'col' to the partial solution ***/
void addSet(instance_t* inst, solution_t* sol, int col) {
    sol->x[col] = 1;
    sol->fx += inst->cost[col];
    for (int i = 0; i < inst->nrow[col]; i++) {
        int row = inst->row[col][i];
        sol->ncol_cover[row] += 1;
        if (!sol->y[row]) {
            sol->y[row] = 1;
            sol->un_rows -= 1;
        }
        for (int j = 0; j < inst->ncol[row]; j++) {
            if (sol->col_cover[row][j] < 0) {
                sol->col_cover[row][j] = col;
                break;
            }
        }
    }
}

/*** Shift columns covering element 'row' to the left ***/
void shift(solution_t* sol, int row, int start) {
    for (int i = start; i < sol->ncol_cover[row]; i++) {
        if (i+1 < sol->ncol_cover[row]) {
            if (sol->col_cover[row][i+1] >= 0) {
                sol->col_cover[row][i] = sol->col_cover[row][i+1];
            } else {
                sol->col_cover[row][i] = -1;
                break;
            }
        } else {
            sol->col_cover[row][i] = -1;
        }
    }
}

/*** Remove set 'col' from the partial solution ***/
void removeSet(instance_t* inst, solution_t* sol, int col) {
    sol->x[col] = 0;
    sol->fx -= inst->cost[col];
    for (int i = 0; i < inst->nrow[col]; i++) {
        int row = inst->row[col][i];
        for (int j = 0; j < sol->ncol_cover[row]; j++) {
            if (sol->col_cover[row][j] == col) {
                sol->col_cover[row][j] = -1;
                shift(sol, row, j);
                break;
            }
        }
        sol->ncol_cover[row] -= 1;
        if (sol->ncol_cover[row] <= 0) {
            sol->un_rows += 1;
            sol->y[row] = 0;
        }
    }
}

/*** Check if current solution is a valid solution ***/
int isSolution(solution_t* sol) {
    return (sol->un_rows <= 0);
}

/*** Allocate memory for a single solution ***/
void allocSolution(instance_t* inst, solution_t* sol) {
    sol->x = mymalloc(inst->n * sizeof(int));
    sol->y = mymalloc(inst->m * sizeof(int));
    sol->col_cover = mymalloc(inst->m * sizeof(int*));
    sol->ncol_cover = mymalloc(inst->m * sizeof(int));
    for (int i = 0; i < inst->m; i++) {
        sol->col_cover[i] = mymalloc(inst->ncol[i] * sizeof(int));
    }
}

/*** Free all memory for a single solution ***/
void freeSolution(instance_t* inst, solution_t* sol) {
    for (int i = 0; i < inst->m; i++) {
        free(sol->col_cover[i]);
    }
    free(sol->col_cover);
    free(sol->ncol_cover);
    free(sol->y);
    free(sol->x);
}

/*** Deep copy one solution to another ***/
void copySolution(instance_t* inst, solution_t* src, solution_t* dest) {
    dest->fx = src->fx;
    dest->un_rows = src->un_rows;
    for (int i = 0; i < inst->n; i++) {
        dest->x[i] = src->x[i];
    }
    for (int i = 0; i < inst->m; i++) {
        dest->y[i] = src->y[i];
        dest->ncol_cover[i] = src->ncol_cover[i];
        for (int j = 0; j < inst->ncol[i]; j++) {
            dest->col_cover[i][j] = src->col_cover[i][j];
        }
    }
}

/*** Update the optimal solution ***/
void updateOptimal(instance_t* inst, optimal_t* opt, solution_t* sol) {
    opt->time = computeTime(start_time, clock());
    opt->fx = sol->fx;
    for (int i = 0; i < inst->n; i++) opt->x[i] = sol->x[i];
}

