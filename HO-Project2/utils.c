//
//  utils.c
//  HO-Project2
//
//  Created by Jens Nevens on 13/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "utils.h"

/*** Allocate memory ***/
void* mymalloc(size_t size) {
    void *s;
    if ((s=malloc(size)) == NULL) {
        fprintf(stderr, "malloc : Not enough memory.\n");
        exit(EXIT_FAILURE);
    }
    return s;
}

/*** Computes runtime of algorithm ***/
double computeTime(clock_t start, clock_t end) {
    return 1000.0 * (end - start)/CLOCKS_PER_SEC;
}
