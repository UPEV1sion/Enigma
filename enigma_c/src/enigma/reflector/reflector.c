#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "reflector.h"
#include "helper/helper.h"

#define UKW_B  "YRUHQSLDPXNGOKMIEBFZCWVJAT"
#define UKW_C  "FVPJIAOYEDRZXWGCTKUQSBNMHL"

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

Reflector* create_reflector_by_type(const char type)
{
    if ('C' == type)
    {
        return create_reflector(UKW_C);
    }

    return create_reflector(UKW_B);
}
