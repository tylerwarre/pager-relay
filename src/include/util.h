#ifndef UTIL_H
#define UTIL_H

#include <json-c/json.h>

void util_detach_json_child(struct json_object *parent, char *key, struct json_object *child);
void util_detach_json_child_idx(struct json_object *parent, int child_index, struct json_object *child);
int util_print_json_str(struct json_object *parent, char *key);

#endif