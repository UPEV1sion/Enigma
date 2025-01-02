#pragma once
#include "enigma/enigma.h"

//
// Created by Emanuel on 22.12.2024.
//

typedef struct
{
    int32_t daily_key_count;
    EnigmaConfiguration *enigma_conf;
} ServerCyclometerOptions;


int32_t server_run(void);
