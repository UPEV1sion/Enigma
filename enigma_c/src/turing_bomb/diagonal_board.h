#pragma once

//
// Created by Emanuel on 30.08.2024.
//

/* This is the Diagonal Board.
 * It detected the cycles/patters between crib and ciphertext by chaining the rotors together.
 * It consisted out of 26 cables á 26 wires.
 * There were 26 contacts at the back of the bombe, that where energized at all time.
 * Each cable/contact represented a letter in the alphabet.
 *
 *
 * If our cycle started with the letter 'A' at position 3,
 * TODO rewrite
 * we would take a cable and plug one side into 'A' and the other into rotor 3 in.
 *
 * If the next letter in to loop was e.g. an 'E' at position 8,
 * we take one cable and plug one side into rotor 3 out and the other one into rotor 8 in.
 * We proceed until the loop is finished.
 *
 * The last step is to determine a "Test Register".
 * This was a letter in the loop that couldn't be starting letter or right next to it.
 * If a candidate was found, i.e., the cycle was closed;
 * the electrical connection was also closed, the turing bombe stopped.
 */

#include <stdbool.h>

#include "helper/helper.h"
#include "enigma/rotor/rotor.h"

#define NUM_SCRAMBLERS_PER_COLUMN 3
#define NUM_FIELDS_PER_COMMON 5
#define NUM_COMMONS 6


// 1 bit for each letter → 26 bits.
typedef uint32_t cable_t;

// I put this there because the in and out connectors where introduced with the Diagonal Board.
typedef struct ScramblerEnigma
{
    cable_t in, out;
    Rotor *rotors[NUM_SCRAMBLERS_PER_COLUMN];
} ScramblerEnigma;


/* This is the so-called common connections.
 * If a cycle had a branch, e.g., a stub at a certain point in the cycle,
 * one connection at that letter wouldn't be enough.
 * This problem was solved with the common connectors, which typically allowed up to 5 connections per letter.
 */
typedef struct
{
    cable_t common[NUM_FIELDS_PER_COMMON];
} CommonConnections;


typedef struct DiagonalBoard
{
    cable_t alphabet[ALPHABET_SIZE];
    CommonConnections commons[NUM_COMMONS];
} DiagonalBoard;

int32_t create_bomb_menu(DiagonalBoard *diagonal_board, const uint8_t *crib, const uint8_t *ciphertext, size_t crib_len);
