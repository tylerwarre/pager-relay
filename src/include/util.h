#ifndef UTIL_H
#define UTIL_H

#include <json-c/json.h>
#include <pcre2.h>

#define RE_ERR_LEN 120

extern const char *RE_PATTERN_UTF8;
// This is technically only the printable us-ascii characterset
extern const char *RE_PATTERN_INVALID_USASCII;
extern const char *RE_PATTERN_EMOJI;

void util_detach_json_child(struct json_object *parent, char *key, struct json_object *child);
void util_detach_json_child_idx(struct json_object *parent, int child_index, struct json_object *child);
int util_print_json_str(struct json_object *parent, char *key);
int util_json_get_str(struct json_object *node, char *key, char **dest, bool allocate);
int util_json_get_bool(struct json_object *node, char *key, bool *value);
int util_re_substitute(const char *pattern, char *subj, char c, uint32_t opt);
static pcre2_code* util_re_compile(const char *pattern, char *subj);

#endif