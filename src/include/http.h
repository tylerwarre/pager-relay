#ifndef HTTP_H
#define HTTP_H

#include <stdlib.h>

typedef struct HttpResponse {
    char *data;
    size_t size;
} HttpResponse;

size_t cb_http_write(char *data, size_t size, size_t nmemb, void *userdata);

#endif
