

#include "openai.h"
#include "openai_internal.h"

extern char* api_key;
extern size_t write_callback(void*, size_t, size_t, void*);

char* openai_create_embedding_json(const char* input, const char* model) {
  if (!api_key || !input || !model) return NULL;

  CURL* curl = curl_easy_init();
  if (!curl) return NULL;

  struct memory chunk = {malloc(1), 0};
  struct curl_slist* headers = NULL;

  char auth_header[512];
  snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s",
           api_key);
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, auth_header);

  cJSON* root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "input", input);
  cJSON_AddStringToObject(root, "model", model);
  char* payload = cJSON_PrintUnformatted(root);

  curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com//embeddings");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&chunk);

  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  cJSON_Delete(root);
  free(payload);

  if (res != CURLE_OK) {
    free(chunk.response);
    return NULL;
  }

  return chunk.response;
}

float* openai_create_embedding_array(const char* input, const char* model,
                                     size_t* length) {
  char* response = openai_create_embedding_json(input, model);
  if (!response) return NULL;

  cJSON* root = cJSON_Parse(response);
  if (!root) {
    free(response);
    return NULL;
  }

  cJSON* data = cJSON_GetObjectItem(root, "data");
  if (!cJSON_IsArray(data)) {
    cJSON_Delete(root);
    free(response);
    return NULL;
  }

  cJSON* item = cJSON_GetArrayItem(data, 0);
  cJSON* embedding = cJSON_GetObjectItem(item, "embedding");
  if (!cJSON_IsArray(embedding)) {
    cJSON_Delete(root);
    free(response);
    return NULL;
  }

  size_t count = cJSON_GetArraySize(embedding);
  float* result = malloc(sizeof(float) * count);
  if (!result) {
    cJSON_Delete(root);
    free(response);
    return NULL;
  }

  for (size_t i = 0; i < count; i++) {
    cJSON* val = cJSON_GetArrayItem(embedding, i);
    result[i] = (float)(val->valuedouble);
  }

  *length = count;
  cJSON_Delete(root);
  free(response);
  return result;
}