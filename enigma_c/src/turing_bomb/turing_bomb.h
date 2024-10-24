#pragma once

#include <stdint.h>

#include "enigma/reflector/reflector.h"
#include "diagonal_board.h"
#include "enigma/rotor/rotor.h"

//
// Created by Emanuel on 07.09.2024.
//

#define MAX_CRIB_LEN                26
#define NUM_SCRAMBLERS_PER_ROW      12
#define NUM_SCRAMBLERS_PER_COLUMN   3

typedef struct
{
    uint8_t active_cable_connections[ALPHABET_SIZE];
    uint32_t active_contacts;
    //    Contact *commons;
    uint8_t num_active_connections;
    //    uint8_t num_common_connections;
    uint8_t contact_num;
} Contact;

typedef struct
{
    Contact *contacts[ALPHABET_SIZE];
    Contact *test_register;
    uint8_t num_commons;
} Terminal;

typedef struct ScramblerEnigma
{
    Contact *in, *out;
    Rotor *rotors[NUM_SCRAMBLERS_PER_COLUMN];
} ScramblerEnigma;

typedef struct TuringBomb
{
    // TODO pointer array?
    ScramblerEnigma bomb_row[NUM_SCRAMBLERS_PER_ROW];
    Terminal *terminal;
    Reflector *reflector;
    uint8_t scrambler_columns_used;
} TuringBomb;

int32_t start_turing_bomb(const char *crib, const char *ciphertext, uint32_t crib_offset);
