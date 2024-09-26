#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "reflector.h"
#include "helper/helper.h"

#define UKW_A_WIRING  "EJMZALYXVBWFCRQUONTSPIKHGD"
#define UKW_B_WIRING  "YRUHQSLDPXNGOKMIEBFZCWVJAT"
#define UKW_C_WIRING  "FVPJIAOYEDRZXWGCTKUQSBNMHL"

Reflector* create_reflector(const char *wiring)
{
    Reflector *ukw = malloc(sizeof(Reflector));
    assertmsg(ukw != NULL, "ukw == NULL");
    ukw->wiring = malloc(ALPHABET_SIZE + 1);
    assertmsg(ukw->wiring, "ukw->wiring == NULL");

    for (uint16_t i = 0; i < ALPHABET_SIZE; i++)
    {
        ukw->wiring[i] = wiring[i] - 'A';
    }

    ukw->wiring[ALPHABET_SIZE] = 0;

    return ukw;
}

Reflector* create_reflector_by_type(const enum REFLECTOR_TYPE type)
{
    switch (type)
    {
        case UKW_A:
            return create_reflector(UKW_A_WIRING);
        case UKW_B:
            return create_reflector(UKW_B_WIRING);
        case UKW_C:
            return create_reflector(UKW_C_WIRING);
        default:
            fprintf(stderr, "Error, Reflector definition not found: %d", type);
            exit(1);
    }
}
