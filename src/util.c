#include <stdbool.h>
#include <stdio.h>

#include "util.h"
#include "error.h"

void util_detach_json_child_idx(struct json_object *parent, int child_index, struct json_object *child) {
    // Increment child object references
    json_object_get(child);

    // Remove child from parent
    json_object_array_del_idx(parent, child_index, 1);

    // Free parent
    json_object_put(parent);
}

void util_detach_json_child(struct json_object *parent, char *key, struct json_object *child) {
    // Increment child object references
    json_object_get(child);

    // Remove child from parent
    json_object_object_del(parent, key);

    // Free parent
    json_object_put(parent);
}

int util_print_json_str(struct json_object *parent, char *key) {
    struct json_object *obj;
    const char *body;
    int ret = E_SUCCESS;

    while (true) {
        if ((json_object_object_get_ex(parent, key, &obj)) == false) {
            ret = E_JSON_PARSE;
            break;
        }
        if ((body = json_object_get_string(obj)) == NULL) {
            ret = E_JSON_ACCESS;
            break;
        }

        printf("body: %s\n", body);
        break;
    }

    json_object_put(obj);

    return ret;
}