#pragma once

#include <stdint.h>

//
// Created by Emanuel on 30.08.2024.
//

#ifndef ALPHABET_SIZE
#define ALPHABET_SIZE 26
#endif
#ifndef ASCII_SIZE
#define ASCII_SIZE 256
#endif


typedef struct
{
    char *cycles[ALPHABET_SIZE];
    char *stubs[ALPHABET_SIZE];
    int32_t num_cycles;
    int32_t num_stubs;
} Cycles;

Cycles* find_cycles(const char *crib, const char *ciphertext);
