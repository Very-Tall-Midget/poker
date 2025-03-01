/*
 * Algorithm from TwoPlusTwo
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "eval.h"

#define HAND_RANKS_FLIP

int64_t ids[612978];
int32_t handRanks[32487834];

int64_t maxID = 0;
int32_t numIDs = 1, nCards = 0, maxHandRank = 0;

int64_t makeID(int64_t idIn, int32_t newCard) {
    // In format rrrrxxss, rrrr is 1 based, ss is 0 for no suit
    int32_t workCards[8] = {0}; // One at end for loop breaking

    for (int32_t i = 0; i < 6; ++i)
        workCards[i + 1] = (int32_t)((idIn >> (8 * i)) & 0xFF);

    newCard--; // make 0 based

    workCards[0] = (((newCard >> 2) + 1) << 4) + (newCard & 0b11) + 1;

    bool duplicate = false;
    int32_t suitCount[5] = {0}, rankCount[14] = {0};
    for (nCards = 0; workCards[nCards]; ++nCards) {
        suitCount[workCards[nCards] & 0xF]++;
        rankCount[(workCards[nCards] >> 4) & 0xF]++;
        if (nCards && workCards[0] == workCards[nCards])
            duplicate = true;
    }

    if (duplicate)
        return 0;

    // Check for more than 4 of same rank
    if (nCards > 4)
        for (int32_t rank = 1; rank < 14; ++rank)
            if (rankCount[rank] > 4)
                return 0;

    int32_t needSuited = nCards - 2;
    if (needSuited > 1)
        for (int32_t i = 0; i < nCards; ++i)
            if (suitCount[workCards[i] & 0xF] < needSuited)
                workCards[i] &= 0xF0;

#define SWAP(x, y)                                                             \
    {                                                                          \
        if (workCards[x] < workCards[y]) {                                     \
            workCards[x] ^= workCards[y];                                      \
            workCards[y] ^= workCards[x];                                      \
            workCards[x] ^= workCards[y];                                      \
        }                                                                      \
    }

    // Sort
    SWAP(0, 4);
    SWAP(1, 5);
    SWAP(2, 6);
    SWAP(0, 2);
    SWAP(1, 3);
    SWAP(4, 6);
    SWAP(2, 4);
    SWAP(3, 5);
    SWAP(0, 1);
    SWAP(2, 3);
    SWAP(4, 5);
    SWAP(1, 4);
    SWAP(3, 6);
    SWAP(1, 2);
    SWAP(3, 4);
    SWAP(5, 6);

    int64_t id = 0;
    for (int32_t i = 0; i < 7; ++i)
        id += (int64_t)workCards[i] << (8 * i);

    return id;
}

int32_t saveID(int64_t id) {
    if (id == 0)
        return 0;

    if (id >= maxID) {
        if (id > maxID) {
            ids[numIDs++] = id;
            maxID = id;
        }
        return numIDs - 1;
    }

    int32_t low = 0, high = numIDs - 1;
    while (high - low > 1) {
        int32_t mid = (high + low + 1) / 2;
        int64_t testVal = ids[mid] - id;
        if (testVal > 0)
            high = mid;
        else if (testVal < 0)
            low = mid;
        else
            return mid;
    }

    memmove(&ids[high + 1], &ids[high], (numIDs - high) * sizeof(ids[0]));

    ids[high] = id;
    numIDs++;
    return high;
}

int32_t eval(int64_t idIn) {
    int32_t holdCards[8] = {0}, numEvalCards = 0, suit, mainSuit = 20, rank = 0;

    if (idIn) {
        for (int32_t i = 0; i < 7; ++i) {
            holdCards[i] = (int32_t)((idIn >> (8 * i)) & 0xFF);
            if (holdCards[i] == 0)
                break;
            ++numEvalCards;
            if ((suit = holdCards[i] & 0xF))
                mainSuit = suit;
        }

        card_t cards[7] = {0};
        int32_t suitIter = 1;
        for (int32_t i = 0; i < numEvalCards; ++i) {
            int32_t card = holdCards[i];
            int32_t suit = card & 0xF;

            if (suit == 0) {
                suit = suitIter++;
                if (suitIter == 5)
                    suitIter = 1;
                if (suit == mainSuit) {
                    suit = suitIter++;
                    if (suitIter == 5)
                        suitIter = 1;
                }
            }

            cards[i] = create_card((card >> 4) - 1, suit - 1);
        }

        switch (numEvalCards) {
        case 5:
            rank = eval5(cards);
            break;
        case 6:
            rank = eval6(cards);
            break;
        case 7:
            rank = eval7(cards);
            break;
        default:
            __builtin_unreachable();
            break;
        }

#ifdef HAND_RANKS_FLIP
        rank = 7463 - rank;

        if (rank < 1278)
            rank = rank - 0 + 4096 * 1; // high card
        else if (rank < 4138)
            rank = rank - 1277 + 4096 * 2; // one pair
        else if (rank < 4996)
            rank = rank - 4137 + 4096 * 3; // two pair
        else if (rank < 5854)
            rank = rank - 4995 + 4096 * 4; // three of a kind
        else if (rank < 5864)
            rank = rank - 5853 + 4096 * 5; // straight
        else if (rank < 7141)
            rank = rank - 5863 + 4096 * 6; // flush
        else if (rank < 7297)
            rank = rank - 7140 + 4096 * 7; // full house
        else if (rank < 7453)
            rank = rank - 7296 + 4096 * 8; // quads
        else
            rank = rank - 7452 + 4096 * 9; // straight flush

#endif // HAND_RANKS_FLIP
    }

    return rank;
}

int main() {
    printf("Initialising 5 card lookup tables...\n");
    eval_init();
    if (eval_test()) {
        printf("Evaluation tests failed\n");
        return 1;
    }

    printf("5 card lookup tables initialised\n");

    memset(ids, 0, sizeof(ids));
    memset(handRanks, 0, sizeof(handRanks));

    printf("Generating card IDs\n");

    for (int32_t i = 0; ids[i] || i == 0; ++i) {
        for (int32_t card = 1; card < 53; ++card) {
            int64_t id = makeID(ids[i], card);
            if (nCards < 7)
                saveID(id);
        }
        printf("\rID - %d", i);
    }

    printf("\nSetting hand ranks\n");

    for (int32_t i = 0; ids[i] || i == 0; ++i) {
        for (int32_t card = 1; card < 53; ++card) {
            int64_t id = makeID(ids[i], card);
            int32_t idSlot;
            if (nCards < 7)
                idSlot = saveID(id) * 53 + 53;
            else
                idSlot = eval(id);

            maxHandRank = i * 53 + card + 53;
            handRanks[maxHandRank] = idSlot;
        }

        if (nCards == 6 || nCards == 7)
            handRanks[i * 53 + 53] = eval(ids[i]);

        printf("\rID - %d", i);
    }

    printf("\nNumber of IDs = %d\nmaxHandRank = %d\n", numIDs, maxHandRank);

    int32_t count = 0, handTypes[10] = {0};
    for (int c0 = 1; c0 < 53; c0++) {
        int u0 = handRanks[53 + c0];
        for (int c1 = c0 + 1; c1 < 53; c1++) {
            int u1 = handRanks[u0 + c1];
            for (int c2 = c1 + 1; c2 < 53; c2++) {
                int u2 = handRanks[u1 + c2];
                for (int c3 = c2 + 1; c3 < 53; c3++) {
                    int u3 = handRanks[u2 + c3];
                    for (int c4 = c3 + 1; c4 < 53; c4++) {
                        int u4 = handRanks[u3 + c4];
                        for (int c5 = c4 + 1; c5 < 53; c5++) {
                            int u5 = handRanks[u4 + c5];
                            for (int c6 = c5 + 1; c6 < 53; c6++) {
                                handTypes[handRanks[u5 + c6] >> 12]++;
                                count++;
                            }
                        }
                    }
                }
            }
        }
    }

    char handRankStr[][16] = {
        "Should be 0", "High card", "Pair",       "Two pair", "Three of a kind",
        "Straight",    "Flush",     "Full house", "Quads",    "Straight flush"};
    for (int i = 0; i < 10; ++i) {
        printf("%16s = %d\n", handRankStr[i], handTypes[i]);
    }

    printf("Total hands = %d\n", count);

    printf("Saving to handranks.dat...\n");

    FILE *fp = fopen("handranks.dat", "wb");
    if (!fp) {
        printf("Failed to open handranks.dat\n");
        return 1;
    }

    uint64_t bytes = fwrite(handRanks, sizeof(handRanks[0]),
                            sizeof(handRanks) / sizeof(handRanks[0]), fp);
    printf("Wrote %ld bytes to handranks.dat\n", bytes);

    fclose(fp);

    return 0;
}
