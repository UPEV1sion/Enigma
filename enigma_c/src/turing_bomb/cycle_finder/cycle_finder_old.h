#pragma once


#include <stdint.h>
#include "turing_bomb/turing_bomb.h"

//
// Created by Emanuel on 30.08.2024.
//

#ifndef ALPHABET_SIZE
#define ALPHABET_SIZE 26
#endif
#ifndef ASCII_SIZE
#define ASCII_SIZE 256
#endif

typedef struct
{
    char *cycles[ALPHABET_SIZE];
    char *stubs[ALPHABET_SIZE];
    int8_t *positions[NUM_SCRAMBLERS_PER_ROW];
    uint32_t num_cycles;
    uint32_t num_stubs;
} Cycles;

// Cycles* find_cycles_old(const uint8_t *crib, const uint8_t *ciphertext, size_t crib_len);
