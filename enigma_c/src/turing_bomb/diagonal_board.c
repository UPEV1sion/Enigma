#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "helper/helper.h"
#include "turing_bomb.h"
#include "diagonal_board.h"

//
// Created by Emanuel on 30.08.2024.
//

#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVXYZ"

#define ERR_NO_CYCLES_FOUND 1

void create_diagonal_board(TuringBomb *restrict turing_bomb)
{
    DiagonalBoard *diagonal_board = turing_bomb->diagonal_board;

    for (int i = 0; i < ALPHABET_SIZE; ++i)
    {
        diagonal_board->terminals[i][i] = -1;
    }
}

int32_t create_bomb_menu(TuringBomb *restrict turing_bomb, const CycleCribCipher *restrict cycle)
{
    create_diagonal_board(turing_bomb);

    return 0;
}
