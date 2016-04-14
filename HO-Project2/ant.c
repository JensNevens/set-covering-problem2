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

#include "ant.h"

/*** Add set 'col' to the partial solution of the ant ***/
void addSet(instance_t* inst, ant_t* ant, int col) {
    ant->x[col] = 1;
    ant->fx += inst->cost[col];
    for (int i = 0; i < inst->nrow[col]; i++) {
        int row = inst->row[col][i];
        ant->ncol_cover[row] += 1;
        if (!ant->y[row]) {
            ant->y[row] = 1;
            ant->un_rows -= 1;
        }
        for (int j = 0; j < inst->ncol[row]; j++) {
            if (ant->col_cover[row][j] < 0) {
                ant->col_cover[row][j] = col;
                break;
            }
        }
    }
}

/*** Shift columns covering element 'row' to the left ***/
void shift(ant_t* ant, int row, int start) {
    for (int i = start; i < ant->ncol_cover[row]; i++) {
        if (i+1 < ant->ncol_cover[row]) {
            if (ant->col_cover[row][i+1] >= 0) {
                ant->col_cover[row][i] = ant->col_cover[row][i+1];
            } else {
                ant->col_cover[row][i] = -1;
                break;
            }
        } else {
            ant->col_cover[row][i] = -1;
        }
    }
}

/*** Remove set 'col' from the partial solution of the ant ***/
void removeSet(instance_t* inst, ant_t* ant, int col) {
    ant->x[col] = 0;
    ant->fx -= inst->cost[col];
    for (int i = 0; i < inst->nrow[col]; i++) {
        int row = inst->row[col][i];
        for (int j = 0; j < ant->ncol_cover[row]; j++) {
            if (ant->col_cover[row][j] == col) {
                ant->col_cover[row][j] = -1;
                shift(ant, row, j);
                break;
            }
        }
        ant->ncol_cover[row] -= 1;
        if (ant->ncol_cover[row] <= 0) {
            ant->un_rows += 1;
            ant->y[row] = 0;
        }
    }
}

/*** Check if current solution of the ant is a valid solution ***/
int isSolution(ant_t* ant) {
    return (ant->un_rows <= 0);
}


