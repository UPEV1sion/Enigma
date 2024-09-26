#pragma once

//
// Created by Emanuel on 30.08.2024.
//

#include <stdbool.h>

#include "helper/helper.h"

#define NUM_FIELDS_PER_COMMON 5
#define NUM_COMMONS 6

typedef uint8_t cable_t[ALPHABET_SIZE];

typedef struct
{
    cable_t co[NUM_FIELDS_PER_COMMON];
} CommonConnections;

typedef struct
{
    cable_t alphabet[ALPHABET_SIZE];
    CommonConnections commons[NUM_COMMONS];
} DiagonalBoard;

// bool passes_welchman_test(const Cycles *candidate_cycles, const Cycles *current_cycles);
