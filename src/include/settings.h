#ifndef SETTINGS_H
#define SETTINGS_H

#include <json-c/json_object.h>

typedef struct BrightSettings {
    char *token;
    bool includeBroadcasts;
    bool includeGuardians;
} BrightSettings;

typedef struct EmailSettings {
    char *sender;
    char *receipients;
    char *password;
} EmailSettings;

typedef struct RelaySettings {
    EmailSettings *email;
    BrightSettings *brightwheel;
} RelaySettings;


int settings_read(RelaySettings *s);
static int settings_validate(RelaySettings *s);
RelaySettings* settings_new();
void settings_free(RelaySettings *s);
static void settings_brightwheel_free(BrightSettings *s);
static void settings_email_free(EmailSettings *s);

#endif