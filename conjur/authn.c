#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "authn.h"

#define CURL_INIT_ERR 1
#define CURL_ERROR 2

#define BUFSIZE 1024

typedef struct {
  char buffer[BUFSIZE];
  char * end;
} read_buf;

static void init_read_buf(read_buf *buf)
{
  buf->end = buf->buffer;
}

static size_t data_read(char *ptr, size_t size, size_t nmemb, void *_buf)
{
  read_buf *buf = _buf;
  
  size = size * nmemb;
  size_t left = buf->buffer + BUFSIZE - buf->end;
  if (size > left)
    size = left;
  
  memcpy(buf->buffer, ptr, size);
  buf->end += size;
  
  return size;
}

static char * base_url()
{
  static char * url = NULL;
  if (!url) {
    url = getenv("CONJUR_AUTHN_URL");
    if (!url) url = "http://localhost:5000";
  }
  
  return url;
}

static char * authentication_url(const char * username)
{
  static char url[256];
  snprintf(url, 256, "%s/users/%s/authenticate", base_url(), username);
  return url;
}

int conjur_authenticate(const char * username, const char * password, char * token)
{
  CURL *curl = NULL;
  CURLcode res;

  if (!curl) {
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
  }
  
  if (!curl)
    return CURL_INIT_ERR;
  
  read_buf buffer;
  init_read_buf(&buffer);
  
  curl_easy_setopt(curl, CURLOPT_URL, authentication_url(username));
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, password);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, data_read); 
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
  
  res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    return CURL_ERROR;
  }
  
  if (token)
    memcpy(token, buffer.buffer, buffer.end - buffer.buffer);
  
  return 0;
}
