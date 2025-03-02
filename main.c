#include <cmph.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "handrange.h"
#include "tests.h"

int main(int argc, char *argv[]) {

    bool showUsage = true;
    if (argc == 3 && strcmp(argv[1], "--test") == 0)
        showUsage = run_tests(argc, argv) != 0;

    if (argc == 3 && strcmp(argv[1], "--range") == 0) {
        showUsage = false;
        handrange_t *handRange = handrange_create(argv[2]);
        if (!handRange) {
            printf("Invalid hand range\n");
            return 1;
        }

        for (int i = 0; i < handRange->size; ++i) {
            const card_t *cards = handrange_get(handRange, i);
            char card1[3], card2[3];
            card_to_string(cards[0], card1);
            card_to_string(cards[1], card2);
            printf("%s %s\n", card1, card2);
        }

        handrange_destroy(handRange);
    }

    if (showUsage) {
        printf("Usage:\n\t%s [--test [test]] | [--range <range>]\n\n", argv[0]);
        print_tests();
    }

    return 0;
}
