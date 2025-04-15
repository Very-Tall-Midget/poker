#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <stddef.h>

#include "card.h"

typedef int32_t handrank_t;

typedef struct Evaluator {
    handrank_t handRanks[32487834];
} evaluator_t;

evaluator_t *evaluator_load(const char *fileName);
void evaluator_destroy(evaluator_t *evaluator);
handrank_t evaluator_evaluate(evaluator_t *evaluator, card_t *cards,
                              size_t nCards);

char *handrank_to_str(handrank_t handRank);

#endif // EVALUATOR_H
