#ifndef HANDRANGE_H
#define HANDRANGE_H

#include <stddef.h>

#include "card.h"

typedef struct HandRange {
    card_t *cards;
    size_t size;
} handrange_t;

handrange_t *handrange_create(char *range);
const card_t *handrange_get(handrange_t *handRange, size_t idx);
void handrange_destroy(handrange_t *handRange);

#endif // HANDRANGE_H
