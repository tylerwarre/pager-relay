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
    struct json_object *j_reponse = NULL;
    
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
    
        #ifndef NDEBUG
            printf("Response: %s\n", resp.data);
        #endif

        if ((j_reponse = bright_parse_msgs(s, resp.data)) == NULL) {
            fprintf(stderr, "[%s] Unable to parse json response\n", __func__);
            ret = E_JSON_PARSE;
            break;
        }

        if (json_object_object_get_ex(j_reponse, "results", msgs) == false) {
            fprintf(stderr, "[%s] Unable to get results array from response\n", __func__);
            ret = E_JSON_ACCESS;
            break;
        }

        json_object_get(*msgs);

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

    json_object_object_del(j_reponse, "results");
    json_object_put(j_reponse);

    curl_easy_cleanup(curl);

    return ret;
}

int bright_get_last_msg(BrightState *state, json_object *msgs, struct json_object **msg) {
    int len = 0;
    int ret = E_SUCCESS;
    struct json_object *j_next_msg = NULL;
    time_t timestamp = 0;

    if((len = json_object_array_length(msgs)) < 1) {
        fprintf(stderr, "[%s] message array is empty\n", __func__);
        return E_EMPTY;
    }

    // TODO: Testing
    state->lastTimestamp = 1776197160;

    for (int i = 0; i < len; i++) {
        if ((j_next_msg = json_object_array_get_idx(msgs, i)) == NULL) {
            fprintf(stderr, "[%s] Unable to get message at index: %d\n", __func__, i);
            ret =  E_JSON_PARSE;
            break;
        }

        // Get "message" object
        if (json_object_object_get_ex(j_next_msg, "message", &j_next_msg) == false) {
            fprintf(stderr, "[%s] Unable to get message data at index: %d\n", __func__, i);
            ret = E_JSON_PARSE;
            break;
        }

        // Get timestamp
        if ((timestamp = bright_get_timestamp(j_next_msg)) == 0) {
            fprintf(stderr, "[%s] Unable to get message timestamp at index: %d\n", __func__, i);
            ret = E_JSON_PARSE;
            break;
        }

        // Check if there are no new messages
        if (state->lastTimestamp > timestamp && i == 0) {
            printf("No new message\n");
            break;
        }
        else if (timestamp > state->lastTimestamp)
        {
            printf("New message!\n");
            *msg = j_next_msg;
            continue;
        }
        else if ((*msg) != NULL) {
            printf("%d is the first unread message index\n", i);

            if ((timestamp = bright_get_timestamp(j_next_msg)) == 0) {
                fprintf(stderr, "[%s] Unable to get unread message timestamp at index: %d\n", __func__, i);
                ret = E_JSON_PARSE;
                break;
            }

            state->lastTimestamp = timestamp;

            break;
        }
    }

    return ret;
}

static struct json_object* bright_parse_msgs(BrightwheelSettings *s, char *j_str) {
    struct json_object *obj = NULL;

    #ifndef NDEBUG
        enum json_tokener_error err;
        obj = json_tokener_parse_verbose(j_str, &err);
        if (err != json_tokener_success) {
            fprintf(stderr, "[%s] Unable to parse json_string:\n\tErr: %s\n\tString: %s\n", __func__, json_tokener_error_desc(err), j_str);
            return NULL;
        }
    #else
        if ((obj = json_tokener_parse(j_str)) == NULL) {
            fprintf(stderr, "[%s] Unable to parse json_string:\n\tString: %s\n", __func__, j_str);
            return NULL;
        }
    #endif

    return obj;
}

static time_t bright_get_timestamp(struct json_object *msg) {
    struct tm tm = {0};
    struct json_object *obj = NULL;
    const char *str = NULL;
    time_t timestamp = 0;

    while (true)
    {
        // Get message timestamp object
        if (json_object_object_get_ex(msg, "created_at", &obj) == false)
        {
            timestamp = 0;
            break;
        }

        if ((str = json_object_get_string(obj)) == NULL)
        {
            timestamp = 0;
            break;
        }

        if (strptime(str, TIMESTAMP_FMT, &tm) == NULL)
        {
            timestamp = 0;
            break;
        }

        if ((timestamp = mktime(&tm)) == (time_t)-1)
        {
            timestamp = 0;
            break;
        }
        break;
    }

    //json_object_put(obj);

    return timestamp;
}