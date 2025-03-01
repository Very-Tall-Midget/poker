#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <stddef.h>

#include "card.h"

typedef struct Evaluator {
    int32_t handRanks[32487834];
} evaluator_t;

evaluator_t *evaluator_load(const char *fileName);
void evaluator_destroy(evaluator_t *evaluator);
int32_t evaluator_evaluate(evaluator_t *evaluator, card_t *cards,
                           size_t nCards);

#endif // EVALUATOR_H
