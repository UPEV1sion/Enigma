#pragma once

//
// Created by Emanuel on 01.11.2024.
//

#include <stdint.h>
#include <stdbool.h>

#include "turing_bombe/turing_bombe.h"

typedef struct MenuNode MenuNode;
typedef struct CribCipherTuple CribCipherTuple;

struct MenuNode
{
    CribCipherTuple **stubs;
    uint8_t num_stubs;
    char letter;
};

struct CribCipherTuple
{
    MenuNode first;
    MenuNode second;
    uint8_t position;
    bool visited;
};

typedef struct
{
    CribCipherTuple *menu;
    uint8_t len_menu;
} Menu;

Menu* find_longest_menu(const char *crib, const char *ciphertext);
void free_menu(Menu *menu);
