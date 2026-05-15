#ifndef SUBSTITUTE_H
#define SUBSTITUTE_H

#include <stdbool.h>

#include "tests.h"
#include "util.h"

static const char *MSGS_PATH = "./msgs/";

bool substitute();
static bool test_case(char *msg_name);
static char* read_file(char *msg_name);

#endif
