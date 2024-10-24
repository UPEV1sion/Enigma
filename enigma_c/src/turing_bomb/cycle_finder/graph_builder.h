#pragma once

//
// Created by Emanuel on 22.10.24.
//

#include <stdbool.h>
#include <stdint.h>

#include "helper/helper.h"
#include "turing_bomb/turing_bomb.h"

typedef struct Node Node;

struct Node
{
    char crib_char, cipher_char;
    uint8_t position;
    bool visited;
};

typedef struct
{
    Node *relations[ALPHABET_SIZE][MAX_CRIB_LEN];
    uint8_t nodes_per_letter[ALPHABET_SIZE];
} Graph;


Graph* build_best_scrambler_graph(const char *crib, const char *ciphertext, TuringBomb *turing_bomb);
