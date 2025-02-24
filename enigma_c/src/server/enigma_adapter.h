//
// Created by escha on 10.02.25.
//

#pragma once

#include "enigma/plugboard/plugboard.h"
#include "enigma/reflector/reflector.h"
#include "enigma/rotor/rotor.h"
#include "enigma/enigma.h"

typedef struct {
    char *plugboard;
    char *message;
    int *rotor_positions;
    int *ring_settings;
    enum ROTOR_TYPE *rotors;
    enum ENIGMA_TYPE type;
    enum REFLECTOR_TYPE reflector;
}EnigmaConfigAdapter;

int enigma_encrypt(EnigmaConfigAdapter *adapter, char *output);
