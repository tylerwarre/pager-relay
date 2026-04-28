#ifndef UTIL_H
#define UTIL_H

#include <json-c/json.h>
#include <pcre2.h>

const char RE_PATTERN_UTF8[] = "[\x00-\x7F]|[\xC2-\xDF][\x80-\xBF]|\xE0[\xA0-\xBF][\x80-\xBF]|[\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}|\xED[\x80-\x9F][\x80-\xBF]|\xF0[\x90-\xBF][\x80-\xBF]{2}|[\xF1-\xF3][\x80-\xBF]{3}|\xF4[\x80-\x8F][\x80-\xBF]{2}";
// This is technically only the printable us-ascii characterset
const char RE_PATTERN_INVALID_USASCII[] = "[^\x20-\x7F]";

void util_detach_json_child(struct json_object *parent, char *key, struct json_object *child);
void util_detach_json_child_idx(struct json_object *parent, int child_index, struct json_object *child);
int util_print_json_str(struct json_object *parent, char *key);
int util_json_get_str(struct json_object *node, char *key, char **dest, bool allocate);
int util_json_get_bool(struct json_object *node, char *key, bool *value);
int util_re_match(char *pattern, PCRE2_SPTR *subj, pcre2_match_data_8 **matches);

#endif