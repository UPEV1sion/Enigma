#pragma once

#include <stdint.h>

#include "enigma/rotor/rotor.h"
#include "enigma/reflector/reflector.h"
#include "helper/helper.h"

//
// Created by Emanuel on 07.09.2024.
//

// This also defines the max crib length
#define NUM_SCRAMBLERS_PER_ROW 12
#define NUM_ROTORS_PER_ENIGMA  3


//TODO cycle reference
typedef struct DiagonalBoard DiagonalBoard;

typedef struct
{
    Rotor *rotors[NUM_ROTORS_PER_ENIGMA];
    Reflector reflector;
} ScramblerEnigma;

// This is almost 700 bytes be careful with wasteful allocations!
// Only allocate this on Stack. Heap will too slow.
typedef struct
{
    ScramblerEnigma bomb_row[NUM_SCRAMBLERS_PER_ROW];
    uint8_t diagonal_board[NUM_SCRAMBLERS_PER_ROW][ALPHABET_SIZE];
} TuringBomb;

int32_t start_turing_bomb(char *crib, const char *ciphertext, uint32_t crib_pos);
