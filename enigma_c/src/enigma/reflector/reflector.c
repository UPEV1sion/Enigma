#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "reflector.h"

#define UKW_A_WIRING        "EJMZALYXVBWFCRQUONTSPIKHGD"
#define UKW_B_WIRING        "YRUHQSLDPXNGOKMIEBFZCWVJAT"
#define UKW_C_WIRING        "FVPJIAOYEDRZXWGCTKUQSBNMHL"

#define UKW_B_THIN_WIRING  "ENKQAUYWJICOPBLMDXZVFTHRGS"
#define UKW_C_THIN_WIRING  "RDOBJNTKVEHMLFCWZAXGYIPSUQ"


/**
 * @brief Creates a reflector with the specified wiring
 * @param wiring The wiring of the reflector
 * @return Reflector*
 */
Reflector* create_reflector(const char *wiring)
{
    Reflector *ukw = malloc(sizeof(Reflector));
    assertmsg(ukw != NULL, "malloc failed");

    for (uint16_t i = 0; i < ALPHABET_SIZE; i++)
    {
        ukw->wiring[i] = wiring[i] - 'A';
    }

    return ukw;
}

/**
 * @brief A factory method for creating Reflectors
 * @param type The type of the reflector
 * @return Reflector*
 */
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
        case UKW_B_THIN:
            return create_reflector(UKW_B_THIN_WIRING);
        case UKW_C_THIN:
            return create_reflector(UKW_C_THIN_WIRING);
        default:
            fprintf(stderr, "Error, Reflector definition not found: %d\n", type);
            exit(1);
    }
}
