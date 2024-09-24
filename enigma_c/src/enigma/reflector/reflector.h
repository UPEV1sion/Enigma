#pragma once

typedef struct
{
    uint8_t *wiring;
} Reflector;

enum REFLECTOR_TYPE
{
    UKW_B = 'B',
    UKW_C = 'C',
};

Reflector* create_reflector(const char *wiring);
Reflector* create_reflector_by_type(char type);
