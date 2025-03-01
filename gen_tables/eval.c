#include <assert.h>
#include <cmph.h>
#include <cmph_types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "eval.h"

#define COMPARE(a, b) (((a) > (b)) - ((a) < (b)))

typedef struct PrimeInfo {
    uint32_t q;
    uint16_t rank;
} primeinfo_t;

const uint32_t rankPrimes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
uint16_t flushTable[0x1F01] = {0}, uniquesTable[0x1F01] = {0};
primeinfo_t primes[4888] = {0};

const size_t perm6[6][5] = {
    {0, 1, 2, 3, 4}, {0, 1, 2, 3, 5}, {0, 1, 2, 4, 5},
    {0, 1, 3, 4, 5}, {0, 2, 3, 4, 5}, {1, 2, 3, 4, 5},
};

const size_t perm7[21][5] = {
    {0, 1, 2, 3, 4}, {0, 1, 2, 3, 5}, {0, 1, 2, 3, 6}, {0, 1, 2, 4, 5},
    {0, 1, 2, 4, 6}, {0, 1, 2, 5, 6}, {0, 1, 3, 4, 5}, {0, 1, 3, 4, 6},
    {0, 1, 3, 5, 6}, {0, 1, 4, 5, 6}, {0, 2, 3, 4, 5}, {0, 2, 3, 4, 6},
    {0, 2, 3, 5, 6}, {0, 2, 4, 5, 6}, {0, 3, 4, 5, 6}, {1, 2, 3, 4, 5},
    {1, 2, 3, 4, 6}, {1, 2, 3, 5, 6}, {1, 2, 4, 5, 6}, {1, 3, 4, 5, 6},
    {2, 3, 4, 5, 6},
};

bool is_wheel_straight(uint16_t n) { return n == 0b1000000001111; }

bool is_straight(uint16_t n) {
    if (is_wheel_straight(n))
        return true;

    int count = 0;
    for (int i = 0; i < 13; ++i)
        if (n & (1 << i)) {
            count++;
            if (count == 5)
                return true;
        } else
            count = 0;
    return false;
}

int unique_sort(const void *a, const void *b) {
    uint16_t handA = *(uint16_t *)a, handB = *(uint16_t *)b;
    bool aStraight = is_straight(handA);
    bool bStraight = is_straight(handB);

    if (aStraight && !bStraight)
        return -1;
    if (!aStraight && bStraight)
        return 1;
    if (aStraight && bStraight) {
        bool aWheel = is_wheel_straight(handA);
        bool bWheel = is_wheel_straight(handB);

        if (aWheel && !bWheel)
            return 1;
        if (!aWheel && bWheel)
            return -1;
    }

    for (int i = 12; i >= 0; --i) {
        uint16_t aBit = (handA >> i) & 1;
        uint16_t bBit = (handB >> i) & 1;
        if (aBit != bBit) {
            return bBit - aBit;
        }
    }
    return 0;
}

typedef struct GroupsAndAppearances {
    uint8_t appearances[13];
    int8_t quad, trip, pair1, pair2;
} groupsAndAppearances_t;

groupsAndAppearances_t get_groups_and_appearances(const uint8_t *cards) {
    groupsAndAppearances_t groupsAndAppearances = {{0}, -1, -1, -1, -1};

    for (int i = 0; i < 5; ++i)
        groupsAndAppearances.appearances[cards[i]]++;

    for (int i = 12; i >= 0; --i) {
        switch (groupsAndAppearances.appearances[i]) {
        case 4:
            groupsAndAppearances.quad = i;
            break;
        case 3:
            groupsAndAppearances.trip = i;
            break;
        case 2:
            if (groupsAndAppearances.pair1 == -1)
                groupsAndAppearances.pair1 = i;
            else
                groupsAndAppearances.pair2 = i;
            break;
        }
    }

    return groupsAndAppearances;
}

