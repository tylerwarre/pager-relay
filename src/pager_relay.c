#include <json-c/json.h>
#include <curl/curl.h>

#include <stdio.h>

int main() {
    CURL *curl;
    struct json_object *obj = json_object_new_object();
    printf("%p\n", obj);

    if ((curl = curl_easy_init()) == NULL) {
        fprintf(stderr, "Unable to initalize curl\n");
        return 1;
    }

    return 0;
}