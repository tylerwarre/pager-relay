#include <stdbool.h>
#include <string.h>
#define __USE_XOPEN // Required for strptime
#include <time.h>

#include <curl/curl.h>
#include <json-c/json.h>

#include "brightwheel.h"
#include "http.h"
#include "error.h"

int bright_get_msgs(BrightwheelSettings *s, struct json_object **msgs) {
    CURL *curl = NULL;
    CURLcode ret = E_SUCCESS;
    HttpResponse resp = {0};
    char *cookies = NULL;
    
    if ((curl = curl_easy_init()) == NULL) {
        fprintf(stderr, "[%s] Unable to initalize curl for request\n", __func__);
        return (int)CURLE_FAILED_INIT;
    }

    // Build cookies
    // +1 to account for NULL characer
    // +16 for length of cookie key
    if ((cookies = calloc(strlen(s->token)+1+16, 1)) == NULL) {
        fprintf(stderr, "[%s] Unable to allocate memory for cookies\n", __func__);
        return E_OUTOFMEMORY;
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
        if ((ret = curl_easy_perform(curl)) != CURLE_OK) {
            fprintf(stderr, "[%s] Unable to process curl request: %s\n", __func__, curl_easy_strerror(ret));
            break;
        }

        if ((*msgs = bright_parse_msgs(s, resp.data)) == NULL) {
            fprintf(stderr, "[%s] Unable to parse json response\n", __func__);
            ret = E_JSON_PARSE;
            break;
        }

        if (json_object_object_get_ex(*msgs, "results", msgs) == false) {
            fprintf(stderr, "[%s] Unable to get results array from response\n", __func__);
            ret = E_JSON_ACCESS;
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

    return ret;
}

int bright_get_last_msg(BrightState *state, json_object *msgs, char *msg) {
    int len = 0;
    int ret = E_SUCCESS;
    struct json_object *j_msg = NULL;
    struct json_object *j_obj = NULL;
    struct tm tm = {0};
    time_t timestamp;
    const char *str = NULL;

    if((len = json_object_array_length(msgs)) < 1) {
        fprintf(stderr, "[%s] message array is empty\n", __func__);
        return E_EMPTY;
    }

    // TODO: Testing
    //state->lastTimestamp = 1776222379;

    // TODO: handle case when 1st message is newer, but last
    for (int i = 0; i < len; i++) {
        if ((j_msg = json_object_array_get_idx(msgs, i)) == NULL) {
            fprintf(stderr, "[%s] Unable to get message at index: %d\n", __func__, i);
            ret =  E_JSON_PARSE;
            break;
        }

        // Get "message" object
        if (json_object_object_get_ex(j_msg, "message", &j_msg) == false) {
            fprintf(stderr, "[%s] Unable to get message data at index: %d\n", __func__, i);
            ret = E_JSON_PARSE;
            break;
        }

        // Get message timestamp object
        if (json_object_object_get_ex(j_msg, "created_at", &j_obj) == false) {
            fprintf(stderr, "[%s] Unable to get message timestamp at index: %d\n", __func__, i);
            ret = E_JSON_PARSE;
            break;
        }

        if ((str = json_object_get_string(j_obj)) == NULL) {
            fprintf(stderr, "[%s] Unable to access message timestamp string\n", __func__);
            ret = E_JSON_ACCESS;
            break;
        }

        if (strptime(str, TIMESTAMP_FMT, &tm) == NULL) {
            fprintf(stderr, "[%s] Unable to convert timestamp string to time object\n", __func__);
            ret = E_CONVERT;
            break;
        }

        if ((timestamp = mktime(&tm)) == (time_t)-1) {
            fprintf(stderr, "[%s] Unable to convert timestamp object\n", __func__);
            ret = E_CONVERT;
            break;
        }

        // Check if there are no new messages
        if (state->lastTimestamp > timestamp && i == 0) {
            printf("No new message\n");
            break;
        }
        // TODO: verify this works
        // TODO: do something if we know we have a new message
        else if (timestamp > state->lastTimestamp)
        {
            printf("New message!\n");
            break;
        }
        
    }

    return ret;
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