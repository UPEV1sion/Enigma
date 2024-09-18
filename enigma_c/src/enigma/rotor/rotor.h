#pragma once

#include <stdint.h>

typedef struct
{
    uint8_t wiring[26];
    uint8_t inverse_wiring[26];
    uint8_t position;
    uint8_t *notch;
    uint8_t notch_count;
    uint8_t offset;
} Rotor;

Rotor* create_one_notch_rotor(const char *wiring, const char *inverse_wiring, char notch, uint8_t position,
                              uint8_t offset);
Rotor* create_two_notch_rotor(const char *wiring, const char *inverse_wiring, char notch1, char notch2);
Rotor* create_rotor(uint8_t type, uint8_t position, uint8_t offset);
uint8_t traverse_rotor(const Rotor *rotor, uint8_t character);
uint8_t traverse_rotor_inverse(const Rotor *rotor, uint8_t character);
uint8_t should_rotate(const Rotor *rotor);
