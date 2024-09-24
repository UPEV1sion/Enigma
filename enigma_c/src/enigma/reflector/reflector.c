#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "reflector.h"
#include "helper/helper.h"

#define UKW_B_WIRING  "YRUHQSLDPXNGOKMIEBFZCWVJAT"
#define UKW_C_WIRING  "FVPJIAOYEDRZXWGCTKUQSBNMHL"

Reflector* create_reflector(const char *wiring)
{
    Reflector *ukw = malloc(sizeof(Reflector));
    assertmsg(ukw != NULL, "ukw == NULL");
    ukw->wiring = malloc(ALPHABET_SIZE + 1);
    assertmsg(ukw->wiring, "ukw == NULL");

    for (uint16_t i = 0; i < ALPHABET_SIZE; i++)
    {
        ukw->wiring[i] = (uint8_t) (wiring[i] - 'A');
    }

    ukw->wiring[ALPHABET_SIZE] = 0;

    return ukw;
}

Reflector* create_reflector_by_type(const enum REFLECTOR_TYPE type)
{
    if(type == UKW_B)
    {
        return create_reflector(UKW_B_WIRING);
    }
    if (type == UKW_C)
    {
        return create_reflector(UKW_C_WIRING);
    }
    printf("Error, Reflector definition not found");
    exit(1);
}
