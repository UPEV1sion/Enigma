#pragma once

#include <stdint.h>

#include "plugboard/plugboard.h"
#include "reflector/reflector.h"
#include "rotor/rotor.h"

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
    char reflector;
} EnigmaConfiguration;

Enigma* create_enigma_from_configuration(const EnigmaConfiguration *enigma_configuration);
uint8_t* traverse_enigma(const Enigma *enigma);
uint8_t* traverse_m3_enigma(const Enigma *enigma, uint8_t *text_in_integer, size_t array_size);
uint8_t* traverse_m4_enigma(const Enigma *enigma, uint8_t *text_in_integer, size_t array_size);
void free_enigma(Enigma *enigma);
