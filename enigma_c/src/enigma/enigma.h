#pragma once

#include <stdint.h>

#include "plugboard/plugboard.h"
#include "reflector/reflector.h"
#include "rotor/rotor.h"

// I really like the simplicity of using this in loops and malloc's.
// But when I add the ENIGMA_M1 and assign it to value 3, the two became synonymous.
enum ENIGMA_TYPE
{
    ENIGMA_M3 = 3,
    ENIGMA_M4 = 4
};

typedef struct
{
    Rotor **rotors;
    Reflector *reflector;
    Plugboard *plugboard;
    char *plaintext;
    enum ENIGMA_TYPE type; // 3 for M3, 4 for M4
} Enigma;

typedef struct
{
    char plugboard[26];
    char *message;
    uint8_t *rotor_positions;
    uint8_t *ring_settings;
    enum ROTOR_TYPE *rotors;
    enum ENIGMA_TYPE type;
    enum REFLECTOR_TYPE reflector;
} EnigmaConfiguration;

Enigma* create_enigma_from_configuration(const EnigmaConfiguration *enigma_configuration);
uint8_t* traverse_enigma(const Enigma *enigma);
void free_enigma(Enigma *enigma);
