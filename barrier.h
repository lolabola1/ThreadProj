//
// Created by Chris Kjellqvist (cmk91@duke.edu) on 8/19/21.
//

#ifndef THREADPROJ_BARRIER_H
#define THREADPROJ_BARRIER_H

#include <pthread.h>
#include <stdlib.h>

typedef struct {
// TO IMPLEMENT!
} barrier_t;

// Implement these in barrier.c
void barrier_init(barrier_t *barr, int num_threads);
void barrier_wait(barrier_t *barr);
void barrier_free(barrier_t *barr);

#endif //THREADPROJ_BARRIER_H
