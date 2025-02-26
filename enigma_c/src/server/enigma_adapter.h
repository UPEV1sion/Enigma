//
// Created by escha on 10.02.25.
//

#pragma once

#include "enigma/plugboard/plugboard.h"
#include "enigma/reflector/reflector.h"
#include "enigma/rotor/rotor.h"
#include "enigma/enigma.h"

typedef struct
{
    char *plugboard;
    char *message;
    int *rotor_positions;
    int *ring_settings;
    enum ROTOR_TYPE *rotors;
    enum ENIGMA_TYPE type;
    enum REFLECTOR_TYPE reflector;
} EnigmaConfigAdapter;


typedef struct
{
    int daily_key_count;
    EnigmaConfigAdapter *enigma_conf;
} CyclometerAdapter;


int enigma_encrypt(EnigmaConfigAdapter *adapter, char *output);
int cyclometer_create_cycles (EnigmaConfigAdapter *adapter, int daily_key_count,
    int *cycles1, int cycles1_len, int *cycles2, int cycles2_len, int *cycles3, int cycles3_len);
