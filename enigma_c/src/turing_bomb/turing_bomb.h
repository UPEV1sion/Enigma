#pragma once

#include <stdint.h>

#include "enigma/reflector/reflector.h"
#include "diagonal_board.h"

//
// Created by Emanuel on 07.09.2024.
//

#define NUM_SCRAMBLERS_PER_ROW 12
#define MAX_CRIB_LEN 26

// Allocate this on the Stack if possible
typedef struct TuringBomb
{
    // TODO pointer array?
    ScramblerEnigma bomb_row[NUM_SCRAMBLERS_PER_ROW];
    DiagonalBoard *diagonal_board;
    Reflector reflector;
    uint8_t scrambler_columns_used;
} TuringBomb;

int32_t start_turing_bomb(const char *crib, const char *ciphertext, uint32_t crib_offset);
