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
    printf("Read %ld bytes from %s\n",
           objects * sizeof(evaluator->handRanks[0]), fileName);
    assert(objects ==
           sizeof(evaluator->handRanks) / sizeof(evaluator->handRanks[0]));
    fclose(fp);

    return evaluator;
}

void evaluator_destroy(evaluator_t *evaluator) { free(evaluator); }

int32_t card_to_idx(card_t card) {
    int32_t cardRank = (card >> 8) & 0xF;
    int32_t cardSuit =
        ((card >> 13) & 1) + ((card >> 14) & 1) * 2 + ((card >> 15) & 1) * 3;
    return cardSuit + cardRank * 4 + 1;
}

int32_t evaluator_evaluate(evaluator_t *evaluator, card_t *cards,
                           size_t nCards) {
    assert(nCards == 5 || nCards == 6 || nCards == 7);
    int32_t cardsConverted[7] = {0};
    for (int i = 0; i < nCards; ++i) {
        cardsConverted[i] = card_to_idx(cards[i]);
    }

    int32_t rank = 53;
    for (int i = 0; i < nCards; ++i) {
        rank = evaluator->handRanks[rank + cardsConverted[i]];
    }

    if (nCards != 7)
        rank = evaluator->handRanks[rank];

    return rank;
}
