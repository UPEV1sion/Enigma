#include <stdint.h>
#include <string.h>

//#ifdef _MSC_VER
//#include <intrin.h> //MSVC intrinsics again
//#endif

#include "turing_bomb.h"
#include "cycle_finder/cycle_finder.h"
#include "turing_bomb/cycle_finder/cycle_finder_graph.h"

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
 * But this implementation aims to mimic the inner workings of the Turing-Welchman Bomb.
 */

#define NUM_ROTORS             5
#define PLUGBOARD              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define ERR_INVALID_CRIB       1
#define ERR_NO_CYCLES_FOUND    2

// TODO count these
#define MAX_CONTACTS_PER_COMMON     5
#define MAX_NUM_COMMONS             5

////TODO really needed?
//typedef struct
//{
//    Contact *test_reg;
//    uint8_t terminal_num, wire_num;
//} TestRegister;


static bool is_valid_crip_position(const char *crib, const char *ciphertext, const uint32_t crib_pos)
{
    const size_t crib_len = strlen(crib);
    if (crib_len + crib_pos > strlen(ciphertext))
    {
        fprintf(stderr, "Plain outside crib\n");
        return false;
    }
    for (size_t i = 0; i < crib_len; ++i)
    {
        if (ciphertext[crib_pos + i] == crib[i])
        {
            fprintf(stderr, "Chars can't be depicted onto itself\n");
            return false;
        }
    }

    return true;
}

static void print_all_active_contacts(const TuringBomb *turing_bomb)
{
    for (uint8_t i = 0; i < turing_bomb->scrambler_columns_used; ++i)
    {
        const Contact *contact =  turing_bomb->bomb_row[i].in;
        printf("Contact No. %d : %d\n", contact->contact_num, contact->num_active_connections);
        for (int j = 0; j < contact->num_active_connections; ++j)
        {
            printf("%d ", contact->active_cable_connections[j]);
        }
        puts("");
    }
}

static void print_contact_status(const Contact *contact, const char *contact_name)
{
    printf("%15s. %2d : ", contact_name, contact->contact_num);
    for (uint8_t i = 0; i < contact->num_active_connections; ++i)
    {
        printf("%d ", contact->active_cable_connections[i]);
    }
    puts("");
}

static void print_current_configuration(const TuringBomb *turing_bomb)
{
    for (uint8_t column = 0; column < turing_bomb->scrambler_columns_used; ++column)
    {
        for (int row = 0; row < NUM_SCRAMBLERS_PER_COLUMN; ++row)
        {
            const Rotor *rotor = turing_bomb->bomb_row[column].rotors[row];
            printf("Rotor %d. %2d\n", row + 1, rotor->position);
        }
    }
}

static void traverse_rotor_column(const Reflector *reflector,
                                  const ScramblerEnigma *current_column,
                                  Contact **restrict contacts)
{

    //TODO clear contacts that are not active anymore
    Rotor *rotor_one   = current_column->rotors[0];
    Rotor *rotor_two   = current_column->rotors[1];
    Rotor *rotor_three = current_column->rotors[2];

    const Contact *input_contact = current_column->in;
    Contact *output_contact      = current_column->out;

    rotor_one->position = (rotor_one->position + 1) % 26;

    if (should_rotate(rotor_one))
    {
        rotor_two->position = (rotor_two->position + 1) % 26;

        if (should_rotate(rotor_two))
        {
            rotor_three->position = (rotor_three->position + 1) % 26;
        }
    }

    uint8_t letter_num;
    for (letter_num = 0; letter_num < input_contact->num_active_connections; ++letter_num)
    {
        uint8_t character = input_contact->active_cable_connections[letter_num];
        character         = traverse_rotor(rotor_one, character);
        character         = traverse_rotor(rotor_two, character);
        character         = traverse_rotor(rotor_three, character);
        character         = reflector->wiring[character];
        character         = traverse_rotor_inverse(rotor_three, character);
        character         = traverse_rotor_inverse(rotor_two, character);
        character         = traverse_rotor_inverse(rotor_one, character);

        if((output_contact->active_contacts & (1 << character)) == 0)
        {
            output_contact->active_cable_connections[output_contact->num_active_connections]     = character;
            output_contact->num_active_connections++;
            if(output_contact->num_active_connections == 26) return;
            output_contact->active_contacts |= (1 << character);
        }
        Contact *diagonal_contact = contacts[character];
        if((diagonal_contact->active_contacts & (1 << letter_num)) == 0)
        {
            diagonal_contact->active_cable_connections[diagonal_contact->num_active_connections] = letter_num;
            diagonal_contact->num_active_connections++;
            if(diagonal_contact->num_active_connections == 26) return;
            diagonal_contact->active_contacts |= (1 << letter_num);
        }
        print_contact_status(input_contact, "in cont");
        print_contact_status(output_contact, "out cont");
        print_contact_status(diagonal_contact, "diag. cont");
        puts("");
    }
    // puts("");

    // output_contact->num_active_connections = letter_num;
}

