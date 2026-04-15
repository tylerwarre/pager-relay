#include <json-c/json.h>
#include <curl/curl.h>

#include <stdio.h>
#include <stdbool.h>

#include "include/error.h"
#include "include/settings.h"

int main() {
    int ret;
    RelaySettings *s;

    if ((ret = settings_read(s)) != E_SUCCESS) {
        return ret;
    }

    settings_free(s);

    return 0;
}