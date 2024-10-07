#include <stdint.h>

#include "helper/helper.h"
#include "turing_bomb.h"
#include "diagonal_board.h"

//
// Created by Emanuel on 30.08.2024.
//


#define ERR_NO_CYCLES_FOUND 1


void create_diagonal_board(TuringBomb *restrict turing_bomb)
{
    for (int i = 0; i < ALPHABET_SIZE; ++i)
    {
        // diagonal_board->terminals[i][i] = -1;
    }
}

static uint8_t find_test_register_pos(TuringBomb *restrict turing_bomb, const CycleCribCipher *restrict cycle)
{
    // A very present letter in the cycle, but not right next to the input
    //TODO support stubs
    uint8_t most_freq_pos = 0;
    for (uint8_t cycle_pos = 1; cycle_pos < cycle->len_wo_stubs-1; ++cycle_pos)
    {
        if(cycle->positions_wo_stubs[cycle_pos] > most_freq_pos)
            most_freq_pos = cycle->positions_wo_stubs[cycle_pos];
    }

    return most_freq_pos;
}

int32_t create_bomb_menu(TuringBomb *restrict turing_bomb, const CycleCribCipher *restrict cycle)
{
    create_diagonal_board(turing_bomb);

    return 0;
}
