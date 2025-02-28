#include <assert.h>
#include <cmph.h>
#include <cmph_types.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMPARE(a, b) (((a) > (b)) - ((a) < (b)))

typedef struct PrimeInfo {
    uint32_t q;
    uint16_t rank;
} primeinfo_t;

const uint32_t rankPrimes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41};
uint16_t unique5[1287] = {0}, ranks[0x1F01] = {0}, hashTableRanks[4888] = {0};
uint8_t hands[4888 * 5] = {0};
primeinfo_t primes[4888] = {0};

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

int main(void) {
    uint32_t idx = 0;
    for (uint16_t hand = 0b11111; hand < (1 << 13); ++hand) {
        int count = 0;
        for (int j = 0; j < 13; ++j)
            if (hand & (1 << j))
                count++;
        if (count == 5)
            unique5[idx++] = hand;
    }

    qsort(unique5, 1287, sizeof(uint16_t), unique_sort);

    for (uint16_t hand = 0; hand < 0x1F01; hand++) {
        for (uint16_t j = 0; j < 1287; ++j) {
            if (unique5[j] == hand) {
                ranks[hand] = (j > 9) ? (j + 156 * 2 + 1) : (j + 1);
                break;
            }
        }
    }

    FILE *fp = fopen("tables.h", "w");
    fprintf(
        fp,
        "#ifndef ARRAYS_H\n#define ARRAYS_H\n\n#include <stdint.h>\n\ntypedef "
        "struct PrimeInfo { uint32_t q; uint16_t rank; } primeinfo_t;\n\n");

    fprintf(fp, "const uint16_t flushTable[0x1F01] = {\n\t");

    for (int i = 0; i < 0x1F01; ++i) {
        fprintf(fp, "%d, ", ranks[i]);
    }

    fprintf(fp, "\n};\n\n");

    memset(ranks, 0, 0x1F01 * sizeof(uint16_t));

    for (uint16_t hand = 0; hand < 0x1F01; hand++) {
        for (uint16_t j = 0; j < 1287; ++j) {
            if (unique5[j] == hand) {
                ranks[hand] =
                    (j > 9) ? (j + 10 + 156 * 2 + 1277 + 858 * 2 + 2860 + 1)
                            : (j + 10 + 156 * 2 + 1277 + 1);
                break;
            }
        }
    }

    fprintf(fp, "const uint16_t uniquesTable[0x1F01] = {\n\t");

    for (int i = 0; i < 0x1F01; ++i) {
        fprintf(fp, "%d, ", ranks[i]);
    }

    fprintf(fp, "\n};\n\n");

    // Generate all possible hands that aren't straights or entirely unique
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

                        hands[idx++] = i;
                        hands[idx++] = j;
                        hands[idx++] = k;
                        hands[idx++] = l;
                        hands[idx++] = m;
                    }
    assert(idx == 4888 * 5);

    qsort(hands, 4888, sizeof(uint8_t) * 5, compare_group_hands);

    for (int n = 0; n < 4888; ++n) {
        if (n < 312)
            primes[n].rank = n + 10 + 1; // Quads or full house
        else
            primes[n].rank = n + 10 + 1287 + 1; // Trips, two pair or pair

        uint8_t *cards = hands + (n * 5);
        primes[n].q = rankPrimes[cards[0]] * rankPrimes[cards[1]] *
                      rankPrimes[cards[2]] * rankPrimes[cards[3]] *
                      rankPrimes[cards[4]];
    }

    qsort(primes, 4888, sizeof(primeinfo_t), compare_prime_info);

    fprintf(fp, "const primeinfo_t primes[4888] = {");
    for (int i = 0; i < 4888; ++i) {
        if (i != 0)
            assert(primes[i].q > primes[i - 1].q);
        fprintf(fp, "{%u,%d},", primes[i].q, primes[i].rank);
    }
    fprintf(fp, "};\n");

    FILE *hashFile = fopen("hash.mph", "w");

    cmph_io_adapter_t *source = cmph_io_struct_vector_adapter(
        primes, sizeof(primeinfo_t), offsetof(primeinfo_t, q), sizeof(uint32_t),
        4888);
    cmph_config_t *config = cmph_config_new(source);
    cmph_config_set_algo(config, CMPH_BMZ);
    cmph_config_set_mphf_fd(config, hashFile);
    cmph_t *hash = cmph_new(config);
    cmph_config_destroy(config);
    cmph_dump(hash, hashFile);
    fclose(hashFile);

    for (int i = 0; i < 4888; ++i)
        hashTableRanks[cmph_search(hash, (char *)&primes[i].q,
                                   sizeof(uint32_t))] = primes[i].rank;

    cmph_destroy(hash);
    cmph_io_struct_vector_adapter_destroy(source);

    fprintf(fp, "const uint16_t hashTable[4888] = {");
    for (int i = 0; i < 4888; ++i)
        fprintf(fp, "%d,", hashTableRanks[i]);

    fprintf(fp, "\n};\n");

    fprintf(fp, "#endif // ARRAYS_H\n");
    fclose(fp);

    return 0;
}
