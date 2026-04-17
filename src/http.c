#include <string.h>

#include "http.h"

size_t cb_http_write(char *data, size_t size, size_t nmemb, void *userdata) {
    size_t chunk_size = nmemb;
    char *ptr = NULL;

    HttpResponse *response = (HttpResponse *)userdata;

    // We use realloc becuase data may come in chunked from a single request
    if ((ptr = realloc(response->data, response->size + chunk_size + 1)) == NULL) {
        return 0;
    }

    response->data = ptr;
    // Copy data into the response starting at the end of the last chunk
    memcpy(&(response->data[response->size]), data, chunk_size);
    response->size += chunk_size;
    // Null terminate the string
    response->data[response->size] = 0;

    return chunk_size;
}