static void setup_scramblers(TuringBomb *restrict turing_bomb,
                             const CycleCribCipher *cycle,
                             const enum ROTOR_TYPE rotor_one_type,
                             const enum ROTOR_TYPE rotor_two_type,
                             const enum ROTOR_TYPE rotor_three_type)
{
    // const uint8_t bound = cycle->len_w_stubs < NUM_SCRAMBLERS_PER_ROW
    //                           ?  cycle->len_w_stubs
    //                           : cycle->len_wo_stubs;

    // const uint8_t *cycle_pos = cycle->len_w_stubs < NUM_SCRAMBLERS_PER_ROW
    //                                ? cycle->positions_w_stubs
    //                                : cycle->positions_wo_stubs;

    // I haven't found a good way yet to denote that a position is part of a stub. Maybe a bitmask will do the trick
    const uint8_t bound                 = cycle->len_wo_stubs - 1;
    turing_bomb->scrambler_columns_used = bound;
    //    const uint8_t *cycle_pos            = cycle->positions_wo_stubs;
    const char *cycle_letters = cycle->chars_wo_stubs;
    const uint8_t *cycle_pos  = cycle->positions_wo_stubs;

    ScramblerEnigma *current_column;
    uint8_t current_terminal        = cycle_letters[0] - 'A';
    uint8_t next_terminal;
    Contact *current_contact;
    Contact *next_contact;

    for (uint8_t column = 0; column < bound; ++column)
    {
        // Rotors work with 1 off.
        // The bottom rotor at the turing bomb, although rotating the slowest,
        // corresponded to the rightmost right enigma rotor
        current_column            = turing_bomb->bomb_row + column;
        current_column->rotors[0] = create_rotor_by_type(rotor_one_type, 1, 1);
        current_column->rotors[1] = create_rotor_by_type(rotor_two_type, 1, 1);
        current_column->rotors[2] = create_rotor_by_type(rotor_three_type, cycle_pos[column] + 1, 1);
        next_terminal             = cycle_letters[column + 1] - 'A';
        current_contact           = turing_bomb->terminal->contacts[current_terminal];
        next_contact              = turing_bomb->terminal->contacts[next_terminal];
        current_column->in        = current_contact;
        current_column->out       = next_contact;

        current_terminal = next_terminal;
    }
}

static int32_t traverse_rotor_conf(TuringBomb *turing_bomb)
{
    const Contact *test_reg_contact = turing_bomb->terminal->test_register;
    // uint8_t input_letter       = test_reg->wire_num;
    const Reflector *reflector = turing_bomb->reflector;
    Contact **contacts         = turing_bomb->terminal->contacts;

    uint8_t column_num = test_reg_contact->contact_num;


    for (int i = 0; i < turing_bomb->scrambler_columns_used; ++i)
    {
        const ScramblerEnigma *current_column = turing_bomb->bomb_row + i;
        printf("%d -> %d\n", current_column->in->contact_num, current_column->out->contact_num);
    }

    // In of first contact is already set
    do
    {
        ScramblerEnigma *current_column;
        do
        {
            //FIXME test_reg postion in the contacts isn't the postion in the scramblers.
            current_column = turing_bomb->bomb_row + column_num;
            column_num     = ((column_num + 1) % turing_bomb->scrambler_columns_used);

            //FIXME traversal isn't connect
            traverse_rotor_column(reflector, current_column, contacts);
        } while (column_num != test_reg_contact->contact_num &&
            test_reg_contact->num_active_connections != 26);
        // traverse_rotor_column(reflector, current_column, contacts);
        // print_all_active_contacts(turing_bomb);
        // exit(0);

        printf("Test Reg: %d\n", test_reg_contact->num_active_connections);
    } while (test_reg_contact->num_active_connections != 1 && test_reg_contact->num_active_connections != 26);

    print_current_configuration(turing_bomb);
//    exit(0);

    printf("%d\n", test_reg_contact->num_active_connections);

    return 1;
}

