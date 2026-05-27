#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "util.h"
#include "error.h"

// This is technically only the printable us-ascii characterset
const char *RE_PATTERN_INVALID_USASCII = "[^\\x20-\\x7F]+";

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
        (*dest)[len] = '\0';
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

int util_json_get_int(struct json_object *node, char *key, int *value) {
    struct json_object *obj = NULL;
    if(json_object_object_get_ex(node, key, &obj) == false) {
        fprintf(stderr, "Unable to parse json int with key: %s\n", key);
        return E_JSON_PARSE;
    }

    *value = json_object_get_int(obj);

    if (*value == 0 || *value == INT_MAX || *value == INT_MIN) {
        fprintf(stderr, "Invlaid integer being parsed: %s\n", key);
        return E_JSON_PARSE;
    }

    return E_SUCCESS;
}

// TODO: Convert to using custom linked list
int util_json_get_array(struct json_object *node, char *key, struct curl_slist **dest) {
    int len = 0;
    int str_len = 0;
    struct json_object *obj = NULL;
    const char *str = NULL;

    if(json_object_object_get_ex(node, key, &node) == false) {
        fprintf(stderr, "Unable to parse json array with key: %s\n", key);
        return E_JSON_PARSE;
    }

    if((len = json_object_array_length(node)) < 1) {
        fprintf(stderr, "[%s] array is empty for key %s\n", key, __func__);
        return E_EMPTY;
    }

    for (int i = 0; i < len; i++) {
        if ((obj = json_object_array_get_idx(node, i)) == NULL) {
            fprintf(stderr, "[%s] Unable to get obj at index: %d\n", __func__, i);
            return E_JSON_PARSE;
        }

        if((str = json_object_get_string(obj)) == NULL) {
            fprintf(stderr, "Unable to access json string with key: %s\n", key);
            return E_JSON_ACCESS;
        }
        str_len = strlen(str);

        *dest = curl_slist_append(*dest, str);
    }

    return E_SUCCESS;
}

int util_re_substitute(const char *pattern, char **subj, char c, uint32_t opt) {
    int len = 0;
    int ret = E_SUCCESS;
    PCRE2_SIZE offset = 0;
    PCRE2_UCHAR err_msg[RE_ERR_LEN];
    PCRE2_SIZE *ovector = NULL;
    PCRE2_SIZE end = 0;
    pcre2_code *re = NULL;
    pcre2_match_data *match = NULL;
    char *m = NULL;

    if ((re = util_re_compile(pattern, *subj, opt)) == NULL) {
        return E_RE_COMPILE;
    }

    if ((match = pcre2_match_data_create_from_pattern(re, NULL)) == NULL)
    {
        fprintf(stderr, "Error allocating memory for regex matches: %d\n", ret);
        ret = E_OUTOFMEMORY;
    }

    while (match != NULL) {
        if ((len = strlen(*subj+offset)) == 0) {
            ret = PCRE2_ERROR_NOMATCH;
            break;
        }

        // Only throw an error if there is actually an error. Proceed even if there is no match
        ret = pcre2_match(re, (PCRE2_SPTR)*subj+offset, len, opt, 0, match, NULL);
        if (ret == PCRE2_ERROR_NOMATCH)
        {
            break;
        }
        else if (ret < 1)
        {
            if (pcre2_get_error_message(ret, err_msg, RE_ERR_LEN) < 0)
            {
                fprintf(stderr, "Error performing regex match: %d\n", ret);
            }
            else
            {
                fprintf(stderr, "Error performing regex match: (%d) %s\n", ret, err_msg);
            }
            break;
        }

        ovector = pcre2_get_ovector_pointer(match);
        util_re_sub_match(*subj+offset, ovector, &offset, c);


        end = ovector[1];
        offset += end+1;
    }

    len = strlen(*subj);
    *subj = realloc(*subj, len+1);

    pcre2_code_free(re);
    pcre2_match_data_free(match);

    return ret;
}

void util_re_sub_match(char *ptr, PCRE2_SIZE *ovector, PCRE2_SIZE *offset, char c) {
    int len = 0;
    char *m = NULL;
    PCRE2_SIZE start = 0;
    PCRE2_SIZE end = 0;

    if ((len = strlen(ptr)) == 0) {
        return;
    }

    start = ovector[0];
    end = ovector[1];

    ptr[start] = c;
    // length of shortened string is found with (len+1)-((end-start)+start).
    // This is computed by first getting the passed in string length and adding
    // 1 to include the null terminator. Then we subtract the length of the match
    // add the start offset
    memmove(&ptr[start+1], &ptr[end], (len+1)-((end-start)+start));

    *offset -= (end-start);
    return;
}

pcre2_code* util_re_compile(const char *pattern, char *subj, uint32_t opt) {
    int ret = E_SUCCESS;
    PCRE2_UCHAR err_msg[RE_ERR_LEN];
    PCRE2_SIZE error_offset = 0;
    pcre2_code *re = NULL;

    re = pcre2_compile((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED, opt, &ret, &error_offset, NULL);
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
