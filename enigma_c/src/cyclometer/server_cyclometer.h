#pragma once

#include <stdint.h>

#include "helper/helper.h"

//
// Created by escha on 1.1.25.
//


#define NUM_ROTORS_PER_ENIGMA      3


typedef struct
{
    uint8_t cycle_values[ALPHABET_SIZE];
    uint8_t length;
} Cycle;

void s_create_cycles(void);