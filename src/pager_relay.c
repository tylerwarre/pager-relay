#include <json-c/json.h>
#include <curl/curl.h>
#include <pcre2.h>

#include <stdio.h>
#include <stdbool.h>

#include "include/error.h"
#include "include/settings.h"
#include "include/state.h"
#include "brightwheel/brightwheel.h"
#include "include/util.h"

int main() {
    int ret = E_SUCCESS;
    CURLcode c_errno = CURLE_OK;
    RelaySettings *s = NULL;
    RelayState *state = NULL;
    struct json_object *msgs = NULL;
    struct json_object *msg = NULL;

    if ((s = settings_new()) == NULL) {
        return E_OUTOFMEMORY;
    }

    if ((ret = settings_read(s)) != E_SUCCESS) {
        return ret;
    }

    // TODO: Figure out how to not cause failure to leave settings allocated
    if ((state = state_new()) == NULL) {
        return E_OUTOFMEMORY;
    }

    if ((c_errno = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK) {
        fprintf(stderr, "Unable to initalize curl: %s\n", curl_easy_strerror(c_errno));
        return (int)c_errno;
    }

    // TODO: implement a loop from here down until the free functions
    
    if ((ret = bright_get_msgs(s->brightwheel, &msgs)) != E_SUCCESS) {
        return ret;
    }


    if ((ret = bright_get_unread(state->brightState, msgs, &msg)) != E_SUCCESS) {
        return ret;
    }

    // TODO: Testing
    char *body;
    pcre2_match_data_8 *matches;
    util_json_get_str(msg, "body", &body);
    util_re_match((PCRE2_SPTR)RE_PATTERN_UTF8, (PCRE2_SPTR)body, &matches);
    pcre2_match_data_free(matches);
    free(body);
    // TODO: End Testing

    json_object_put(msg);

    //json_object_put(msgs);

    // TODO: End loop here

    // Free Relay objects
    settings_free(s);
    state_free(state);

    curl_global_cleanup();

    return 0;
}