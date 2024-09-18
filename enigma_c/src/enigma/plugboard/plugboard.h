#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t *plugboard_data;
} Plugboard;

Plugboard* create_plugboard(const char *input);
