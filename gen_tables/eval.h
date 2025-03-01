#ifndef EVAL_H
#define EVAL_H

#include <stdint.h>

#include "card.h"

void eval_init();
int eval_test();
uint16_t eval5(card_t cards[5]);
uint16_t eval6(card_t cards[6]);
uint16_t eval7(card_t cards[7]);

#endif // EVAL_H
