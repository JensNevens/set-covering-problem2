//
//  genetic.h
//  HO-Project2
//
//  Created by Jens Nevens on 16/04/16.
//  Copyright © 2016 Jens Nevens. All rights reserved.
//

#ifndef genetic_h
#define genetic_h

#include "data.h"

void GENinitialize(instance_t* inst);
void GENfinalize(instance_t* inst);

// GA Methods
//     Constructive methods
void constructIndv(instance_t* inst, inidividual_t* indv);

//     Parent Selection methods
void selectParents(int* parents);
void tournamentSelection(int* parents);
void proportionateSelection(int* parents);

//     Crossover operators
void crossover(instance_t* inst, int parent1, int parent2, inidividual_t* child);
void uniformCross(instance_t* inst, int parent1, int parent2, inidividual_t* child);
void fusionCross(instance_t* inst, int parent1, int parent2, inidividual_t* child);

//     Mutation operators
void mutate(instance_t* inst, inidividual_t* child);

//     Make Individual valid
double coverCost(instance_t* inst, inidividual_t* indv, int col);
void makeFeasible(instance_t* inst, inidividual_t* indv);

//     Local Search
void localSearchGEN(instance_t* inst, inidividual_t* indv);

//     Check for duplicates
int isDuplicate(instance_t* inst, inidividual_t* child);

//     Replace an Individual
void replaceIndv(instance_t* inst, inidividual_t* newIndv);

//     Update best solution
void updateBestIndv(instance_t* inst, optimal_t* opt);

// General methods
void GENsolve(instance_t* inst, optimal_t* opt);

#endif /* genetic_h */

// 1. Generate initial population:
//    The population is generated by
//    1. choosing a random uncovered row and
//       choosing a column covering that row
//       based on the least cost. (ch2)
//       (Paper used 5 least cost columns)
//    2. applying redundancy elimination

// 2. Representation and fitness:
//    Binary representation is used for representation.
//    Fitness value of an individual is simply the sum
//    of the cost of the selected columns.

// 3.1 Parent selection techniques:
//     Proportionate selection:
//         Calculates probabilities of individual being
//         selected as proportional to their fitness and
//         selects an individual according to this PDF.
//     Tournament selection:
//         Forms 2 pools of T individuals, drawn randomly
//         from the population. Two individuals with the
//         best fitness, each taken from one of the pools,
//         are chosen for mating. The value of T can be
//         determined through experimentation.
//     Here, tournament selection with T=2 is used,
//     but results should be comparable.

// 3.2.1 Crossover operators:
//     1. One-point or two-point crossover operator
//        where k is bounded as follows:
//            min[i, P1[i] != P2[i]] <= k < max[i, P1[i] != P2[i]]
//        (Generates 2 children)
//     2. Uniform crossover operator where each gene
//        in the child is created by copying the corresponding
//        gene from one or the other parent. This is chosen
//        by generating a 0 or 1 randomly.
//        (Generates 1 child)
//     3. Fusion crossover operator:
//        Let f_P1 and f_P2 be the fitness of the parents.
//        For all columns i:
//            If P1[i]==P2[i]:
//                C[i] = P1[i] (= P2[i])
//            Else:
//                C[i] = P1[i] with prob. p = f_P2/(f_P1+f_P2)
//                C[i] = P2[i] with prob. q = 1-p

// 3.2.2 Mutation operators:
//       Mutation works by flipping bits with some small
//       probability. A variable mutation rate is chosen,
//       based on the algorithm's convergence. In the initial
//       stage, the crossover is mostly responsible for the
//       search, whilst converging, the crossover becomes
//       less important and the mutation increases to diversify
//       the search process. When the algorithm converges, the
//       mutation rate will also become stable. The rate at which
//       the algorithm converges depends on the population replacement
//       method.
//       Number of bits mutated =
//           ceil( M_f / (1 + exp(-4*M_g*((t-M_c)/M_f))))
//       where
//           t = #child solutions generated
//           M_f = final stable mutation rate
//           M_c = #child solutions generated at which
//                 a mutation rate of M_f/2 is reached
//           M_g = the gradient at t = M_c
//       (Paper only chooses columns to mutate out of
//        the 5 columns with the least cost in the solution)

// 3.2.3 Feasibility operators:
//       Add columns covering uncovered rows based on
//       the ratio:
//           cost(j) / #uncovered rows that j covers
//       Afterwards, apply redundancy elimination.

// 3.4 Population replacement:
//     1. Steady-state replacement:
//        A child replaces a less fit parent (i.e. a
//        parent with above average fitness value).
//        Care should be taken that no duplicate
//        solutions enter in the population.
//     2. Generational replacement:
//        The entire population is replaced.

