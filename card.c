#include <stdint.h>

#include "card.h"

card_t create_card(rank_t rank, suit_t suit) { return rank * 4 + suit + 1; }

card_t card_from_idx(uint32_t idx) { return create_card(idx / 4, idx % 4); }

void card_two_from_idx(uint32_t idx, card_t *cards) {
    uint32_t card1 = 0;
    for (uint32_t i = 0; i < 52; ++i) {
        uint32_t count = 52 - i - 1;
        if (idx < count) {
            card1 = i;
            break;
        }
        idx -= count;
    }

    cards[0] = card_from_idx(card1);
    cards[1] = card_from_idx(card1 + 1 + idx);
}
