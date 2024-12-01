//
// Created by escha on 01.12.24.
//

#pragma once

#include <stdint.h>

typedef struct MNode MNode;

struct MNode
{
    char letter;
    uint8_t position;
    MNode *next;
    MNode **stubs;
    uint8_t num_stubs;
};

typedef struct
{
    MNode *nodes;
    uint8_t len_menu;
    uint8_t num_stubs;
} MenuGraph;

MenuGraph* find_menu(const char *restrict crib, const char *ciphertext, size_t offset);
