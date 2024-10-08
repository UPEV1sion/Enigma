#include <stdint.h>
#include <string.h>

#ifdef _MSC_VER
#include <intrin.h> //MSVC intrinsics again
#endif

#include "turing_bomb.h"
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
 */

#define NUM_ROTORS             5
#define PLUGBOARD              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define ERR_INVALID_CRIB       1
#define ERR_NO_CYCLES_FOUND    2

static bool is_valid_crip_position(const char *crib, const char *ciphertext, const uint32_t crib_pos)
{
    const size_t crip_len = strlen(crib);
    if (crip_len + crib_pos > strlen(ciphertext))
    {
        fprintf(stderr, "Plain outside crib\n");
        return false;
    }
    for (size_t i = 0; i < crip_len; ++i)
    {
        if (ciphertext[crib_pos + i] == crib[i])
        {
            fprintf(stderr, "Chars can't be depicted onto itself\n");
            return false;
        }
    }

    return true;
}

static void setup_scramblers(TuringBomb *restrict turing_bomb,
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
    const uint8_t bound = cycle->len_wo_stubs;
    const uint8_t *cycle_pos = cycle->positions_wo_stubs;

    for (uint8_t column = 0; column < bound; ++column)
    {
        // Rotors work with 1 off.
        // The bottom rotor at the turing bomb, although rotating the slowest,
        // corresponded to the rightmost right enigma rotor
        turing_bomb->bomb_row[column].rotors[0] = create_rotor_by_type(rotor_one_type, 1, 1);
        turing_bomb->bomb_row[column].rotors[1] = create_rotor_by_type(rotor_two_type, 1, 1);
        turing_bomb->bomb_row[column].rotors[2] = create_rotor_by_type(rotor_three_type, cycle_pos[column] + 1, 1);
    }
}

static uint8_t traverse_rotor_column(Rotor **rotor_column, Reflector *reflector, uint8_t input_letter)
{

    uint8_t character;
    Rotor *rotor_one = rotor_column[0];
    Rotor *rotor_two = rotor_column[1];
    Rotor *rotor_three = rotor_column[2];

    rotor_one->position = (rotor_one->position + 1) % 26;

    if (should_rotate(rotor_one))
    {
        rotor_two->position = (rotor_two->position + 1) % 26;

        if (should_rotate(rotor_two))
        {
            rotor_three->position = (rotor_three->position + 1) % 26;
        }
    }

    character = traverse_rotor(rotor_one, input_letter);
    character = traverse_rotor(rotor_two, character);
    character = traverse_rotor(rotor_three, character);
    character = reflector->wiring[character];
    character = traverse_rotor_inverse(rotor_three, character);
    character = traverse_rotor_inverse(rotor_two, character);
    character = traverse_rotor_inverse(rotor_one, character);

    return character;
}

static int32_t traverse_rotor_conf(TuringBomb *turing_bomb)
{
    TestRegister *test_reg = turing_bomb->terminal->test_register;
    uint8_t input_letter = test_reg->wire_num;
    ScramblerEnigma *current_column = turing_bomb->bomb_row + test_reg->terminal_num;
    Reflector *reflector = turing_bomb->reflector;
    Contact **contacts = turing_bomb->terminal->contacts;

    while(test_reg->active_wires != 1 && test_reg->active_wires != 25)
    {
        //TODO connect out
        current_column->out->contact |= (1 << input_letter);
        contacts[input_letter]->contact |= (1 << current_column->in->contact_num);
        input_letter = traverse_rotor_column(
                current_column->rotors,
                reflector,
                input_letter);
        test_reg->active_wires = POPCNT(test_reg->test_reg->contact);
//        current_column = current_column->

    }

    return 1;
}

static uint8_t find_test_register_pos(const CycleCribCipher *restrict cycle)
{
    // A very present letter in the cycle, but not right next to the input
    //TODO support stubs
    uint8_t most_freq_pos = 0;
    for (uint8_t cycle_pos = 1; cycle_pos < cycle->len_wo_stubs - 2; ++cycle_pos)
    {
        if (cycle->positions_wo_stubs[cycle_pos] > most_freq_pos)
            most_freq_pos = cycle_pos;
    }

    return most_freq_pos;
}

static void setup_test_register(TuringBomb *restrict turing_bomb, const CycleCribCipher *restrict cycle)
{
//    create_diagonal_board(turing_bomb);
    const uint8_t most_freq_pos = find_test_register_pos(cycle);

    const uint8_t test_reg_letter = cycle->chars_wo_stubs[most_freq_pos] - 'A';
    const uint8_t test_reg_wire_letter = cycle->chars_wo_stubs[most_freq_pos + 1] - 'A';

    Contact *test_reg_contact;
    TestRegister *test_reg = turing_bomb->terminal->test_register;
    Contact **contacts = turing_bomb->terminal->contacts;

    test_reg_contact = contacts[test_reg_letter];
    test_reg->test_reg = test_reg_contact;
    test_reg_contact->active = true;
    test_reg_contact->contact = (1 << test_reg_wire_letter);
    test_reg->terminal_num = test_reg_letter;
    test_reg->wire_num = test_reg_wire_letter;
    // Commutative properties of the diagonal board
    contacts[test_reg_wire_letter]->contact = (1 << test_reg_letter);
}


int32_t start_turing_bomb(const char *restrict crib, const char *restrict ciphertext, const uint32_t crib_offset)
{
    if (!is_valid_crip_position(crib, ciphertext, crib_offset)) return ERR_INVALID_CRIB;

    Contact contacts[ALPHABET_SIZE] = {0};
    Terminal terminal = {0};
    TestRegister test_reg = {0};
    for (uint8_t contact = 0; contact < ALPHABET_SIZE; ++contact)
    {
        terminal.contacts[contact] = &contacts[contact];
    }
    terminal.test_register = &test_reg;
    Reflector *reflector = create_reflector_by_type(UKW_B);
    TuringBomb turing_bomb = {.terminal = &terminal, .reflector = reflector};


    CycleCribCipher *cycle = find_best_cycle_graph(crib, ciphertext);

    if (cycle == NULL)
    {
        fprintf(stderr, "No cycles found\n");
        return ERR_NO_CYCLES_FOUND;
    }

    // TODO look setup_scramblers
    turing_bomb.scrambler_columns_used = cycle->len_wo_stubs;

    setup_test_register(&turing_bomb, cycle);
    //TODO start traversing

    // Different rotor types
    // 60 * 26 * 26 * 26 = 1054560 Permutations
    int32_t ret_val = 1;

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
                setup_scramblers(&turing_bomb, cycle, rotor_one_type, rotor_two_type, rotor_three_type);
                ret_val |= traverse_rotor_conf(&turing_bomb);
            }
        }
    }

    free(cycle);

    return ret_val;
}
