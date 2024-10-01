#pragma once


#include <stdint.h>
#include "turing_bomb/turing_bomb.h"

//
// Created by Emanuel on 30.08.2024.
//

typedef struct
{
    uint8_t chars_w_stubs[ALPHABET_SIZE];
    uint8_t chars_wo_stubs[ALPHABET_SIZE];
    int8_t positions_w_stubs[ALPHABET_SIZE];
    int8_t positions_wo_stubs[ALPHABET_SIZE];
    uint8_t len_w_stubs;
    uint8_t len_wo_stubs;
} CycleCribPlain;

typedef struct
{
    CycleCribPlain *cycles_positions[ALPHABET_SIZE];
    uint32_t num_cycles;
} CyclesCribPlain;


CyclesCribPlain* find_cycles(const char *crib, const char *ciphertext);