int compare_group_hands(const void *_a, const void *_b) {
    const uint8_t *a = (const uint8_t *)_a, *b = (const uint8_t *)_b;
    groupsAndAppearances_t aGroups = get_groups_and_appearances(a),
                           bGroups = get_groups_and_appearances(b);

    // Quads
    if (aGroups.quad != -1 && bGroups.quad != -1) {
        if (aGroups.quad == bGroups.quad) {
            for (int i = 12; i >= 0; --i)
                if (aGroups.appearances[i] != bGroups.appearances[i])
                    return -COMPARE(aGroups.appearances[i],
                                    bGroups.appearances[i]);
            return 0;
        }
        return -COMPARE(aGroups.quad, bGroups.quad);
    }
    if (aGroups.quad != -1)
        return -1;
    if (bGroups.quad != -1)
        return 1;

    // Full house
    if (aGroups.trip != -1 && aGroups.pair1 != -1 && bGroups.trip != -1 &&
        bGroups.pair1 != -1) {
        if (aGroups.trip == bGroups.trip)
            return -COMPARE(aGroups.pair1, bGroups.pair1);
        return -COMPARE(aGroups.trip, bGroups.trip);
    }
    if (aGroups.trip != -1 && aGroups.pair1 != -1)
        return -1;
    if (bGroups.trip != -1 && bGroups.pair1 != -1)
        return 1;

    // Trips
    if (aGroups.trip != -1 && bGroups.trip != -1) {
        if (aGroups.trip == bGroups.trip) {
            for (int i = 12; i >= 0; --i)
                if (aGroups.appearances[i] != bGroups.appearances[i])
                    return -COMPARE(aGroups.appearances[i],
                                    bGroups.appearances[i]);
            return 0;
        }

        return -COMPARE(aGroups.trip, bGroups.trip);
    }
    if (aGroups.trip != -1)
        return -1;
    if (bGroups.trip != -1)
        return 1;

    // Two pair
    if (aGroups.pair1 != -1 && aGroups.pair2 != -1 && bGroups.pair1 != -1 &&
        bGroups.pair2 != -1) {
        if (aGroups.pair1 != bGroups.pair1)
            return -COMPARE(aGroups.pair1, bGroups.pair1);
        if (aGroups.pair2 != bGroups.pair2)
            return -COMPARE(aGroups.pair2, bGroups.pair2);

        for (int i = 12; i >= 0; --i)
            if (aGroups.appearances[i] != bGroups.appearances[i])
                return -COMPARE(aGroups.appearances[i], bGroups.appearances[i]);
        return 0;
    }
    if (aGroups.pair1 != -1 && aGroups.pair2 != -1)
        return -1;
    if (bGroups.pair1 != -1 && bGroups.pair2 != -1)
        return 1;

    // Pair
    if (aGroups.pair1 != -1 && bGroups.pair1 != -1) {
        if (aGroups.pair1 == bGroups.pair1) {
            for (int i = 12; i >= 0; --i)
                if (aGroups.appearances[i] != bGroups.appearances[i])
                    return -COMPARE(aGroups.appearances[i],
                                    bGroups.appearances[i]);
            return 0;
        }
        return -COMPARE(aGroups.pair1, bGroups.pair1);
    }

    __builtin_unreachable();
}

int compare_prime_info(const void *a, const void *b) {
    const primeinfo_t *infoA = (const primeinfo_t *)a,
                      *infoB = (const primeinfo_t *)b;

    int val = (infoA->q > infoB->q) - (infoA->q < infoB->q);
    return val;
}

void eval_init() {
    uint16_t unique5Hands[1287] = {0};
    uint32_t idx = 0;
    for (uint16_t hand = 0b11111; hand < (1 << 13); ++hand) {
        int count = 0;
        for (int j = 0; j < 13; ++j)
            if (hand & (1 << j))
                count++;
        if (count == 5)
            unique5Hands[idx++] = hand;
    }

    qsort(unique5Hands, 1287, sizeof(uint16_t), unique_sort);

    for (uint16_t hand = 0; hand < 0x1F01; hand++) {
        for (uint16_t j = 0; j < 1287; ++j) {
            if (unique5Hands[j] == hand) {
                flushTable[hand] = (j > 9) ? (j + 156 * 2 + 1) : (j + 1);
                break;
            }
        }
    }

    for (uint16_t hand = 0; hand < 0x1F01; hand++) {
        for (uint16_t j = 0; j < 1287; ++j) {
            if (unique5Hands[j] == hand) {
                uniquesTable[hand] =
                    (j > 9) ? (j + 10 + 156 * 2 + 1277 + 858 * 2 + 2860 + 1)
                            : (j + 10 + 156 * 2 + 1277 + 1);
                break;
            }
        }
    }

    // Generate all possible hands that aren't straights or entirely unique
    uint8_t primesHands[4888 * 5] = {0};
    idx = 0;
    for (uint8_t i = 0; i < 13; ++i)
        for (uint8_t j = i; j < 13; ++j)
            for (uint8_t k = j; k < 13; ++k)
                for (uint8_t l = k; l < 13; ++l)
                    for (uint8_t m = l; m < 13; ++m) {
                        uint16_t bits = (1 << i) | (1 << j) | (1 << k) |
                                        (1 << l) | (1 << m);
                        if (is_straight(bits))
                            continue;

                        int uniqueRanks = 0;
                        for (int z = 0; z < 13; ++z)
                            uniqueRanks += ((1 << z) & bits) > 0;
                        if (uniqueRanks <= 1 || uniqueRanks >= 5)
                            continue;

                        primesHands[idx++] = i;
                        primesHands[idx++] = j;
                        primesHands[idx++] = k;
                        primesHands[idx++] = l;
                        primesHands[idx++] = m;
                    }
    assert(idx == 4888 * 5);

    qsort(primesHands, 4888, sizeof(uint8_t) * 5, compare_group_hands);

    for (int n = 0; n < 4888; ++n) {
        if (n < 312)
            primes[n].rank = n + 10 + 1; // Quads or full house
        else
            primes[n].rank = n + 10 + 1287 + 1; // Trips, two pair or pair

        uint8_t *cards = primesHands + (n * 5);
        primes[n].q = rankPrimes[cards[0]] * rankPrimes[cards[1]] *
                      rankPrimes[cards[2]] * rankPrimes[cards[3]] *
                      rankPrimes[cards[4]];
    }

    qsort(primes, 4888, sizeof(primeinfo_t), compare_prime_info);
}

