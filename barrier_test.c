//
// Created by Chris Kjellqvist (cmk91@duke.edu) on 8/19/21.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#define GRID_SIZE_ROWS 256
#define GRID_SIZE_COLS 512
#define NUM_ITERATIONS 1000
#define NUM_THREADS 16
#define FREEZING 273.0 // kelvin
#define BOILING 373.0 // kelvin

#include "barrier.h"

static double ** plate_temperature;
static double ** plate_temperature_delta;

double get_temperature(int r, int c) {
  if (r < 0) return FREEZING;
  if (c < 0) return FREEZING;

  // outside of our plate we have heating elements that gradually get closer and closer to BOILING further to the top
  // right we get
  if (c >= GRID_SIZE_COLS) {
    return FREEZING + (BOILING - FREEZING) * ((double) r / (GRID_SIZE_ROWS - 1));
  }
  if (r >= GRID_SIZE_ROWS) {
    return FREEZING + (BOILING - FREEZING) * ((double) c / (GRID_SIZE_COLS - 1));
  }

  return plate_temperature[r][c];
}

struct work_range {
  int begin, end;
};

barrier_t barr1, barr2;
void* do_laplace_solve(void * args) {
  struct work_range *wk = args;
  int begin = wk->begin, end = wk->end;
  for (int iteration = 0; iteration < NUM_ITERATIONS; ++iteration) {
    barrier_wait(&barr1);
    // find differences
    for (int i = begin; i < end; ++i) {
      for (int j = 0; j < GRID_SIZE_COLS; ++j) {
        plate_temperature_delta[i][j] = (get_temperature(i-1,j) + get_temperature(i+1, j) + get_temperature(i, j-1) +
                                         get_temperature(i, j+1)) / 4.0 - get_temperature(i, j);
      }
    }
    barrier_wait(&barr2);
    // change the state of the plate
    for (int i = begin; i < end; ++i) {
      for (int j = 0; j < GRID_SIZE_COLS; ++j) {
        plate_temperature[i][j] += plate_temperature_delta[i][j];
      }
    }
  }
  return NULL;
}

void launch_laplace_test(FILE *os){
  barrier_init(&barr1, NUM_THREADS);
  barrier_init(&barr2, NUM_THREADS);

  plate_temperature = malloc(sizeof(double*) * GRID_SIZE_ROWS);
  plate_temperature_delta = malloc(sizeof(double*) * GRID_SIZE_ROWS);

  for (int i = 0; i < GRID_SIZE_ROWS; ++i) {
    plate_temperature[i] = malloc(sizeof(double) * GRID_SIZE_COLS);
    plate_temperature_delta[i] = malloc(sizeof(double) * GRID_SIZE_COLS);
    // initially we have a freezing cold plate
    for (int j = 0; j < GRID_SIZE_COLS; ++j) plate_temperature_delta[i][j] = plate_temperature[i][j] = 273.0;
  }

  // split the work as evenly as we can between threads
  double row_accum = 0;
  double row_diff = (double)GRID_SIZE_ROWS / NUM_THREADS;
  struct work_range thread_params[NUM_THREADS];
  for (int thr = 0; thr < NUM_THREADS - 1; ++thr) {
    double nxt = row_accum + row_diff;
    thread_params[thr].begin = (int) row_accum;
    thread_params[thr].end = (int) nxt;
    row_accum = nxt;
  }
  thread_params[NUM_THREADS-1].begin = (int)row_accum;
  thread_params[NUM_THREADS-1].end = GRID_SIZE_ROWS;

  // launch threads
  pthread_t threads[NUM_THREADS];
  for (int thr = 0; thr < NUM_THREADS; ++thr) {
    pthread_create(&threads[thr], NULL, do_laplace_solve, (void*)&thread_params[thr]);
  }

  // join threads
  for (int thr = 0; thr < NUM_THREADS; ++thr) {
    pthread_join(threads[thr], NULL);
  }
  for (int i = 0; i < GRID_SIZE_ROWS; ++i) {
    for (int j = 0; j < GRID_SIZE_COLS; ++j) {
      fprintf(os, "%f ", plate_temperature[i][j]);
    }
    fprintf(os, "\n");
  }
}

