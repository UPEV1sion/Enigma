#pragma once

typedef struct
{
    uint8_t *wiring;
} Reflector;

Reflector* create_reflector(const char *wiring);
Reflector* create_reflector_by_type(char type);
