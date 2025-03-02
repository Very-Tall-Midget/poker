#ifndef CARD_H
#define CARD_H

#include <stdint.h>

#define CARD_RANK(card) ((card - 1) / 4)
#define CARD_SUIT(card) ((card - 1) % 4)

/* Format:
 * 1-52 (inclusive)
 * 2s, 2h, 2d, 2c, ..., As, Ah, Ad, Ac
 */
typedef int32_t card_t;

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
void card_to_string(card_t card, char str[3]);
card_t card_from_str(const char *str);

#endif // CARD_H
