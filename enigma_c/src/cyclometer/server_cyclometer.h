#pragma once

#include <stdint.h>

#include "helper/helper.h"

//
// Created by escha on 1.1.25.
//


#define DAILY_KEY_SIZE 3


typedef struct
{
    uint8_t cycle_values[ALPHABET_SIZE];
    uint8_t length;
} Cycle;

Cycle* server_create_cycles(char **enc_daily_keys, int32_t daily_key_count);
