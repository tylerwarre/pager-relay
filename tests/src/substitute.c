#include <dirent.h>

#include <stdio.h>
#include <string.h>

#include "substitute.h"

static char* read_file(char *msg_name) {
    char *path = NULL;
    char *msg = NULL;
    FILE *fd = NULL;
    int len_path = 0;
    int len_file = 0;

    len_path = strlen(MSGS_PATH) + strlen(msg_name);
    if ((path = calloc(len_path + 1, sizeof(char))) == NULL) {
        return false;
    }

    while (true) {
        strncpy(path, MSGS_PATH, strlen(MSGS_PATH));
        strncat(path, msg_name, strlen(msg_name));

        if ((fd = fopen(path, "r")) == NULL) {
            break;
        }

        // Seek to the end of the file
        if (fseek(fd, 0, SEEK_END) == -1) {
            break;
        }

        // Get the offset (e.g. length) from the start of the file
        if ((len_file = ftell(fd)) < 1) {
            break;
        }

        // Seek to the start of the file
        if (fseek(fd, 0, SEEK_SET) == -1) {
            break;
        }

        if ((msg = calloc(len_file, sizeof(char))) == NULL) {
            break;
        }

        if (fread(msg, sizeof(char), len_file, fd) != len_file) {
            free(msg);
            msg = NULL;
            break;
        }

        break;
    }

    if (path != NULL) {
        free(path);
        path = NULL;
    }

    if (fd != NULL) {
        fclose(fd);
        fd = NULL;
    }

    return msg;
}

static bool test_case(char *msg_name) {
    char *msg = NULL;
    bool result = true;

    if ((msg = read_file(msg_name)) != NULL) {

        if (util_re_substitute((const char*)"[^\\x20-\\x7F]+", &msg, '^', 0) != PCRE2_ERROR_NOMATCH) {
            result = false;
        }

        // TODO use a lookup table to compare expected results

        if (msg != NULL) {
            free(msg);
            msg = NULL;
        }
    }
    else {
        result = false;
    }

    test_print_result(msg_name, result);

    return result;
}

bool substitute() {
    int pass = 0;
    int fail = 0;

    test_print_start(__func__);

    DIR *d = NULL;
    struct dirent *dir;
    // Requires the test message directory to be in the same folder as the
    //  executable
    d = opendir("./msgs");
    // Ensure the directory exits
    if (d) {
        // Find all files in the directory
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type != DT_REG) {
                continue;
            }

            if (test_case(dir->d_name)) {
                pass++;
            }
            else {
                fail++;
            }
        }
        closedir(d);
    }

    return test_print_summary(__func__, pass, fail);
}
