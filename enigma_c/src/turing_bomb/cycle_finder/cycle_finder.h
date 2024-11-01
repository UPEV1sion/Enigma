#pragma once

//
// Created by Emanuel on 01.11.2024.
//

#include <stdint.h>
#include <stdbool.h>

#include "turing_bomb/turing_bomb.h"

typedef struct MenuNode MenuNode;

struct MenuNode
{
    MenuNode *next;
    char crib_char;
    char cipher_char;
    uint8_t position;
    bool is_cycle;
    bool is_stub;
} ;

typedef struct
{
    MenuNode *relations[ALPHABET_SIZE];
    MenuNode *all_menu_nodes[MAX_CRIB_LEN];
    uint8_t len_all_menu_nodes;
} MenuGraph;

typedef struct
{
    //TODO maybe linked list
    MenuNode *nodes_with_stubs;
    MenuNode *nodes_without_stubs;
    uint8_t len_with_stubs;
    uint8_t len_without_stubs;
    MenuGraph graph;
} Menu;

Menu* find_longest_cycle(const char *crib, const char *ciphertext);
