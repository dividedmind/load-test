#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#include "load.h"

static size_t noop_write(char *ptr, size_t size, size_t nmemb, void *_buf)
{
  return size * nmemb;
}

static void usage()
{
  puts("Usage:\n\tload-authn <username> [<password>]\n\n"
       "Password can also be passed in CONJUR_PASSWD environment variable.");
  exit(1);
}

static CURL *curl;

int load_test_init(int argc, char **argv)
{
  if (argc < 2)
    usage();
  
  const char * username = argv[1];
  char * password;
  if (argc == 3)
    password = argv[2];
  else if (!(password = getenv("CONJUR_PASSWD")))
    usage();
  
  char * base_url = getenv("CONJUR_AUTHN_URL");
  if (!base_url)
    base_url = "http://localhost:5000";
  
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
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
  
  return 0;
}

static CURL *_curl;
#pragma omp threadprivate(_curl)

int load_test_prepare()
{
  _curl = curl_easy_duphandle(curl);
  return !curl;
}

int load_test_run()
{
  CURLcode res = curl_easy_perform(_curl);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    return res;
  }
  
  return 0;
}
