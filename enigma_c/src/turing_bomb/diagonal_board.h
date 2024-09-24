#pragma once

//
// Created by Emanuel on 30.08.2024.
//

#include <stdbool.h>

#include "turing_bomb.h"
#include "cycle_finder.h"

typedef struct
{
    uint8_t diagonal_board[NUM_SCRAMBLERS_PER_ROW][ALPHABET_SIZE];
} DiagonalBoard;

bool passes_welchman_test(const Cycles *candidate_cycles, const Cycles *current_cycles);
