#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <stddef.h>

#include "card.h"

typedef struct Evaluator {
    int32_t handRanks[32487834];
} evaluator_t;

typedef int32_t handrank_t;

evaluator_t *evaluator_load(const char *fileName);
void evaluator_destroy(evaluator_t *evaluator);
handrank_t evaluator_evaluate(evaluator_t *evaluator, card_t *cards,
                              size_t nCards);

const char *handrank_to_str(handrank_t handRank);

#endif // EVALUATOR_H