#define SBT_THREADS 10
// mask with SBT_THREADS number of 1s
#define SBT_MASK 0x3FF
int sbt_buffer[SBT_THREADS*2];
int sbt_buffer_idx;
pthread_mutex_t sbt_lk = PTHREAD_MUTEX_INITIALIZER;

// This test makes sure the barrier can work one time
void * do_single_barrier_test (void * t_id) {
  int thread_number = *(int*)t_id;

  pthread_mutex_lock(&sbt_lk);
  sbt_buffer[sbt_buffer_idx++] = thread_number;
  pthread_mutex_unlock(&sbt_lk);

  barrier_wait(&barr1);

  pthread_mutex_lock(&sbt_lk);
  sbt_buffer[sbt_buffer_idx++] = thread_number;
  pthread_mutex_unlock(&sbt_lk);

  return NULL;
}

void launch_single_barrier_test(FILE *os) {
  int t_ids[SBT_THREADS];
  for (int i = 0; i < SBT_THREADS; ++i)
    t_ids[i] = i;

  for (int tries = 0; tries < 50; ++tries) {

    barrier_init(&barr1, SBT_THREADS);
    pthread_t threads[SBT_THREADS];
    sbt_buffer_idx = 0;
    for(int i = 0; i < SBT_THREADS; ++i)
      pthread_create(&threads[i], NULL, do_single_barrier_test, &t_ids[i]);
    for(int i = 0; i < SBT_THREADS; ++i)
      pthread_join(threads[i], NULL);

    for(int i = 0; i < SBT_THREADS * 2; ++i) {
      fprintf(os, "%d ", sbt_buffer[i]);
    }
  }
}

void launch_barrier_reuse_test(FILE *os) {
  int t_ids[SBT_THREADS];
  for (int i = 0; i < SBT_THREADS; ++i)
    t_ids[i] = i;

  barrier_init(&barr1, SBT_THREADS);
  for (int tries = 0; tries < 50; ++tries) {
    pthread_t threads[SBT_THREADS];
    sbt_buffer_idx = 0;
    for(int i = 0; i < SBT_THREADS; ++i)
      pthread_create(&threads[i], NULL, do_single_barrier_test, &t_ids[i]);
    for(int i = 0; i < SBT_THREADS; ++i)
      pthread_join(threads[i], NULL);
    for(int i = 0; i < SBT_THREADS * 2; ++i)
      fprintf(os, "%d ", sbt_buffer[i]);
  }
  barrier_free(&barr1);
}


#define SBT 1
#define BRT 2
#define LAP 4
#define ALL 7
char * test_strings[] = {"-sbt", "-brt", "-app"};


int main(int argc, char** argv) {
  int exe_flags = 0;
  for (int q = 1; q < argc; ++q) {
    for (int tst = 0; tst < 3; ++tst) {
      if (strcmp(argv[q], test_strings[tst])==0){
        exe_flags |= 1 << tst;
      } else if (strcmp(argv[q], "-all")==0) exe_flags = ALL;
    }
  }

  if (exe_flags == 0) {
    printf("To run programs add command lines options to enable running programs:\n"
        "-sbt\tRun the Single Use Barrier Test - this just runs 10 threads throught barrier once and makes sure they stop in and wait for each other in the expected way.\n"
        "-brt\tRun the Barrier Re-use Test - Do the same as the test above but between trial runs, use the same barrier. This should help verify that threads are resetting the barrier in the expected way.\n"
        "-app\tRun the Heat Plate Application Test - This is the test for testing full functionality of the barrier.\n"
        "-all\tRun all of the tests.\n");
  }

  FILE *os;
  if (exe_flags & SBT) {
    os = fopen("sbt_out.txt", "w");
    launch_single_barrier_test(os);
    fclose(os);
  }

  if (exe_flags & BRT) {
    os = fopen("brt_out.txt", "w");
    launch_barrier_reuse_test(os);
    fclose(os);
  }

  if (exe_flags & LAP) {
    os = fopen("laplace_out.txt", "w");
    launch_laplace_test(os);
    fclose(os);
  }
}
