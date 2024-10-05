#pragma once
#include "helper/helper.h"

typedef struct
{
    uint8_t wiring[ALPHABET_SIZE];
} Reflector;

//TODO implement the UKW D reflector.
enum REFLECTOR_TYPE
{
    UKW_A      = 'A',
    UKW_B      = 'B',
    UKW_C      = 'C',
    UKW_B_THIN = 'b',
    UKW_C_THIN = 'c'
};

Reflector* create_reflector(const char *wiring);
Reflector* create_reflector_by_type(enum REFLECTOR_TYPE type);
