#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "conjur/authn.h"

#define NS_IN_SEC 1000000000

static inline long timediff_ns(struct timespec new, struct timespec old)
{
  return (new.tv_sec - old.tv_sec) * NS_IN_SEC + (new.tv_nsec - old.tv_nsec);
}

static void usage()
{
  puts("Usage:\n\tload-authn <username> <password>\n");
  exit(1);
}

static volatile int finish = 0;

void end(int __attribute__((unused)) signal)
{
  finish = 1;
}

int main(int argc, char **argv)
{
  if (argc < 3)
    usage();
  
  signal(SIGINT, end);
  
  const char * username = argv[1];
  const char * password = argv[2];
  
  char token[1024];
  
  struct timespec last_time, first_time, new_time;
  
  clock_gettime(CLOCK_MONOTONIC, &first_time);
  last_time = first_time;
  
  int count = 0;
  int last_count = 0;
  
  while (!finish) {
    conjur_authenticate(username, password, token);
    count++;
    
    clock_gettime(CLOCK_MONOTONIC, &new_time);
    
    if (last_time.tv_sec != new_time.tv_sec) {
      unsigned long ns = timediff_ns(new_time, last_time);
      printf("%.02f requests per second\n", 1.0 * (count - last_count) * NS_IN_SEC / ns);
      last_count = count;
      last_time = new_time;
    }
  }
  
  const float seconds = 1.0 * timediff_ns(new_time, first_time) / NS_IN_SEC;
  printf("\n\n%d requests total in %.02f seconds (average %.02f per second).\n", count, seconds, 1.0 * count / seconds);
  return 0;
}
