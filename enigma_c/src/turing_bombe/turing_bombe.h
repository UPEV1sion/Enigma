#pragma once

#include <stdint.h>

#include "enigma/reflector/reflector.h"
#include "enigma/rotor/rotor.h"

//
// Created by Emanuel on 07.09.2024.
//

#define MAX_CRIB_LEN                25
#define NUM_SCRAMBLERS_PER_ROW      12
#define NUM_SCRAMBLERS_PER_COLUMN   3
#define NUM_CONTACTS_PER_COMMON     5
#define NUM_COMMONS                 6

typedef struct BombeNode BombeNode;

int32_t start_turing_bombe(const char *crib, const char *ciphertext, uint32_t crib_offset);
