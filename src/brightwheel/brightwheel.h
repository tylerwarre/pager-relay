#ifndef BRIGHTWHEEL_H
#define BRIGHTWHEEL_H

#include "settings.h"
#include "state.h"

#define TIMESTAMP_LEN 24

static const char BRIGHT_API_URL[] = "https://schools.mybrightwheel.com/api/v2/guardians/8bb273a7-0b9c-47b5-a35b-87d1500b58a2/message_threads/69de9e3d-7da5-ce78-dc92-1bbe46f51eff/messages?page_limit=5";
// Example 2026-04-14T20:06:21.107Z
static const char TIMESTAMP_FMT[] = "%Y-%m-%dT%H:%M:%S";

int bright_get_msgs(BrightwheelSettings *s, struct json_object **msgs);
int bright_get_unread(BrightState *state, json_object *msgs, struct json_object **msg);
static int bright_get_msg_count(BrightwheelSettings *s);
static struct json_object* bright_parse_msgs(BrightwheelSettings *s, char *j_str);
static time_t bright_get_timestamp(struct json_object *obj);

#endif