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
    
    printf("token: %s", s->brightwheel->token);

    bright_get_msgs(s->brightwheel);

    settings_free(s);

    curl_global_cleanup();

    return 0;
}