#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <omp.h>

#include "load.h"

#define NS_IN_SEC 1000000000

static inline long timediff_ns(struct timespec new, struct timespec old)
{
  return (new.tv_sec - old.tv_sec) * NS_IN_SEC + (new.tv_nsec - old.tv_nsec);
}

static volatile int finish = 0;

void end(int __attribute__((unused)) signal)
{
  finish = 1;
}

int main(int argc, char **argv)
{
  int result;
  if ((result = load_test_init(argc, argv)))
    return result;
  
  struct timespec last_time, first_time, new_time;
  
  clock_gettime(CLOCK_MONOTONIC, &first_time);
  last_time = first_time;
  
  int count = 0;
  int last_count = 0;
  
  int err = 0;
  signal(SIGINT, end);

  int nthreads = 0;
  char *env_thr = getenv("NUM_THREADS");
  if (env_thr)
    nthreads = atoi(env_thr);
  if (!nthreads)
    nthreads = 16;
  
  omp_set_num_threads(nthreads);
  
  #pragma omp parallel private(result)
  {
    result = load_test_prepare();
    
    if (result)
      err = finish = result;
    else while (!finish) {
      if ((result = load_test_run())) {
        finish = 1;
        err = result;
        break;
      }
      
      #pragma omp critical
      { count++; }
      
      #pragma omp single
      {    
        clock_gettime(CLOCK_MONOTONIC, &new_time);
        
        if (last_time.tv_sec != new_time.tv_sec) {
          unsigned long ns = timediff_ns(new_time, last_time);
          printf("%.02f requests per second\n", 1.0 * (count - last_count) * NS_IN_SEC / ns);
          last_count = count;
          last_time = new_time;
        }
      }
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &new_time);
  
  const float seconds = 1.0 * timediff_ns(new_time, first_time) / NS_IN_SEC;
  printf("\n\n%d requests total in %.02f seconds (average %.02f requests per second).\n", count, seconds, 1.0 * count / seconds);
  return err;
}
