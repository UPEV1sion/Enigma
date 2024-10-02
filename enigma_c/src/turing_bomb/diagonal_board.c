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
// All letters turned on.
#define ALPHABET_BITMASK 0xFFFFFFF

#define ERR_NO_CYCLES_FOUND 1


int32_t create_bomb_menu(TuringBomb *restrict turing_bomb, const CycleCribCipher *restrict cycle)
{
    turing_bomb->diagonal_board->alphabet[cycle->chars_wo_stubs[0]] = ALPHABET_BITMASK;
    for (uint8_t column = 1; column < cycle->len_wo_stubs; ++column)
    {
        // turing_bomb->bomb_row[column].in
    }

    return 0;
}
