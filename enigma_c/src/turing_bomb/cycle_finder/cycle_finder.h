#pragma once


#include <stdint.h>

//
// Created by Emanuel on 30.08.2024.
//

typedef struct
{
    char chars_w_stubs[ALPHABET_SIZE];
    char chars_wo_stubs[ALPHABET_SIZE];
    uint8_t positions_w_stubs[ALPHABET_SIZE]; // with / without
    uint8_t positions_wo_stubs[ALPHABET_SIZE];
    uint8_t len_w_stubs;
    uint8_t len_wo_stubs;
} CycleCribCipher;

typedef struct
{
    CycleCribCipher *cycles_positions[ALPHABET_SIZE];
    uint32_t num_cycles;
} CyclesCribCipher;

DEPRECATED("This function is deprecated. Use find_best_cycle_graph() instead.")
CyclesCribCipher* find_cycles(const char *crib, const char *ciphertext);
