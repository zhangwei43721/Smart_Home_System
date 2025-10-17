#ifndef OPENAI_INTERNAL_H
#define OPENAI_INTERNAL_H

#include <libs/curl/include/curl/curl.h>
#include <stdlib.h>
#include <string.h>

struct memory {
  char* response;
  size_t size;
};

extern char* api_key;

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

#endif  // OPENAI_INTERNAL_H
