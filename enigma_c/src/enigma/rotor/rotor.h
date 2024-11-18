#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "helper/helper.h"

typedef struct
{
    uint8_t wiring[ALPHABET_SIZE];
    uint8_t inverse_wiring[ALPHABET_SIZE];
    uint8_t *notch;
    uint8_t position;
    uint8_t notch_count;
} Rotor;

enum ROTOR_TYPE
{
    ROTOR_1 = 1,
    ROTOR_2,
    ROTOR_3,
    ROTOR_4,
    ROTOR_5,
    ROTOR_6,
    ROTOR_7,
    ROTOR_8,
    ROTOR_BETA,
    ROTOR_GAMMA
};

Rotor* create_rotor_by_type(enum ROTOR_TYPE type, uint8_t position, uint8_t offset);
uint8_t traverse_rotor(const Rotor *rotor, uint8_t character);
uint8_t traverse_rotor_inverse(const Rotor *rotor, uint8_t character);
bool should_rotate(const Rotor *rotor);
