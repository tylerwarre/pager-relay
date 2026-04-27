#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <pcre2.h>

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

int util_json_get_str(struct json_object *node, char *key, char **dest) {
    const char *str = NULL;
    int len = 0;
    struct json_object *obj = NULL;

    if(json_object_object_get_ex(node, key, &obj) == false) {
        fprintf(stderr, "Unable to parse json string with key: %s\n", key);
        return E_JSON_PARSE;
    }

    if((str = json_object_get_string(obj)) == NULL) {
        fprintf(stderr, "Unable to access json string with key: %s\n", key);
        return E_JSON_ACCESS;
    }
    len = strlen(str);

    if ((*dest = calloc(len+1, 1)) == NULL) {
        fprintf(stderr, "Ran out of memory when allocating json string with key: %s\n", key);
        return E_OUTOFMEMORY;
    }

    strncpy(*dest, str, len);

    return E_SUCCESS;
}

/** Gets the boolean value from the passed in object.
 * On failure it always returns true
 */
int util_json_get_bool(struct json_object *node, char *key, bool *value) {
    struct json_object *obj = NULL;
    if(json_object_object_get_ex(node, key, &obj) == false) {
        fprintf(stderr, "Unable to parse json bool with key: %s\n", key);
        return E_JSON_PARSE;
    }

    *value = json_object_get_boolean(obj);

    return E_SUCCESS;
}

int util_re_findall(char *pattern) {
    int len = 0;
    int ret = E_SUCCESS;
    PCRE2_SIZE error_offset;
    pcre2_code *re = NULL;

    if ((len = strlen(pattern)) == 0) {
        fprintf(stderr, "Regex pattern is empty\n");
        return E_EMPTY;
    }

    re = pcre2_compile((PCRE2_SPTR)pattern, len, 0, &ret, &error_offset, NULL);
    if (re == NULL) {
        // TODO implement printing PCRE2 error message
        fprintf(stderr, "Unable to compile regex expression: %d\n", ret);
    }

    pcre2_code_free(re);

    return ret;
}