#include <stdint.h>
#include <string.h>

#include "turing_bomb.h"
#include "helper/helper.h"
#include "diagonal_board.h"
#include "cycle_finder/cycle_finder.h"

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
 * The "Hill-climbing" will possibly be a faster alternative, but this is not the goal of this implementation.
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
                                        const Cycle *restrict cycle,
                                        const enum ROTOR_TYPE rotor_one_type,
                                        const enum ROTOR_TYPE rotor_two_type,
                                        const enum ROTOR_TYPE rotor_three_type)
{

    Rotor *rotors[NUM_SCRAMBLERS_PER_ROW] = {
        create_rotor_by_type(rotor_one_type, 0, 0),
        create_rotor_by_type(rotor_two_type, 0, 0),
        create_rotor_by_type(rotor_three_type, 0, 0)
    };

    ScramblerEnigma scrambler_row = {
        .in = 0,
        .out = 0
    };

    memcpy(scrambler_row.rotors, rotors, sizeof scrambler_row.rotors);

    for (uint8_t column = 0; column < NUM_SCRAMBLERS_PER_ROW; ++column)
    {
        memcpy(turing_bomb->bomb_row + column, &scrambler_row, sizeof scrambler_row);
    }

    const uint8_t bound = cycle->len_w_stubs < NUM_SCRAMBLERS_PER_ROW
                              ? cycle->len_w_stubs
                              : cycle->len_wo_stubs;

    const uint8_t *cycle_pos = cycle->len_w_stubs < NUM_SCRAMBLERS_PER_ROW
                                   ? cycle->pos_cycle_w_stubs
                                   : cycle->pos_cycle_wo_stubs;

    for (uint8_t column = 0; column < bound; ++column)
    {
        turing_bomb->bomb_row[column].rotors[0]->position = cycle_pos[column];
    }

    for (uint8_t i = 0; i < NUM_SCRAMBLERS_PER_ROW; ++i)
    {
        free(rotors[i]);
    }
}


int32_t start_turing_bomb(const char *restrict crib, const char *restrict ciphertext, const uint32_t crib_pos)
{
    if (!is_valid_crip_position(crib, ciphertext, crib_pos)) return 1;

    const size_t crib_len = strlen(crib);

    if (crib_len > NUM_SCRAMBLERS_PER_ROW)
    {
        fprintf(stderr, "Try a shorter crib\n");
        return 1;
    }

    if (crib_pos + crib_len > strlen(ciphertext))
    {
        fprintf(stderr, "Plain outside crib\n");
        return 1;
    }

    uint8_t *crib_as_ints       = get_int_array_from_string(crib);
    uint8_t *ciphertext_as_ints = get_int_array_from_string(ciphertext);


    TuringBomb turing_bomb = {0};
    const Cycle *cycle     = find_cycles(crib, ciphertext);

    create_bomb_menu(turing_bomb.diagonal_board, crib_as_ints, ciphertext_as_ints, crib_len);

    // Different rotor types
    // 60 * 26 * 26 * 26 = 1054560 Permutations
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

                set_starting_pos_scramblers(&turing_bomb, cycle, rotor_one_type, rotor_two_type, rotor_three_type);

            }
        }
    }
    free(crib_as_ints);
    free(ciphertext_as_ints);

    return 1;
}


/*
 EnigmaConfiguration conf                     = {
    //     .rotors = rotors, .rotor_positions = rotor_positions, .ring_settings = ring_settings,
    //     .type = ENIGMA_M3, .reflector = UKW_B, .message = crib
    // };
    // memcpy(conf.plugboard, PLUGBOARD, sizeof(PLUGBOARD));
    // uint8_t *output = traverse_m3_enigma_at_position(&conf, crib_pos, crib_len);

    const Cycles *current_cycles = find_cycles(crib_as_ints, output, crib_len);


    if (passes_welchman_test(candidate_cycles, current_cycles))
    {
        printf("Possible match found: ");
        printf("%d : %d : %d at %d : %d : %d", conf.rotors[0], conf.rotors[1], conf.rotors[2],
               conf.rotor_positions[0], conf.rotor_positions[1], conf.rotors[2]);
        return 0;
    }
    free(output);

*/
