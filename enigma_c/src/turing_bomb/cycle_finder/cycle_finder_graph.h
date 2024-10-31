#pragma once

//
// Created by Emanuel on 29.09.2024.
//

#include <stdint.h>

#include "cycle_finder.h"
#include "helper/helper.h"
#include "turing_bomb/turing_bomb.h"

typedef struct
{
    char chars_w_stubs[ALPHABET_SIZE];
    char chars_wo_stubs[ALPHABET_SIZE];
    uint8_t positions_w_stubs[ALPHABET_SIZE]; // with / without
    uint8_t positions_wo_stubs[ALPHABET_SIZE];
    uint8_t len_w_stubs;
    uint8_t len_wo_stubs;
} CyclePositions;

typedef struct
{
    // TODO store them as uint8_t
    char crib_char, cipher_char;
    uint8_t position, cycle_position;
    bool visited_cycle;
    bool visited_stub;
} Node;

typedef struct
{
    Node *relations[ALPHABET_SIZE][MAX_CRIB_LEN];
    uint8_t nodes_per_letter[ALPHABET_SIZE];
} Graph;

typedef struct
{
    CyclePositions *positions;
    Graph *graph;

} Cycle;

Cycle* find_longest_cycle_graph(const char *restrict crib, const char *restrict ciphertext);
void free_cycle(Cycle *cycle);
