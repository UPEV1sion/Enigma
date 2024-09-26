#pragma once

#include <stdint.h>

#include "helper/helper.h"

typedef struct
{
    uint8_t plugboard_data[ALPHABET_SIZE];
} Plugboard;

Plugboard* create_plugboard(const char *input);
