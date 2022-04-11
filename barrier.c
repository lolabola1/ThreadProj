//
// Created by Chris Kjellqvist (cmk91@duke.edu) on 8/19/21.
//

#include "barrier.h"

// initialize your barrier by setting the initial values for all the fields of
// the given barrier_t struct.
// remember that you get to define the barrier_t struct's fields in barrier.h!
void barrier_init(barrier_t *barr, int num_threads) {
}

// free anything that is dynamically allocated inside the barrier, do not
// free the barrier itself - it can be statically allocated or located on the
// stack, in which case freeing the barrier struct will result in a segfault.
void barrier_free(barrier_t *barr) {
}

// wait on the barrier. The barrier should be initialized to expect a certain
// number of threads. If there are (n-1) threads waiting and another thread
// calls barrier_wait(), that thread is responsible for releasing the threads
// to keep working and is also responsible for resetting the barrier.
void barrier_wait(barrier_t *barr) {
}
