#include <json-c/json.h>
#include <curl/curl.h>

#include <stdio.h>
#include <stdbool.h>

#include "include/error.h"
#include "include/settings.h"

int main() {
    int ret = E_SUCCESS;
    RelaySettings *s = NULL;

    if ((s = settings_new()) == NULL) {
        return E_OUTOFMEMORY;
    }

    if ((ret = settings_read(s)) != E_SUCCESS) {
        return ret;
    }

    printf("token: %s", s->brightwheel->token);

    settings_free(s);

    return 0;
}