#pragma once

#include <stdint.h>

#include "enigma/reflector/reflector.h"
#include "helper/helper.h"
#include "diagonal_board.h"

//
// Created by Emanuel on 07.09.2024.
//

// This also defines the max crib length
#define NUM_SCRAMBLERS_PER_ROW 12

// This is over 2 KB be careful with wasteful allocations!
// Only allocate this on Stack. Heap will be too slow.
typedef struct TuringBomb
{
    ScramblerEnigma bomb_row[NUM_SCRAMBLERS_PER_ROW];
    DiagonalBoard *diagonal_board;
    Reflector reflector;
} TuringBomb;

int32_t start_turing_bomb(const char *crib, const char *ciphertext, uint32_t crib_pos);
