#define _POSIX_C_SOURCE 199309L
#include <cmph.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "card.h"
#include "equity.h"
#include "evaluator.h"

#define test_assert(expr)                                                      \
    if (!(expr))                                                               \
        return 1;

int test_flush(evaluator_t *evaluator) {
    test_assert(
        evaluator_evaluate(
            evaluator,
            (card_t[]){create_card(Ace, Spades), create_card(Two, Spades),
                       create_card(Three, Spades), create_card(Four, Spades),
                       create_card(Five, Spades), create_card(Eight, Hearts),
                       create_card(Nine, Diamonds)},
            7) == 36865);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Spades),
                                              create_card(King, Spades),
                                              create_card(Ten, Spades),
                                              create_card(Queen, Spades),
                                              create_card(Jack, Spades)},
                                   5) == 36874);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Spades),
                                              create_card(King, Spades),
                                              create_card(Nine, Spades),
                                              create_card(Queen, Spades),
                                              create_card(Jack, Spades)},
                                   5) == 25853);

    return 0;
}

int test_unique(evaluator_t *evaluator) {
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ten, Hearts),
                                              create_card(King, Spades),
                                              create_card(Nine, Clubs),
                                              create_card(Queen, Spades),
                                              create_card(Jack, Diamonds)},
                                   5) == 20489);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Two, Hearts),
                                              create_card(Three, Spades),
                                              create_card(Seven, Clubs),
                                              create_card(Four, Spades),
                                              create_card(Five, Diamonds)},
                                   5) == 4097);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Two, Hearts),
                                              create_card(Three, Diamonds),
                                              create_card(Five, Clubs),
                                              create_card(Four, Clubs),
                                              create_card(Eight, Spades)},
                                   5) == 4101);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Hearts),
                                              create_card(Nine, Diamonds),
                                              create_card(King, Clubs),
                                              create_card(Queen, Clubs),
                                              create_card(Jack, Spades)},
                                   5) == 5373);

    return 0;
}

int test_primes(evaluator_t *evaluator) {
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Ace, Diamonds),
                                              create_card(Ace, Hearts),
                                              create_card(Ace, Clubs),
                                              create_card(Ace, Spades),
                                              create_card(King, Hearts)},
                                   5) == 32924);
    test_assert(evaluator_evaluate(evaluator,
                                   (card_t[]){create_card(Two, Diamonds),
                                              create_card(Five, Hearts),
                                              create_card(Two, Clubs),
                                              create_card(Four, Spades),
                                              create_card(Three, Hearts)},
                                   5) == 8193);

    return 0;
}

int test_all_5card(evaluator_t *evaluator) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint32_t count = 0, invalid = 0;
    for (uint32_t n = 0; n < 50; ++n)
        for (uint32_t i = 0; i < 48; ++i)
            for (uint32_t j = i + 1; j < 49; ++j)
                for (uint32_t k = j + 1; k < 50; ++k)
                    for (uint32_t l = k + 1; l < 51; ++l)
                        for (uint32_t m = l + 1; m < 52; ++m)
                            if (evaluator_evaluate(
                                    evaluator,
                                    (card_t[]){create_card(i / 4, i % 4),
                                               create_card(j / 4, j % 4),
                                               create_card(k / 4, k % 4),
                                               create_card(l / 4, l % 4),
                                               create_card(m / 4, m % 4)},
                                    5))
                                count += 1;
                            else
                                invalid += 1;

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    test_assert(invalid == 0);
    printf("Evaluated %d 5 card hands in %.2fs (%.2f hands per second)\n",
           count, time, (double)count / time);

    return 0;
}

int test_equity(evaluator_t *evaluator) {
    card_t cards[8] = {create_card(Ace, Spades), create_card(Ace, Clubs),
                       create_card(King, Spades), create_card(King, Clubs)};
    card_t community[4] = {create_card(Queen, Hearts), create_card(Two, Spades),
                           create_card(Ace, Diamonds),
                           create_card(King, Hearts)};
    printf("Hand 1: As Ac\nHand 2: Ks Kc\n");
    {
        equityinfo_t *equity = equity_calc(evaluator, cards, 2, NULL, 0);

        printf("Evaluated %u 7 card hands in %.2fs\n", equity->total,
               equity->time);
        for (int i = 0; i < 2; ++i) {
            printf("Hand %d: win %.2f%%, chop %.2f%%\n", i + 1,
                   equity->equities[i].win * 100,
                   equity->equities[i].chop * 100);
        }

        test_assert(equity->equities[0].win < 0.83 &&
                    equity->equities[0].win > 0.82);

        equity_destroy(equity);
    }
    printf("Flop: Qh 2s Ad\n");
    {
        equityinfo_t *equity = equity_calc(evaluator, cards, 2, community, 3);

        printf("Evaluated %u 7 card hands in %.2fs\n", equity->total,
               equity->time);
        for (int i = 0; i < 2; ++i) {
            printf("Hand %d: win %.2f%%, chop %.2f%%\n", i + 1,
                   equity->equities[i].win * 100,
                   equity->equities[i].chop * 100);
        }

        test_assert(equity->equities[0].win > 0.98);

        equity_destroy(equity);
    }
    printf("Turn: Kh\n");
    {
        equityinfo_t *equity = equity_calc(evaluator, cards, 2, community, 4);

        printf("Evaluated %u 7 card hands in %.2fs\n", equity->total,
               equity->time);
        for (int i = 0; i < 2; ++i) {
            printf("Hand %d: win %.2f%%, chop %.2f%%\n", i + 1,
                   equity->equities[i].win * 100,
                   equity->equities[i].chop * 100);
        }

        test_assert(equity->equities[1].win > 0.022 &&
                    equity->equities[1].win < 0.023);

        equity_destroy(equity);
    }

    return 0;
}

const struct Test {
    const char *name;
    int (*func)(evaluator_t *);
} tests[] = {{"flush", test_flush},
             {"unique", test_unique},
             {"primes", test_primes},
             {"all_5card", test_all_5card},
             {"equity", test_equity}};
const size_t nTests = sizeof(tests) / sizeof(struct Test);

int main(int argc, char *argv[]) {
    evaluator_t *evaluator = evaluator_load("handranks.dat");
    if (!evaluator)
        return 1;

    bool showUsage = true;
    if (argc == 3 && strcmp(argv[1], "--test") == 0) {
        int failed = 0, success = 0;
        for (int i = 0; i < nTests; ++i) {
            if (strcmp(argv[2], tests[i].name) == 0 ||
                strcmp(argv[2], "all") == 0) {
                showUsage = false;
                printf("Running test %s...\n", tests[i].name);
                if (tests[i].func(evaluator)) {
                    printf("Test %s failed\n", tests[i].name);
                    failed++;
                } else {
                    printf("Test %s succeeded\n", tests[i].name);
                    success++;
                }
            }
        }
        if (!showUsage)
            printf("\n%d tests succeeded, %d tests failed\n", success, failed);
    }

    if (showUsage) {
        printf("Usage:\n\t%s --test [test]\n\n", argv[0]);
        printf("Available tests:\n\tall\n");
        for (int i = 0; i < nTests; ++i)
            printf("\t%s\n", tests[i].name);
    }

    evaluator_destroy(evaluator);

    return 0;
}
