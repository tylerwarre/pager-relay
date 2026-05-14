#ifndef TESTS_H
#define TESTS_H

#include <stdbool.h>

void test_print_result(char *test_name, bool result);
void test_print_start(const char *test_group);
bool test_print_summary(const char *test_group, int pass, int fail);

#endif