#pragma once

typedef struct
{
    uint8_t *wiring;
} Reflector;

enum REFLECTOR_TYPE
{
    UKW_A = 'A',
    UKW_B = 'B',
    UKW_C = 'C'
};

Reflector* create_reflector(const char *wiring);
Reflector* create_reflector_by_type(enum REFLECTOR_TYPE type);
