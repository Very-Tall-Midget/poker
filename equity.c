#define _POSIX_C_SOURCE 199309L
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "card.h"
#include "equity.h"

uint32_t choose(uint32_t n, uint32_t k) {
    if (k == 0)
        return 1;
    else
        return (n * choose(n - 1, k - 1)) / k;
}

equityinfo_t *equity_calc(evaluator_t *evaluator, card_t *hands, size_t nHands,
                          card_t *community, size_t nCommunity) {
    assert(nCommunity == 0 || nCommunity == 3 || nCommunity == 4);

    handequity_t *equities = calloc(nHands, sizeof(handequity_t));

    uint32_t n = 5 - nCommunity;
    size_t nUsedCards = nHands * 2 + nCommunity;
    card_t *usedCards = calloc(nUsedCards, sizeof(card_t));
    memcpy(usedCards, hands, sizeof(card_t) * nHands * 2);
    if (community)
        memcpy(usedCards + (nHands * 2), community,
               sizeof(card_t) * nCommunity);

    uint32_t *wins = calloc(nHands, sizeof(uint32_t)),
             *chops = calloc(nHands, sizeof(uint32_t));
    uint32_t total = 0;
    card_t newCommunity[5] = {0};
    uint16_t *handRanks = calloc(nHands, sizeof(uint16_t));
    card_t *card7Eval = malloc(7 * sizeof(card_t));

    if (community && nCommunity > 0)
        memcpy(newCommunity, community, nCommunity * sizeof(card_t));

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint32_t combinations = choose(52, n);
    for (uint32_t idx = 0; idx < combinations; ++idx) {
        switch (n) {
        case 1: {
            newCommunity[4] = card_from_idx(idx);

            bool duplicate = false;
            for (int i = 0; i < nUsedCards; ++i) {
                if (usedCards[i] == newCommunity[4]) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate)
                continue;
        } break;
        case 2: {
            card_two_from_idx(idx, newCommunity + 3);

            bool duplicate = false;
            for (int i = 0; i < nUsedCards; ++i) {
                if (usedCards[i] == newCommunity[4] ||
                    usedCards[i] == newCommunity[3]) {
                    duplicate = true;
                    break;
                }
            }

            if (duplicate)
                continue;
        } break;
        case 5: {
            uint32_t cards[5] = {0}, nextCardIdx = 0, next = 0, idxCopy = idx;

            for (uint32_t k = 0; k < 5; ++k) {
                for (uint32_t i = next; i < 52; ++i) {
                    bool duplicate = false;
                    for (uint32_t j = 0; j < 5; ++j)
                        if (cards[j] == i) {
                            duplicate = true;
                            break;
                        }
                    if (duplicate)
                        continue;

                    uint32_t count = choose(52 - i - 1, 4 - k);
                    if (idxCopy < count) {
                        cards[nextCardIdx++] = i;
                        next = i + 1;
                        break;
                    }
                    idxCopy -= count;
                }
            }

            bool duplicate = false;
            for (int i = 0; i < 5; ++i) {
                newCommunity[i] = card_from_idx(cards[i]);

                for (int j = 0; j < nUsedCards; ++j)
                    if (usedCards[j] == newCommunity[i]) {
                        duplicate = true;
                        break;
                    }

                if (duplicate)
                    break;
            }

            if (duplicate)
                continue;
        } break;
        default:
            __builtin_unreachable();
        }

        for (int i = 0; i < nHands; ++i) {
            memcpy(card7Eval, hands + (i * 2), 2 * sizeof(card_t));
            memcpy(card7Eval + 2, newCommunity, sizeof(newCommunity));
            handRanks[i] = evaluator_evaluate(evaluator, card7Eval, 7);
        }

        uint16_t best = handRanks[0];
        int winners = 1;

        for (int i = 1; i < nHands; ++i) {
            if (handRanks[i] > best) {
                best = handRanks[i];
                winners = 1;
            } else if (handRanks[i] == best)
                winners++;
        }

        for (int i = 0; i < nHands; ++i) {
            if (handRanks[i] == best) {
                if (winners > 1)
                    chops[i]++;
                else
                    wins[i]++;
            }
        }

        ++total;
    }

    for (int i = 0; i < nHands; ++i) {
        equities[i] =
            (handequity_t){(float)wins[i] / (float)total,
                           (float)chops[i] / (float)total, wins[i], chops[i]};
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time =
        (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    free(wins);
    free(chops);
    free(handRanks);
    free(usedCards);
    free(card7Eval);

    equityinfo_t *equity = malloc(sizeof(equityinfo_t));
    equity->total = total;
    equity->equities = equities;
    equity->time = time;

    return equity;
}

void equity_destroy(equityinfo_t *equity) {
    free(equity->equities);
    free(equity);
}
