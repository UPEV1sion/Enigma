#pragma once

#include <stdint.h>

#include "enigma/reflector/reflector.h"
#include "diagonal_board.h"
#include "enigma/rotor/rotor.h"

//
// Created by Emanuel on 07.09.2024.
//

#define MAX_CRIB_LEN                26
#define NUM_SCRAMBLERS_PER_ROW      12
#define NUM_SCRAMBLERS_PER_COLUMN   3

int32_t start_turing_bomb(const char *crib, const char *ciphertext, uint32_t crib_offset);
