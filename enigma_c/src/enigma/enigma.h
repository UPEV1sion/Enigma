#pragma once

#include <stdint.h>

#include "plugboard/plugboard.h"
#include "reflector/reflector.h"
#include "rotor/rotor.h"

enum
{
    M3 = 3,
    M4 = 4
} TYPE;

typedef struct
{
    Rotor **rotors;
    enum TYPE type; // 3 for M3, 4 for M4
    Reflector *reflector;
    Plugboard *plugboard;
    char *plaintext;
} Enigma;

typedef struct
{
    uint8_t *rotors;
    uint8_t *rotor_positions;
    uint8_t *ring_settings;
    enum TYPE type;
    char reflector;
    char plugboard[26];
    char *message;
} EnigmaConfiguration;

Enigma* create_enigma_from_configuration(const EnigmaConfiguration *enigma_configuration);
uint8_t* traverse_enigma(const Enigma *enigma);
uint8_t* traverse_m3_enigma(const Enigma *enigma, uint8_t *text_in_integer, size_t array_size);
uint8_t* traverse_m4_enigma(const Enigma *enigma, uint8_t *text_in_integer, size_t array_size);
void free_enigma(Enigma *enigma);
