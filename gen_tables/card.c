#include <stdint.h>

#include "card.h"

card_t create_card(rank_t rank, suit_t suit) {
    uint32_t prime = rankPrimes[rank];
    uint32_t rankBit = 1 << rank;
    uint32_t rankNum = (uint32_t)rank;
    uint32_t suitBit = 1 << (uint32_t)suit;

    return (prime & 0b111111) | ((rankNum & 0b1111) << 8) | (suitBit << 12) |
           (rankBit << 16);
}

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
