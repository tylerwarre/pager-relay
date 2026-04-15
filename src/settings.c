#include <json-c/json.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "include/error.h"
#include "settings.h"

int settings_read(RelaySettings *s) {
    int ret;
    struct json_object *root;
    struct json_object *node;

    if ((s = malloc(sizeof(RelaySettings))) == NULL) {
        fprintf(stderr, "Unable to allocate memory for settings struct\n");
        return E_OUTOFMEMORY;
    }

    if ((root = json_object_from_file("settings.json")) == NULL) {
        fprintf(stderr, "Unable to load settings: %s\n", json_util_get_last_err());
        return E_SETTINGS_PARSE;
    }

    if(json_object_object_get_ex(root, "brightwheel", &node) == false) {
        fprintf(stderr, "Unable to load brightwheel settings: %s\n", json_util_get_last_err());
        return E_SETTINGS_PARSE;
    }

    if(json_object_object_get_ex(node, "token", &(s->brightwheel->token)) == false) {
        fprintf(stderr, "Unable to load brightwheel token: %s\n", json_util_get_last_err());
        return E_SETTINGS_ACCESS;
    }

    json_object_put(node);

    // TODO implement email settings

    json_object_put(root);

    return settings_validate(s);
}

static int settings_validate(RelaySettings *s) {
    if (json_object_get_string_len(s->brightwheel->token) == 0) {
        fprintf(stderr, "Brightwheel token is empty");
        return E_SETTINGS_EMPTY;
    }

    // TODO implement email settings

    return E_SUCCESS;
}

void settings_free(RelaySettings *s) {
    settings_brightwheel_free(s->brightwheel);
    settings_email_free(s->email);

    free(s);
    s = NULL;
}

static void settings_brightwheel_free(BrightwheelSettings *s) {
    if (s->token != NULL) {
        json_object_put(s->token);
        s->token = NULL;
    }
}

static void settings_email_free(EmailSettings *s) {
    // TODO implement email free
    return;
}