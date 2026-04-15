#ifndef SETTINGS_H
#define SETTINGS_H

#include <json-c/json_object.h>

typedef struct BrightwheelSettings {
    struct json_object *token;
} BrightwheelSettings;

typedef struct EmailSettings {
    struct json_object *sender;
    struct json_object *receipients;
    struct json_object *password;
} EmailSettings;

typedef struct RelaySettings {
    EmailSettings *email;
    BrightwheelSettings *brightwheel;
} RelaySettings;


int settings_read(RelaySettings *s);
static int settings_validate(RelaySettings *s);
void settings_free(RelaySettings *s);
static void settings_brightwheel_free(BrightwheelSettings *s);
static void settings_email_free(EmailSettings *s);

#endif