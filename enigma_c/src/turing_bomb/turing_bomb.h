#pragma once

#include <stdint.h>

#include "enigma/rotor/rotor.h"
#include "enigma/reflector/reflector.h"
#include "helper/helper.h"
#include "diagonal_board.h"

//
// Created by Emanuel on 07.09.2024.
//

// This also defines the max crib length
#define NUM_SCRAMBLERS_PER_ROW 12
#define NUM_ROTORS_PER_ENIGMA  3

//TODO cycle reference header
typedef struct
{
    cable_t in, out;
    Rotor *rotors[NUM_ROTORS_PER_ENIGMA];
} ScramblerEnigma;

// This is over 1 KB be careful with wasteful allocations!
// Only allocate this on Stack. Heap will be too slow.
typedef struct
{
    ScramblerEnigma bomb_row[NUM_SCRAMBLERS_PER_ROW];
    //TODO this might need to be a struct
    DiagonalBoard *diagonal_board;
    Reflector reflector;
} TuringBomb;

int32_t start_turing_bomb(const char *crib, const char *ciphertext, uint32_t crib_pos);
