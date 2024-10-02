#include <stdint.h>
#include <string.h>

#include "turing_bomb.h"
#include "helper/helper.h"
#include "diagonal_board.h"
#include "cycle_finder/cycle_finder.h"
#include "cycle_finder/cycle_finder_graph.h"

//
// Created by Emanuel on 07.09.2024.
//

/* A very useful resource was a YouTube video by Gustav Vogels named
 * TURING WELCHMAN BOMBE - Beschreibung des kompletten Entschlüsselungsverfahrens der ENIGMA.
 * In which explained the inner workings of the bombe in great depth and detail.
 * https://youtu.be/7pOsBhwwmhI
 */

/* This aims to be an "authentic" turing implementation.
 * The turing bomb back in the day, of course, used no software.
 * But this implementation aims to mimic the inner workings of the Turing-Welchman Bomb as close as possible.
 * I've always programmed with performance in mind and made this as fast as possible.
 * The "Hill-climbing" algorithm will possibly be a faster alternative, but this is not the goal of this implementation.
 */

#define NUM_ROTORS             5
#define PLUGBOARD              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

static bool is_valid_crip_position(const char *crib, const char *encrypted_text, const uint32_t crib_pos)
{
    const size_t crip_len = strlen(crib);
    if (crip_len + crib_pos > strlen(encrypted_text)) return false;
    for (size_t i = 0; i < crip_len; ++i)
    {
        if (*(encrypted_text + crib_pos + i) == *(crib + i)) return false;
    }

    return true;
}

static void set_starting_pos_scramblers(TuringBomb *restrict turing_bomb,
                                        const CycleCribCipher *restrict cycle,
                                        const enum ROTOR_TYPE rotor_one_type,
                                        const enum ROTOR_TYPE rotor_two_type,
                                        const enum ROTOR_TYPE rotor_three_type)
{
    // const uint8_t bound = cycle->len_w_stubs < NUM_SCRAMBLERS_PER_ROW
    //                           ? cycle->len_w_stubs
    //                           : cycle->len_wo_stubs;

    // const uint8_t *cycle_pos = cycle->len_w_stubs < NUM_SCRAMBLERS_PER_ROW
    //                                ? cycle->positions_w_stubs
    //                                : cycle->positions_wo_stubs;

    // I haven't found a good way yet to denote that a position is part of a stub. Maybe a bitmask will do the trick
    const uint8_t bound      = cycle->len_wo_stubs;
    const uint8_t *cycle_pos = cycle->positions_wo_stubs;

    for (uint8_t column = 0; column < bound; ++column)
    {
        // Rotors work with 1 off
        turing_bomb->bomb_row[column].rotors[0] = create_rotor_by_type(rotor_one_type, cycle_pos[column] + 1, 1);
        turing_bomb->bomb_row[column].rotors[1] = create_rotor_by_type(rotor_two_type, 1, 1);
        turing_bomb->bomb_row[column].rotors[2] = create_rotor_by_type(rotor_three_type, 1, 1);
    }
}

CycleCribCipher* find_longest_cycle(const CyclesCribCipher *cycles)
{
    CycleCribCipher *cycle = cycles->cycles_positions[0];
    for (uint8_t i = 1; i < cycles->num_cycles; ++i)
    {
        if (cycles->cycles_positions[i]->len_wo_stubs > cycle->len_wo_stubs
            && cycles->cycles_positions[i]->len_wo_stubs <= NUM_SCRAMBLERS_PER_ROW)
        {
            cycle = cycles->cycles_positions[i];
        }
    }

    return cycle;
}

void traverse_rotor_column(Rotor **rotor_column)
{

}

int32_t traverse_rotor_conf(TuringBomb *turing_bomb)
{
    for (uint8_t column = 0; column < turing_bomb->scrambler_columns_used; ++column)
    {



    }

    return 1;
}


int32_t start_turing_bomb(const char *restrict crib, const char *restrict ciphertext, const uint32_t crib_offset)
{
    if (!is_valid_crip_position(crib, ciphertext, crib_offset)) return 1;

    const size_t crib_len = strlen(crib);


    if (crib_offset + crib_len > strlen(ciphertext))
    {
        fprintf(stderr, "Plain outside crib\n");
        return 1;
    }

    DiagonalBoard diagonal_board = {0};
    TuringBomb turing_bomb       = {.diagonal_board = &diagonal_board};
    CycleCribCipher *cycle     = find_best_cycle_graph(crib, ciphertext);

    if (cycle == NULL)
    {
        fprintf(stderr, "No cycles found\n");
        return 1;
    }

    // TODO look set_starting_pos_scramblers
    turing_bomb.scrambler_columns_used = cycle->len_wo_stubs;

    // TODO hook up diagonal board
    create_bomb_menu(&turing_bomb, cycle);

    // Different rotor types
    // 60 * 26 * 26 * 26 = 1054560 Permutations
    uint32_t ret_val = 1;

    for (uint8_t rotor_one_type = 1; rotor_one_type <= NUM_ROTORS; ++rotor_one_type)
    {
        for (uint8_t rotor_two_type = 1; rotor_two_type <= NUM_ROTORS; ++rotor_two_type)
        {
            if (rotor_one_type == rotor_two_type)
            {
                continue;
            }
            for (uint8_t rotor_three_type = 1; rotor_three_type <= NUM_ROTORS; ++rotor_three_type)
            {
                if (rotor_one_type == rotor_three_type || rotor_two_type == rotor_three_type)
                {
                    continue;
                }
                // Should be setup correctly now
                set_starting_pos_scramblers(&turing_bomb, cycle, rotor_one_type, rotor_two_type, rotor_three_type);
                ret_val |= traverse_rotor_conf(&turing_bomb);
            }
        }
    }

    free(cycle);

    return ret_val;
}