#define test_assert(expr)                                                      \
    if (!(expr))                                                               \
        return 1;

int eval_test() {
    test_assert(
        eval5((card_t[]){create_card(Ace, Spades), create_card(Two, Spades),
                         create_card(Three, Spades), create_card(Four, Spades),
                         create_card(Five, Spades)}) == 10);
    test_assert(
        eval5((card_t[]){create_card(Ace, Spades), create_card(King, Spades),
                         create_card(Ten, Spades), create_card(Queen, Spades),
                         create_card(Jack, Spades)}) == 1);
    test_assert(
        eval5((card_t[]){create_card(Ace, Spades), create_card(King, Spades),
                         create_card(Nine, Spades), create_card(Queen, Spades),
                         create_card(Jack, Spades)}) == 323);

    test_assert(
        eval5((card_t[]){create_card(Ten, Hearts), create_card(King, Spades),
                         create_card(Nine, Clubs), create_card(Queen, Spades),
                         create_card(Jack, Diamonds)}) == 1601);
    test_assert(
        eval5((card_t[]){create_card(Two, Hearts), create_card(Three, Spades),
                         create_card(Seven, Clubs), create_card(Four, Spades),
                         create_card(Five, Diamonds)}) == 7462);
    test_assert(
        eval5((card_t[]){create_card(Two, Hearts), create_card(Three, Diamonds),
                         create_card(Five, Clubs), create_card(Four, Clubs),
                         create_card(Eight, Spades)}) == 7458);
    test_assert(
        eval5((card_t[]){create_card(Ace, Hearts), create_card(Nine, Diamonds),
                         create_card(King, Clubs), create_card(Queen, Clubs),
                         create_card(Jack, Spades)}) ==
        10 + 156 * 2 + 1277 + 858 * 2 + 10 + 2860 + 1);

    test_assert(
        eval5((card_t[]){create_card(Ace, Diamonds), create_card(Ace, Hearts),
                         create_card(Ace, Clubs), create_card(Ace, Spades),
                         create_card(King, Hearts)}) == 11);
    test_assert(
        eval5((card_t[]){create_card(Two, Diamonds), create_card(Five, Hearts),
                         create_card(Two, Clubs), create_card(Four, Spades),
                         create_card(Three, Hearts)}) ==
        10 + 156 * 2 + 1277 + 858 * 2 + 10 + 2860);

    return 0;
}

int32_t binary_search(const primeinfo_t *arr, int32_t low, int32_t high,
                      uint32_t q) {
    while (low <= high) {
        int32_t mid = (low + high) / 2;

        if (arr[mid].q == q)
            return mid;

        if (arr[mid].q < q)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return -1;
}

uint16_t eval5(card_t cards[5]) {
    uint32_t flush = 0xF000, q = 0;
    for (int i = 0; i < 5; ++i) {
        flush &= cards[i];
        q |= cards[i];
    }
    q >>= 16;

    uint16_t eval;
    if (flush)
        eval = flushTable[q];
    else
        eval = uniquesTable[q];

    if (!eval) {
        uint32_t q = 1;
        for (int i = 0; i < 5; ++i)
            q *= cards[i] & 0xFF;
        int32_t idx = binary_search(primes, 0, 4888, q);
        if (idx != -1)
            eval = primes[idx].rank;
    }

    return eval;
}

uint16_t eval6(card_t cards[6]) {
    uint16_t best = 0;
    card_t card5[5] = {0};
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 5; ++j)
            card5[j] = cards[perm6[i][j]];
        uint16_t eval = eval5(card5);
        if (best == 0 || eval < best)
            best = eval;
    }
    return best;
}

uint16_t eval7(card_t cards[7]) {
    uint16_t best = 0;
    card_t card5[5] = {0};
    for (int i = 0; i < 21; ++i) {
        for (int j = 0; j < 5; ++j)
            card5[j] = cards[perm7[i][j]];
        uint16_t eval = eval5(card5);
        if (best == 0 || eval < best)
            best = eval;
    }
    return best;
}
