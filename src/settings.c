#include <json-c/json.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/error.h"
#include "settings.h"

int settings_read(RelaySettings *s) {
    int ret = E_SUCCESS;
    struct json_object *root;
    struct json_object *node;
    struct json_object *obj = NULL;

    while (true) {
        if ((root = json_object_from_file("settings.json")) == NULL) {
            fprintf(stderr, "Unable to load settings: %s\n", json_util_get_last_err());
            ret = E_SETTINGS_PARSE;
            break;
        }

        if(json_object_object_get_ex(root, "brightwheel", &node) == false) {
            fprintf(stderr, "Unable to load brightwheel settings: %s\n", json_util_get_last_err());
            ret =  E_SETTINGS_PARSE;
            break;
        }

        if(json_object_object_get_ex(node, "token", &obj) == false) {
            fprintf(stderr, "Unable to load brightwheel token: %s\n", json_util_get_last_err());
            ret =  E_SETTINGS_PARSE;
            break;
        }

        if ((ret = settings_get_str(obj, &(s->brightwheel->token))) != E_SUCCESS) {
            break;
        }

        if ((ret = settings_validate(s)) != E_SUCCESS) {
            break;
        }
        // TODO implement email settings
        break;
    }

    json_object_put(node);
    json_object_put(root);

    return ret;
}

static int settings_validate(RelaySettings *s) {
    if (strlen(s->brightwheel->token) == 0) {
        fprintf(stderr, "Brightwheel token is empty");
        return E_SETTINGS_EMPTY;
    }

    // TODO implement email settings

    return E_SUCCESS;
}

static int settings_get_str(struct json_object *obj, char **dest) {
    const char *str = NULL;
    int len = 0;
    int ret = E_SUCCESS;

    while (true) {
        if((str = json_object_get_string(obj)) == NULL) {
            fprintf(stderr, "Unable to load brightwheel token: %s\n", json_util_get_last_err());
            ret =  E_SETTINGS_ACCESS;
            break;
        }
        len = strlen(str);

        if ((*dest = malloc(len+1)) == NULL) {
            fprintf(stderr, "%s: Unable to allocate memory for string\n", __func__);
            ret = E_OUTOFMEMORY;
            break;
        }

        strncpy(*dest, str, len);
        break;
    }

    json_object_put(obj);

    return ret;
}

RelaySettings* settings_new() {
    RelaySettings *s = NULL;
    if ((s = malloc(sizeof(RelaySettings))) == NULL) {
        fprintf(stderr, "Unable to allocate memory for settings struct\n");
        return NULL;
    }

    if ((s->brightwheel = malloc(sizeof(BrightwheelSettings))) == NULL) {
        fprintf(stderr, "Unable to allocate memory for Brightwheel struct\n");
        return NULL;
    }

    if ((s->email = malloc(sizeof(EmailSettings))) == NULL) {
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

static void settings_brightwheel_free(BrightwheelSettings *s) {
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

    // TODO implement email free
    if (s != NULL) {
        free(s);
        s = NULL;
    }
    return;
}