static uint8_t find_test_register_pos(const CycleCribCipher *cycle)
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

static void setup_test_register(const TuringBomb *restrict turing_bomb, const CycleCribCipher *cycle)
{
    //    create_diagonal_board(turing_bomb);
    const uint8_t most_freq_pos = find_test_register_pos(cycle);

    const uint8_t test_reg_letter      = cycle->chars_wo_stubs[most_freq_pos] - 'A';
    const uint8_t test_reg_wire_letter = cycle->chars_wo_stubs[most_freq_pos + 1] - 'A';

    Contact *test_reg_contact;
//    TestRegister *test_reg = turing_bomb->terminal->test_register;
    Contact **contacts     = turing_bomb->terminal->contacts;

    test_reg_contact = contacts[test_reg_letter];
    turing_bomb->terminal->test_register = test_reg_contact;
    test_reg_contact->active_cable_connections[test_reg_contact->num_active_connections] = test_reg_wire_letter;
    test_reg_contact->num_active_connections++;
    test_reg_contact->active_contacts |= (1 << test_reg_wire_letter);
    Contact *diagonal_contact = contacts[test_reg_wire_letter];
    diagonal_contact->active_cable_connections[diagonal_contact->num_active_connections] = test_reg_letter;
    diagonal_contact->num_active_connections++;
    diagonal_contact->active_contacts |= (1 << test_reg_wire_letter);
    //TODO stack or at the corresponding position
    test_reg_contact->active_cable_connections[test_reg_contact->active_contacts] = most_freq_pos;
    test_reg_contact->active_contacts++;
//    test_reg->wire_num = test_reg_wire_letter;
}

static void free_scramblers(TuringBomb *turing_bomb)
{
    for (uint8_t column = 0; column < turing_bomb->scrambler_columns_used; ++column)
    {
        for (uint8_t scrambler = 0; scrambler < NUM_SCRAMBLERS_PER_COLUMN; ++scrambler)
        {
            Rotor *rotor = turing_bomb->bomb_row[column].rotors[scrambler];
            free(rotor->notch);
            free(rotor);
        }
    }
}

int32_t start_turing_bomb(const char *restrict crib, const char *restrict ciphertext, const uint32_t crib_offset)
{
    if (!is_valid_crip_position(crib, ciphertext, crib_offset)) return ERR_INVALID_CRIB;

    Contact contacts[ALPHABET_SIZE] = {0};
    Terminal terminal               = {0};
    for (uint8_t contact = 0; contact < ALPHABET_SIZE; ++contact)
    {
        contacts[contact].contact_num = contact;
        terminal.contacts[contact]    = &contacts[contact];
    }
    //TODO set test reg in setup_test_register
//    terminal.test_register = &test_reg;
    Reflector *reflector   = create_reflector_by_type(UKW_B);
    TuringBomb turing_bomb = {.terminal = &terminal, .reflector = reflector};

    CycleCribCipher *cycle = find_longest_cycle_graph(crib, ciphertext);
    if (cycle == NULL)
    {
        fprintf(stderr, "No cycles found\n");
        return ERR_NO_CYCLES_FOUND;
    }

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
                //TODO reuse rotors?
//                setup_scramblers(&turing_bomb, cycle, rotor_one_type, rotor_two_type, rotor_three_type);
                ret_val |= traverse_rotor_conf(&turing_bomb);
                free_scramblers(&turing_bomb);
            }
        }
    }

//    free(cycle);
    free(turing_bomb.reflector);

    return ret_val;
}
