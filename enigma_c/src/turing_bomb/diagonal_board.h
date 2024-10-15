#pragma once

//
// Created by Emanuel on 30.08.2024.
//

/* This is the Diagonal Board.
 * It detected the cycles/patters between crib and ciphertext by chaining the rotors together.
 * It consisted out of 26 cables á 26 wires.
 * There were 26 contacts at the back of the bombe, that where energized at all time.
 * Each cable/cable represented a letter in the alphabet.
 *
 * If our cycle started with the letter 'A' at position 3,
 * and the next letter in the letter was e.g. an 'E' at position 5,
 * our top scrambler at the top row had the position 1 and the next 3.
 * We set up the scrambler position in such a way that they match the "offset" between the letters in the loop.
 *
 * Now we connect the rotors together.
 * We connect a cable from the starting letter of the loop to the input of rotor 1, out 1 → in 2, and so on.
 *
 * If we integrate "stubs" in the cycle, we will need the common connectors.
 * If a position contributes to the cycle and a stub arises from it, we need the common connectors or commons.
 * Each rotor has one input and one output, so we can't plug 2 connections in.
 * The commons typically had five contacts, so up to four branches per position where feasible.
 * One cable was needed for connecting the branches to the rotor itself.
 * We would simply plug all cables where the branches happen in the same common field.
 *
 */

#include "enigma/rotor/rotor.h"
#include "diagonal_board.h"
#include "cycle_finder/cycle_finder.h"

#define NUM_SCRAMBLERS_PER_COLUMN   3
#define MAX_CONTACTS_PER_COMMON     5
#define MAX_NUM_COMMONS             5

typedef struct Contact Contact;
typedef struct TuringBomb TuringBomb;

// TODO last input char in rotor column
// This has in and outputs, so leave it here.
typedef struct ScramblerEnigma
{
    Contact *in, *out;
    Rotor *rotors[NUM_SCRAMBLERS_PER_COLUMN];
} ScramblerEnigma;

struct Contact
{
    uint8_t active_cable_connections[ALPHABET_SIZE];

//    Contact *commons;
    uint8_t num_active_connections;
//    uint8_t num_common_connections;
    uint8_t contact_num;
};

typedef struct
{
    Contact *test_reg;
    uint8_t terminal_num, wire_num;
} TestRegister;

typedef struct
{
    Contact *contacts[ALPHABET_SIZE];
    TestRegister *test_register;
    uint8_t num_commons;
} Terminal;
