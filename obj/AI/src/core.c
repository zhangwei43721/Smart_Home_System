// core.c

#include "openai.h"
#include "openai_internal.h"

char* api_key = NULL;

void openai_init(const char* key) {
  if (api_key) free(api_key);
  api_key = strdup(key);
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

void openai_cleanup() {
  if (api_key) free(api_key);
  api_key = NULL;
  curl_global_cleanup();
}

size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
  size_t realsize = size * nmemb;
  struct memory* mem = (struct memory*)userp;

  char* ptr = realloc(mem->response, mem->size + realsize + 1);
  if (!ptr) return 0;

  mem->response = ptr;
  memcpy(&(mem->response[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->response[mem->size] = 0;

  return realsize;
}
