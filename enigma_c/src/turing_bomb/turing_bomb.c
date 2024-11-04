#include <stdint.h>
#include <string.h>

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
#define ERR_CYCLE              2

// TODO count these
#define MAX_CONTACTS_PER_COMMON     5
#define MAX_NUM_COMMONS             5

typedef struct BombeNode BombeNode;

typedef struct
{
    uint8_t active_cable_connections[ALPHABET_SIZE];
    uint32_t active_contacts;
    //    Contact *commons;
    uint8_t num_active_connections;
    //    uint8_t num_common_connections;
    uint8_t contact_num;
} Contact;

typedef struct
{
    Contact *contacts[ALPHABET_SIZE];
    Contact *test_register;
    uint8_t num_commons;
} Terminal;

typedef struct ScramblerEnigma
{
    Contact *in, *out;
    Rotor *rotors[NUM_SCRAMBLERS_PER_COLUMN];
} ScramblerEnigma;

typedef struct TuringBomb
{
//    ScramblerEnigma bomb_row[NUM_SCRAMBLERS_PER_ROW];
    // This doesn't represent the scrambler order but the scramblers connected to the test register and so forth.
    // As I needed a way to efficiently traverse the bombe and simulate current.
    BombeNode *starting_node;
    Terminal *terminal;
    Reflector *reflector;
    uint8_t scrambler_columns_used;
} TuringBomb;

struct BombeNode
{
  ScramblerEnigma *scrambler_enigma;
  BombeNode *neighbours;
  uint8_t neighbour_count;
  Contact *contact;
};

static void free_bomb(TuringBomb *turing_bomb)
{
    free(turing_bomb->reflector);
    //TODO
}

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

//static void print_all_active_contacts(const TuringBomb *turing_bomb)
//{
//    for (uint8_t i = 0; i < turing_bomb->scrambler_columns_used; ++i)
//    {
//        const Contact *contact =  turing_bomb->bomb_row[i].in;
//        printf("Contact No. %d : %d\n", contact->contact_num, contact->num_active_connections);
//        for (int j = 0; j < contact->num_active_connections; ++j)
//        {
//            printf("%d ", contact->active_cable_connections[j]);
//        }
//        puts("");
//    }
//}

static void print_contact_status(const Contact *contact, const char *contact_name)
{
    printf("%15s. %2d : ", contact_name, contact->contact_num);
    for (uint8_t i = 0; i < contact->num_active_connections; ++i)
    {
        printf("%d ", contact->active_cable_connections[i]);
    }
    puts("");
}

//static void print_current_configuration(const TuringBomb *turing_bomb)
//{
//    for (uint8_t column = 0; column < turing_bomb->scrambler_columns_used; ++column)
//    {
//        for (int row = 0; row < NUM_SCRAMBLERS_PER_COLUMN; ++row)
//        {
//            const Rotor *rotor = turing_bomb->bomb_row[column].rotors[row];
//            printf("Rotor %d. %2d\n", row + 1, rotor->position);
//        }
//    }
//}

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

    rotor_one->position++;
    if(rotor_one->position >= 26)
    {
        rotor_one->position = rotor_one->position % 26;
        rotor_two->position++;
        if(rotor_two->position >= 26)
        {
            rotor_two->position = rotor_two->position % 26;
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
            output_contact->active_cable_connections[output_contact->num_active_connections] = character;
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

static uint8_t find_most_frequent_menu_pos(Menu *menu)
{
    CribCipherTuple *most_freq_tuple = menu->cycle;
    uint8_t most_freq_pos = 0;

    for(uint8_t tuple = 1; tuple < menu->len_cycle; ++tuple)
    {
        CribCipherTuple *current_tuple = menu->cycle + tuple;

        if(current_tuple->first.num_stubs > most_freq_tuple->first.num_stubs)
        {
            most_freq_tuple = current_tuple;
            most_freq_pos   = tuple;
        }
    }

    return most_freq_pos;
}

static void setup_test_register(Menu *menu, CribCipherTuple *most_freq_pos, TuringBomb *restrict turing_bomb)
{
    const uint8_t terminal_i = most_freq_pos->first.letter - 'A';
    turing_bomb->terminal->test_register = turing_bomb->terminal->contacts[terminal_i];
    Contact *test_reg = turing_bomb->terminal->test_register;

    test_reg->active_cable_connections[0] = 0; //Test the letter "A"
    turing_bomb->terminal->contacts[0]->active_cable_connections[0] = test_reg->contact_num; //Contact connected through the diag. board
    //TODO ringspeicher node setup
}

static int32_t setup_turing_bomb(const char *restrict crib,
                                 const char *restrict ciphertext,
                                 TuringBomb *restrict turing_bomb,
                                 BombeNode *bomb_nodes)
{
    Menu *menu = find_longest_menu(crib, ciphertext);
    if (menu == NULL)
    {
        free_bomb(turing_bomb);
        return ERR_CYCLE;
    }
    const uint8_t most_freq_pos = find_most_frequent_menu_pos(menu);

    setup_test_register(menu, menu->cycle + most_freq_pos, turing_bomb);



    free_menu(menu);
    return 0;
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
//    terminal.test_register = &test_reg;
    Reflector *reflector   = create_reflector_by_type(UKW_B);
    BombeNode nodes[MAX_CRIB_LEN];
    TuringBomb turing_bomb = {.terminal = &terminal, .reflector = reflector};

    int32_t err_code = setup_turing_bomb(crib, ciphertext, &turing_bomb, nodes);
    if(err_code != 0) return err_code;

//    Cycle *cycle = find_longest_cycle_graph(crib, ciphertext);
    // setup_test_register(&turing_bomb, cycle);

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
                //TODO rewrite
//                setup_scramblers(&turing_bomb, cycle, rotor_one_type, rotor_two_type, rotor_three_type);
//                ret_val |= traverse_rotor_conf(&turing_bomb);
            }
        }
    }

    free(turing_bomb.reflector);

    return ret_val;
}
