#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <cmph.h>

#include "card.h"

typedef struct Evaluator {
    cmph_t *mphf;
} evaluator_t;

evaluator_t *evaluator_load(const char *fileName);
void evaluator_destroy(evaluator_t *evaluator);
uint32_t evaluator_lookup(evaluator_t *evaluator, uint32_t key);
uint16_t evaluator_evaluate(evaluator_t *evaluator, card_t cards[5]);
uint16_t evaluator_evaluate_with_community(evaluator_t *evaluator,
                                           card_t hand[2], card_t community[5]);
uint16_t evaluator_evaluate7(evaluator_t *evaluator, card_t cards[7]);

#endif // EVALUATOR_H
