#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t wiring[26];
    uint8_t inverse_wiring[26];
    uint8_t position;
    uint8_t *notch;
    uint8_t notch_count;
    uint8_t offset;
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
    ROTOR_8
};

Rotor* create_rotor(uint8_t type, uint8_t position, uint8_t offset);
uint8_t traverse_rotor(const Rotor *rotor, uint8_t character);
uint8_t traverse_rotor_inverse(const Rotor *rotor, uint8_t character);
bool should_rotate(const Rotor *rotor);
