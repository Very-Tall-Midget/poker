#ifndef CARD_H
#define CARD_H

#include <stdint.h>

extern const uint32_t rankPrimes[13];

/* Format:
 *
 * xxxbbbbb bbbbbbbb cdhsrrrr xxpppppp
 *
 * p = prime number of rank
 *
 * r = rank of card (two=0, three=1, ...)
 *
 * cdhs = bit for suit
 *
 * b = bit for rank */
typedef uint32_t card_t;

typedef enum Suit {
    Spades,
    Hearts,
    Diamonds,
    Clubs,
} suit_t;

typedef enum Rank {
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Nine,
    Ten,
    Jack,
    Queen,
    King,
    Ace,
} rank_t;

card_t create_card(rank_t rank, suit_t suit);
card_t card_from_idx(uint32_t idx);
void card_two_from_idx(uint32_t idx, card_t *cards);

#endif // CARD_H
