#pragma once

#include "helper/helper.h"

#define NUM_ROTORS_PER_ENIGMA      3

typedef struct
{
    uint8_t cycle_values[ALPHABET_SIZE];
    uint8_t length;
} Cycle;

void create_cycles(void);
