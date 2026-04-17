#include <stdbool.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "brightwheel.h"
#include "http.h"

struct json_object* bright_get_msgs(BrightwheelSettings *s) {
    CURL *curl = NULL;
    CURLcode errno = CURLE_OK;
    HttpResponse resp = {0};
    char *cookies = NULL;
    struct json_object *j_msgs = NULL;
    
    if ((curl = curl_easy_init()) == NULL) {
        fprintf(stderr, "[%s] Unable to initalize curl for request\n", __func__);
        return NULL;
    }

    // Build cookies
    // +1 to account for NULL characer
    // +16 for length of cookie key
    if ((cookies = calloc(strlen(s->token)+1+16, 1)) == NULL) {
        fprintf(stderr, "[%s] Unable to allocate memory for cookies\n", __func__);
        return NULL;
    }
    strcpy(cookies, "_brightwheel_v2=");
    strcat(cookies, s->token);

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
    // Setup authentication cookie
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies);
    #ifndef NDEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    #endif

    while (true) {
        if ((errno = curl_easy_perform(curl)) != CURLE_OK) {
            fprintf(stderr, "[%s] Unable to process curl request: %s\n", __func__, curl_easy_strerror(errno));
            break;
        }

        if ((j_msgs = bright_parse_msgs(s, resp.data)) == NULL) {
            break;
        }

        break;
    }

    if (cookies != NULL) {
        free(cookies);
        cookies = NULL;
    }

    if (resp.data != NULL) {
        free(resp.data);
        resp.data = NULL;
    }

    curl_easy_cleanup(curl);

    return j_msgs;
}

static struct json_object* bright_parse_msgs(BrightwheelSettings *s, char *msgs) {
    struct json_object *obj = NULL;

    #ifndef NDEBUG
        enum json_tokener_error err;
        obj = json_tokener_parse_verbose(msgs, &err);
        if (err != json_tokener_success) {
            fprintf(stderr, "[%s] Unable to parse json_string:\n\tErr: %s\n\tString: %s\n", __func__, json_tokener_error_desc(err), msgs);
            return NULL;
        }
    #else
        if ((obj = json_tokener_parse(msgs)) == NULL) {
            fprintf(stderr, "[%s] Unable to parse json_string:\n\tString: %s\n", __func__, msgs);
            return NULL;
        }
    #endif

    return obj;
}