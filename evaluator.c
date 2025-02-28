#include <string.h>

#include "evaluator.h"
#include "tables.h"

/*#define FAST_EVAL*/
#ifdef FAST_EVAL
#include "fasttt.h"
#endif

const size_t perm7[21][5] = {
    {0, 1, 2, 3, 4}, {0, 1, 2, 3, 5}, {0, 1, 2, 3, 6}, {0, 1, 2, 4, 5},
    {0, 1, 2, 4, 6}, {0, 1, 2, 5, 6}, {0, 1, 3, 4, 5}, {0, 1, 3, 4, 6},
    {0, 1, 3, 5, 6}, {0, 1, 4, 5, 6}, {0, 2, 3, 4, 5}, {0, 2, 3, 4, 6},
    {0, 2, 3, 5, 6}, {0, 2, 4, 5, 6}, {0, 3, 4, 5, 6}, {1, 2, 3, 4, 5},
    {1, 2, 3, 4, 6}, {1, 2, 3, 5, 6}, {1, 2, 4, 5, 6}, {1, 3, 4, 5, 6},
    {2, 3, 4, 5, 6},
};

evaluator_t *evaluator_load(const char *fileName) {
    evaluator_t *evaluator = malloc(sizeof(evaluator_t));
    if (!evaluator) {
        fprintf(stderr, "Failed to allocate evaluator\n");
        return 0;
    }

    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to open file %s\n", fileName);
        return 0;
    }

    evaluator->mphf = cmph_load(fp);
    fclose(fp);

    return evaluator;
}

void evaluator_destroy(evaluator_t *evaluator) {
    cmph_destroy(evaluator->mphf);
    free(evaluator);
}

uint32_t evaluator_lookup(evaluator_t *evaluator, uint32_t key) {
    return cmph_search(evaluator->mphf, (char *)&key, sizeof(uint32_t));
}

int32_t binary_search(const primeinfo_t *arr, int32_t low, int32_t high,
                      uint32_t q) {
    while (low <= high) {
        int32_t mid = low + (high - low) / 2;

        if (arr[mid].q == q) {
            return mid;
        }

        if (arr[mid].q < q)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return -1;
}

uint16_t evaluator_evaluate(evaluator_t *evaluator, card_t cards[5]) {
    uint32_t flush = 0xF000, q = 0;
    for (int i = 0; i < 5; ++i) {
        flush &= cards[i];
        q |= cards[i];
    }
    q >>= 16;

    uint16_t eval;
    if (flush)
        eval = flushTable[q];
    else
        eval = uniquesTable[q];

    if (eval == 0) {
        uint32_t q = 1;
        for (int i = 0; i < 5; ++i)
            q *= cards[i] & 0xFF;
#ifdef FAST_EVAL
        return fasttt_eval(q);
#else
#ifdef MPHF_EVAL
        uint32_t idx = evaluator_lookup(evaluator, q);
        return hashTable[idx];
#else
        int32_t idx = binary_search(primes, 0, 4888, q);
        if (idx == -1)
            return 0;
        return primes[idx].rank;
#endif
#endif
    }

    return eval;
}

uint16_t evaluator_evaluate_with_community(evaluator_t *evaluator,
                                           card_t hand[2],
                                           card_t community[5]) {
    card_t cards[7] = {0};
    memcpy(cards, hand, sizeof(card_t) * 2);
    memcpy(cards + 2, community, sizeof(card_t) * 5);
    return evaluator_evaluate7(evaluator, cards);
}

uint16_t evaluator_evaluate7(evaluator_t *evaluator, card_t cards[7]) {
    uint16_t best = 0;
    card_t card5[5] = {0};
    for (int i = 0; i < 21; ++i) {
        for (int j = 0; j < 5; ++j)
            card5[j] = cards[perm7[i][j]];
        uint16_t eval = evaluator_evaluate(evaluator, card5);
        if (best == 0 || eval < best)
            best = eval;
    }
    return best;
}
