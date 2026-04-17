#include <json-c/json.h>
#include <curl/curl.h>

#include <stdio.h>
#include <stdbool.h>

#include "include/error.h"
#include "include/settings.h"
#include "brightwheel/brightwheel.h"

int main() {
    int ret = E_SUCCESS;
    RelaySettings *s = NULL;
    CURLcode c_errno = CURLE_OK;
    struct json_object *msgs = NULL;

    if ((s = settings_new()) == NULL) {
        return E_OUTOFMEMORY;
    }

    if ((ret = settings_read(s)) != E_SUCCESS) {
        return ret;
    }

    if ((c_errno = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK) {
        fprintf(stderr, "Unable to initalize curl: %s\n", curl_easy_strerror(c_errno));
        return (int)c_errno;
    }

    // TODO: implement a loop from here down until the free functions
    
    // TODO: update so that bright_get_msgs() returns an int
    if ((msgs = bright_get_msgs(s->brightwheel)) == NULL) {
        return EXIT_FAILURE;
    }

    int count = 0;
    struct json_object *obj;
    if(json_object_object_get_ex(msgs, "count", &obj) == false) {
        return EXIT_FAILURE;
    }
    if((count = json_object_get_int(obj)) == 0) {
        return EXIT_FAILURE;
    }

    printf("Message Count: %d\n", count);

    json_object_put(msgs);

    settings_free(s);

    curl_global_cleanup();

    return 0;
}