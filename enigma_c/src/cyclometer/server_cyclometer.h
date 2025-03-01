#pragma once

#include <stdint.h>

#include "helper/helper.h"

//
// Created by escha on 1.1.25.
//


#define DAILY_KEY_SIZE 3

typedef struct
{
    int cycles_1_4_len;
    int cycles_2_5_len;
    int cycles_3_6_len;
    int cycles_1_4[ALPHABET_SIZE];
    int cycles_2_5[ALPHABET_SIZE];
    int cycles_3_6[ALPHABET_SIZE];
} ComputedCycles;

typedef struct
{
    int rotor_one_permutations[ALPHABET_SIZE];
    int rotor_two_permutations[ALPHABET_SIZE];
    int rotor_three_permutations[ALPHABET_SIZE];
} RotorPermutations;

void add_daily_key_to_permutations(RotorPermutations *permutations, const uint8_t *daily_key_as_int);
void server_compute_cycles(const RotorPermutations *permutations, ComputedCycles *restrict computed_cycles);
