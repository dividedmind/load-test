#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <curl/curl.h>

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

static size_t noop_write(char *ptr, size_t size, size_t nmemb, void *_buf)
{
  return size * nmemb;
}

int main(int argc, char **argv)
{
  if (argc < 3)
    usage();
  
  signal(SIGINT, end);
  
  const char * username = argv[1];
  const char * password = argv[2];
  
  char * base_url = getenv("CONJUR_AUTHN_URL");
  if (!base_url)
    base_url = "http://localhost:5000";
  
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();
  if (!curl) {
    fputs("curl_easy_init() failed", stderr);
    return 2;
  }
  
  char authn_url[256];
  snprintf(authn_url, 256, "%s/users/%s/authenticate", base_url, username);
  curl_easy_setopt(curl, CURLOPT_URL, authn_url);
  
  printf("Load testing %s...\n", authn_url);
  
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, password);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, noop_write);
  
  struct timespec last_time, first_time, new_time;
  
  clock_gettime(CLOCK_MONOTONIC, &first_time);
  last_time = first_time;
  
  int count = 0;
  int last_count = 0;
  
  #pragma omp parallel num_threads(16)
  {
    CURL *_curl = curl_easy_duphandle(curl);
    
    while (!finish) {
      CURLcode res = curl_easy_perform(_curl);
      if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
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
    
    curl_easy_cleanup(_curl);
  }
  
  const float seconds = 1.0 * timediff_ns(new_time, first_time) / NS_IN_SEC;
  printf("\n\n%d requests total in %.02f seconds (average %.02f per second).\n", count, seconds, 1.0 * count / seconds);
  return 0;
}
