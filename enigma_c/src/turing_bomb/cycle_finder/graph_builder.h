#pragma once

//
// Created by Emanuel on 22.10.24.
//

#include <stdbool.h>
#include <stdint.h>

#include "helper/helper.h"
#include "turing_bomb/turing_bomb.h"

typedef struct GraphNode GraphNode;

struct GraphNode
{
    char crib_char, cipher_char;
    uint8_t position;
    bool visited;
};

typedef struct
{
    GraphNode *relations[ALPHABET_SIZE][MAX_CRIB_LEN];
    uint8_t nodes_per_letter[ALPHABET_SIZE];
} GraphOfLongestCycle;


GraphOfLongestCycle* build_best_scrambler_graph(const char *crib, const char *ciphertext, TuringBomb *turing_bomb);
