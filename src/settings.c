#include <json-c/json.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "settings.h"
#include "util.h"

int settings_read(RelaySettings *s) {
    int ret = E_SUCCESS;
    struct json_object *root;
    struct json_object *node;
    struct json_object *obj = NULL;

    while (true) {
        if ((root = json_object_from_file("settings.json")) == NULL) {
            fprintf(stderr, "Unable to load settings: %s\n", json_util_get_last_err());
            ret = E_JSON_PARSE;
            break;
        }

        if(json_object_object_get_ex(root, "brightwheel", &node) == false) {
            fprintf(stderr, "Unable to load brightwheel settings: %s\n", json_util_get_last_err());
            ret =  E_JSON_PARSE;
            break;
        }

        if ((ret = util_json_get_str(node, "token", &(s->brightwheel->token))) != E_SUCCESS) {
            break;
        }

        if ((ret = util_json_get_bool(node, "includeBroadcasts", &(s->brightwheel->includeBroadcasts))) != E_SUCCESS) {
            break;
        }

        if ((ret = util_json_get_bool(node, "includeGuardians", &(s->brightwheel->includeGuardians))) != E_SUCCESS) {
            break;
        }

        if(json_object_object_get_ex(node, "token", &obj) == false) {
            fprintf(stderr, "Unable to load brightwheel token: %s\n", json_util_get_last_err());
            ret =  E_JSON_PARSE;
            break;
        }

        if ((ret = settings_validate(s)) != E_SUCCESS) {
            break;
        }
        // TODO: implement email settings
        break;
    }

    // Freeing the root object frees all children
    json_object_put(root);

    return ret;
}

static int settings_validate(RelaySettings *s) {
    if (strlen(s->brightwheel->token) == 0) {
        fprintf(stderr, "Brightwheel token is empty");
        return E_EMPTY;
    }

    // TODO: implement email settings

    return E_SUCCESS;
}

RelaySettings* settings_new() {
    RelaySettings *s = NULL;
    if ((s = calloc(sizeof(RelaySettings), 1)) == NULL) {
        fprintf(stderr, "Unable to allocate memory for settings struct\n");
        return NULL;
    }

    if ((s->brightwheel = calloc(sizeof(BrightSettings), 1)) == NULL) {
        fprintf(stderr, "Unable to allocate memory for Brightwheel struct\n");
        return NULL;
    }

    if ((s->email = calloc(sizeof(EmailSettings), 1)) == NULL) {
        fprintf(stderr, "Unable to allocate memory for Email struct\n");
        return NULL;
    }

    return s;
}

void settings_free(RelaySettings *s) {
    settings_brightwheel_free(s->brightwheel);
    settings_email_free(s->email);

    free(s);
    s = NULL;
}

static void settings_brightwheel_free(BrightSettings *s) {
    if (s->token != NULL) {
        free(s->token);
        s->token = NULL;
    }

    if (s != NULL) {
        free(s);
        s = NULL;
    }
}

static void settings_email_free(EmailSettings *s) {
    if (s->sender != NULL) {
        free(s->sender);
        s->sender = NULL;
    }

    if (s->receipients != NULL) {
        free(s->receipients);
        s->receipients = NULL;
    }

    if (s->password != NULL) {
        free(s->password);
        s->password = NULL;
    }

    // TODO: implement email free
    if (s != NULL) {
        free(s);
        s = NULL;
    }
    return;
}