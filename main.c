#include <cmph.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tests.h"

int main(int argc, char *argv[]) {

    bool showUsage = true;
    if (argc == 3 && strcmp(argv[1], "--test") == 0)
        showUsage = run_tests(argc, argv) == 1;

    if (showUsage) {
        printf("Usage:\n\t%s --test [test]\n\n", argv[0]);
        print_tests();
    }

    return 0;
}
