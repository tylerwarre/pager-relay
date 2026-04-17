#ifndef BRIGHTWHEEL_H
#define BRIGHTWHEEL_H

#include "settings.h"

static const char BRIGHT_API_URL[] = "https://schools.mybrightwheel.com/api/v2/guardians/8bb273a7-0b9c-47b5-a35b-87d1500b58a2/message_threads/69de9e3d-7da5-ce78-dc92-1bbe46f51eff/messages?page_limit=5";

static int bright_get_msg_count(BrightwheelSettings *s);
struct json_object* bright_get_msgs(BrightwheelSettings *s);
static struct json_object* bright_parse_msgs(BrightwheelSettings *s, char *msgs);

#endif