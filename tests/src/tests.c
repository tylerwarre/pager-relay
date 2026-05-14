#include <stdio.h>
#include <string.h>

#include "tests.h"
#include "substitute.h"

void test_print_result(char *test_name, bool result) {
    if (result) {
        printf("\t[+] %s: Pass\n", test_name);
    }
    else {
        printf("\t[-] %s: Fail\n", test_name);
    }

    return;
}

bool test_print_summary(const char *test_group, int pass, int fail) {
    if (fail == 0 && pass > 0) {
        printf("[+] %s: %d/%d Passed\n", test_group, pass, pass+fail);
        return true;
    }
    else {
        printf("[-] %s: %d/%d Passed\n", test_group, pass, pass+fail);
        return false;
    }
}

void test_print_start(const char *test_group) {
    printf("Initalizing tests: %s\n", test_group);

    return;
}

int main() {
    substitute();
}