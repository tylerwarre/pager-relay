#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "util.h"
#include "error.h"

const char *RE_PATTERN_UTF8 = "[\\x00-\\x7F]|[\\xC2-\\xDF][\\x80-\\xBF]|\\xE0[\\xA0-\\xBF][\\x80-\\xBF]|[\\xE1-\\xEC\\xEE\\xEF][\\x80-\\xBF]{2}|\\xED[\\x80-\\x9F][\\x80-\\xBF]|\\xF0[\\x90-\\xBF][\\x80-\\xBF]{2}|[\\xF1-\\xF3][\\x80-\\xBF]{3}|\\xF4[\\x80-\\x8F][\\x80-\\xBF]{2}";
// Smaple emoji regex (\ud83c[\ud000-\udfff]|\ud83d[\ud000-\udfff]|\ud83e[\ud000-\udfff])
const char *RE_PATTERN_EMOJI = "(\\xF0[\\x90-\\xBF][\\x80-\\xBF][\\x80-\\xBF])";
// This is technically only the printable us-ascii characterset
const char *RE_PATTERN_INVALID_USASCII = "([^\\x20-\\x7F]+)";

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

// TODO: Debug why this causes a memory leak
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

int util_json_get_str(struct json_object *node, char *key, char **dest, bool allocate) {
    struct json_object *obj = NULL;

    if(json_object_object_get_ex(node, key, &obj) == false) {
        fprintf(stderr, "Unable to parse json string with key: %s\n", key);
        return E_JSON_PARSE;
    }

    if (allocate) {
        const char *str = NULL;
        int len = 0;

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
    }
    else {
        if((*dest = (char *)json_object_get_string(obj)) == NULL) {
            fprintf(stderr, "Unable to access json string with key: %s\n", key);
            return E_JSON_ACCESS;
        }
    }

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

int util_re_substitute(const char *pattern, char *subj, char c, uint32_t opt) {
    int len = 0;
    int ret = E_SUCCESS;
    PCRE2_UCHAR err_msg[RE_ERR_LEN];
    pcre2_code *re = NULL;
    pcre2_match_data *match = NULL;

    if ((re = util_re_compile(pattern, subj)) == NULL) {
        return E_RE_COMPILE;
    }

    while (true) {
        if ((len = strlen(subj)) == 0)
        {
            fprintf(stderr, "Subject is empty\n");
            ret = E_EMPTY;
            break;
        }

        if ((match = pcre2_match_data_create_from_pattern(re, NULL)) == NULL)
        {
            fprintf(stderr, "Error allocating memory for regex matches: %d\n", ret);
            ret = E_OUTOFMEMORY;
            break;
        }

        // If there is an error when matching. >1 indicates a match
        if ((ret = pcre2_match(re, (PCRE2_SPTR)subj, len, opt, 0, match, NULL)) < 1)
        {
            if (pcre2_get_error_message(ret, err_msg, 120) < 0)
            {
                fprintf(stderr, "Error performing regex match: %d\n", ret);
            }
            else
            {
                fprintf(stderr, "Error performing regex match: (%d) %s\n", ret, err_msg);
            }
            break;
        }

        char *m = calloc(100, 1);

        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match);

        PCRE2_SIZE start = ovector[0];
        PCRE2_SIZE end = ovector[1];

        memcpy(m, subj + start, (end - start));

        printf("Match: %s\n", m);
        free(m);

        ret = E_SUCCESS;
        break;
    }

    pcre2_code_free(re);
    pcre2_match_data_free(match);

    return ret;
}

static pcre2_code* util_re_compile(const char *pattern, char *subj) {
    int len = 0;
    int ret = E_SUCCESS;
    PCRE2_UCHAR err_msg[RE_ERR_LEN];
    PCRE2_SIZE error_offset = 0;
    pcre2_code *re = NULL;

    if ((len = strlen(pattern)) == 0) {
        fprintf(stderr, "Regex pattern is empty\n");
        return NULL;
    }

    re = pcre2_compile((PCRE2_SPTR)pattern, len, 0, &ret, &error_offset, NULL);
    if (re == NULL) {
        if (pcre2_get_error_message(ret, err_msg, RE_ERR_LEN) < 0) {
            fprintf(stderr, "Unable to compile regex expression: %d\n", ret);
        }
        else {
            fprintf(stderr, "Unable to compile regex expression: (%d) %s\n", ret, err_msg);
        }
        return NULL;
    }

    return re;
}
