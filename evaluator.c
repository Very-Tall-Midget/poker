#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "evaluator.h"

evaluator_t *evaluator_load(const char *fileName) {
    evaluator_t *evaluator = calloc(1, sizeof(evaluator_t));
    if (!evaluator) {
        fprintf(stderr, "Failed to allocate evaluator\n");
        return 0;
    }

    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s\n", fileName);
        free(evaluator);
        return 0;
    }

    uint64_t objects = fread(
        evaluator->handRanks, sizeof(evaluator->handRanks[0]),
        sizeof(evaluator->handRanks) / sizeof(evaluator->handRanks[0]), fp);
    assert(objects ==
           sizeof(evaluator->handRanks) / sizeof(evaluator->handRanks[0]));
    fclose(fp);

    return evaluator;
}

void evaluator_destroy(evaluator_t *evaluator) { free(evaluator); }

handrank_t evaluator_evaluate(evaluator_t *evaluator, card_t *cards,
                              size_t nCards) {
    assert(nCards == 5 || nCards == 6 || nCards == 7);

    handrank_t rank = 53;
    for (int i = 0; i < nCards; ++i) {
        rank = evaluator->handRanks[rank + cards[i]];
    }

    if (nCards != 7)
        rank = evaluator->handRanks[rank];

    return rank;
}

const char *handRanksStr[] = {
    "impossible",      "High Card",     "One Pair", "Two Pair",
    "Three of a Kind", "Straight",      "Flush",    "Full House",
    "Four of a Kind",  "Straight Flush"};

const char *cardRanks[] = {"Deuce", "Three", "Four", "Five", "Six",
                           "Seven", "Eight", "Nine", "Ten",  "Jack",
                           "Queen", "King",  "Ace"};
const char *cardRanksPlural[] = {"Deuces", "Threes", "Fours", "Fives", "Sixes",
                                 "Sevens", "Eights", "Nines", "Tens",  "Jacks",
                                 "Queens", "Kings",  "Aces"};

char *handrank_to_str(handrank_t handRank) {
    char *out = calloc(100, sizeof(char));
    const char *rankName = handRanksStr[handRank >> 12];
    sprintf(out, "%s, ", rankName);

    handrank_t lower = handRank & 0xFFF;
    switch (handRank >> 12) {
    // Straight and straight flush
    case 5:
    case 9: {
        rank_t high = Five + (lower - 1);
        if (high == Ace)
            sprintf(out, "Royal Flush");
        else {
            if (high == Five)
                sprintf(out + strlen(out), "Ace to Five");
            else
                sprintf(out + strlen(out), "%s to %s", cardRanks[high - 4],
                        cardRanks[high]);
        }
    } break;
    // Four of a kind and full house
    case 7:
    case 8: {
        rank_t quad = Two, kicker = Three;
        for (handrank_t i = 1; i != lower; ++i) {
            kicker++;
            if (quad == kicker)
                kicker++;
            if (kicker > Ace) {
                kicker = Two;
                quad++;
            }
        }

        if (handRank >> 12 == 8)
            sprintf(out + strlen(out), "%s", cardRanksPlural[quad]);
        else
            sprintf(out + strlen(out), "%s full of %s", cardRanksPlural[quad],
                    cardRanksPlural[kicker]);
    } break;
    // Trips
    case 4: {
        rank_t trip = lower / (11 * (11 + 1) / 2);
        sprintf(out + strlen(out), "%s", cardRanksPlural[trip]);
    } break;
    // TODO: One pair, high card, two pair, flush
    // Default just remove the comma
    default:
        out[strlen(out) - 2] = 0;
        break;
    }

    return out;
}
