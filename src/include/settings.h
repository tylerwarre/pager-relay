#ifndef SETTINGS_H
#define SETTINGS_H

#include <json-c/json_object.h>

typedef struct BrightwheelSettings {
    char *token;
} BrightwheelSettings;

typedef struct EmailSettings {
    char *sender;
    char *receipients;
    char *password;
} EmailSettings;

typedef struct RelaySettings {
    EmailSettings *email;
    BrightwheelSettings *brightwheel;
} RelaySettings;


int settings_read(RelaySettings *s);
static int settings_validate(RelaySettings *s);
static int settings_get_str(struct json_object *obj, char **dest);
RelaySettings* settings_new();
void settings_free(RelaySettings *s);
static void settings_brightwheel_free(BrightwheelSettings *s);
static void settings_email_free(EmailSettings *s);

#endif