#pragma once


#include <stdint.h>
#include "turing_bomb.h"

//
// Created by Emanuel on 30.08.2024.
//

#ifndef ALPHABET_SIZE
#define ALPHABET_SIZE 26
#endif
#ifndef ASCII_SIZE
#define ASCII_SIZE 256
#endif

// The old approach is suffiecient?
// typedef struct
// {
//     char *stubs[ALPHABET_SIZE];
//     uint8_t indexes_cycle[NUM_SCRAMBLERS_PER_ROW];
//     char *cycle;
//     uint32_t num_stubs;
// } Cycle;

typedef struct
{
    char *cycles[ALPHABET_SIZE];
    char *stubs[ALPHABET_SIZE];
    uint32_t num_cycles;
    uint32_t num_stubs;
} Cycles;

Cycles* find_cycles(const uint8_t *crib, const uint8_t *ciphertext, size_t crib_len);
int32_t create_bomb_menu(TuringBomb *turing_bomb, const uint8_t *crib, const uint8_t *ciphertext, const size_t crib_len);
