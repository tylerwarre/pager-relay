#include <stdbool.h>

#include <curl/curl.h>

#include "brightwheel.h"
#include "http.h"

struct json_object* bright_get_msgs(BrightwheelSettings *s) {
    CURL *curl = NULL;
    CURLcode errno = CURLE_OK;
    HttpResponse resp = {0};
    
    if ((curl = curl_easy_init()) == NULL) {
        fprintf(stderr, "[%s] Unable to initalize curl for request\n", __func__);
        return NULL;
    }

    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, BRIGHT_API_URL);
    // Disable tls verification
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    // Cache CA certsfor a day
    curl_easy_setopt(curl, CURLOPT_CA_CACHE_TIMEOUT, 86400L);
    // Assign buffer for curl response
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb_http_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &resp);
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    while (true) {
        if ((errno = curl_easy_perform(curl)) != CURLE_OK) {
            fprintf(stderr, "[%s] Unable to process curl request: %s\n", __func__, curl_easy_strerror(errno));
            break;
        }
        // TODO: Implement parsing as json
        printf("response: %s\n", resp.data);
        break;
    }

    if (resp.data != NULL) {
        free(resp.data);
        resp.data = NULL;
    }

    curl_easy_cleanup(curl);

    return NULL;
}