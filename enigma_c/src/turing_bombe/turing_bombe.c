#include <stdint.h>
#include <string.h>
#include <helper/linkedlist.h>
#include <bits/stdint-uintn.h>

#include "turing_bombe.h"
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
 * But this implementation aims to mimic the inner workings of the Turing-Welchman Bomb.
 */

#define NUM_ROTORS             5
#define PLUGBOARD              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#define ERR_INVALID_CRIB       1
#define ERR_CYCLE              2

typedef struct BombeNode BombeNode;

typedef struct
{
    uint8_t active_cable_connections[ALPHABET_SIZE];
    uint32_t active_bit_vector;
    uint8_t num_active_connections;
    uint8_t contact_num;
} Contact;

typedef struct
{
    Contact *contacts[ALPHABET_SIZE];
    Contact *test_register;
    uint8_t num_commons; //TODO remove?
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
} TuringBombe;

struct BombeNode
{
    ScramblerEnigma scrambler_enigma;
    BombeNode **outgoing_commons;
    uint8_t outgoing_commons_count;
//    BombeNode *next_node; //TODO this may need to be a neighbour pointer, pointing back
};

static void free_scrambler(BombeNode *bombe_node)
{
    free(bombe_node->scrambler_enigma.rotors[0]);
    free(bombe_node->scrambler_enigma.rotors[1]);
    free(bombe_node->scrambler_enigma.rotors[2]);
}

static void free_scramblers(TuringBombe *turing_bombe, BombeNode *bombes_nodes)
{
    for (uint8_t scrambler = 0; scrambler < turing_bombe->scrambler_columns_used; ++scrambler)
    {
        free_scrambler(bombes_nodes + scrambler);
    }
}

static void free_bombe(TuringBombe *turing_bombe, BombeNode *bombe_nodes)
{
    free(turing_bombe->reflector);
    free_scramblers(turing_bombe, bombe_nodes);
    for (uint8_t scrambler = 0; scrambler < turing_bombe->scrambler_columns_used; ++scrambler)
    {
        BombeNode *current_node = bombe_nodes + scrambler;
        free(current_node->outgoing_commons);
    }
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

static void print_contact_status(const Contact *contact, const char *contact_name)
{
    printf("%15s. %2d : ", contact_name, contact->contact_num);
    for (uint8_t i = 0; i < contact->num_active_connections; ++i)
    {
        printf("%d ", contact->active_cable_connections[i]);
    }
    puts("");
}

static void print_node(BombeNode *bombe_node)
{
    printf("%c -- %c [", bombe_node->scrambler_enigma.in->contact_num + 'A', bombe_node->scrambler_enigma.out->contact_num + 'A');
    for(uint8_t common = 0; common < bombe_node->outgoing_commons_count; ++common)
    {
        ScramblerEnigma current_scrambler = bombe_node->outgoing_commons[common]->scrambler_enigma;
        printf("%c -- %c, ",
               current_scrambler.in->contact_num + 'A',
               current_scrambler.out->contact_num + 'A');
    }
    puts("]");
}

static void print_wiring(TuringBombe *restrict turing_bombe)
{

}

static uint8_t find_most_frequent_menu_pos(const Menu *menu)
{
    const CribCipherTuple *most_freq_tuple = menu->menu;
    uint8_t most_freq_pos                  = 0;

    for (uint8_t tuple = 1; tuple < menu->len_menu; ++tuple)
    {
        CribCipherTuple *current_tuple = menu->menu + tuple;

        if (current_tuple->first.num_stubs > most_freq_tuple->first.num_stubs)
        {
            most_freq_tuple = current_tuple;
            most_freq_pos   = tuple;
        }
    }

    return most_freq_pos;
}

static void activate_contact(TuringBombe *restrict turing_bombe, const uint8_t first_contact, const uint8_t second_contact)
{
    Contact *primary_contact = turing_bombe->terminal->contacts[first_contact];
    Contact *secondary_contact = turing_bombe->terminal->contacts[second_contact];

    primary_contact->active_cable_connections[primary_contact->num_active_connections] = second_contact;
    secondary_contact->active_cable_connections[secondary_contact->num_active_connections] = first_contact;

    primary_contact->active_bit_vector |= (1 << second_contact);
    secondary_contact->active_bit_vector |= (1 << first_contact);

    primary_contact->num_active_connections++;
    secondary_contact->num_active_connections++;
}

static void setup_test_register(const CribCipherTuple *most_freq_pos, TuringBombe *restrict turing_bombe)
{
    const uint8_t terminal_i              = most_freq_pos->first.letter - 'A';
    turing_bombe->terminal->test_register = turing_bombe->terminal->contacts[terminal_i];
    activate_contact(turing_bombe, terminal_i, 0); //Test the letter "A"
}

static void set_stubs(TuringBombe *restrict turing_bombe,
                      BombeNode *bombe_nodes,
                      const CribCipherTuple *current_tuple,
                      BombeNode *current_node)
{
    const uint8_t num_stubs = current_tuple->first.num_stubs;
    current_node->outgoing_commons = malloc(sizeof(BombeNode *) * num_stubs);
    assertmsg(current_node->outgoing_commons != NULL, "malloc failed");
    current_node->outgoing_commons_count = num_stubs;

    const uint8_t i_current = current_tuple->first.letter - 'A';

    for(uint8_t stub = 0; stub < num_stubs; ++stub)
    {
        BombeNode *stub_bombe_node = bombe_nodes + turing_bombe->scrambler_columns_used;
        turing_bombe->scrambler_columns_used++;
        current_node->outgoing_commons[stub] = stub_bombe_node;

        CribCipherTuple *current_stub_tuple = current_tuple->first.stubs + stub;
        const char stub_char = (char) ((current_stub_tuple->first.letter == current_tuple->first.letter)
                                       ? current_stub_tuple->second.letter
                                       : current_stub_tuple->first.letter);

        const uint8_t i_stub = stub_char - 'A';
        stub_bombe_node->scrambler_enigma.in  = turing_bombe->terminal->contacts[i_current];
        stub_bombe_node->scrambler_enigma.out = turing_bombe->terminal->contacts[i_stub];
    }
}

static void setup_contact_connections(const Menu *menu,
                                      const uint8_t most_freq_menu_pos,
                                      TuringBombe *restrict turing_bombe,
                                      BombeNode *bombe_nodes)
{
    CribCipherTuple *most_freq_tuple = menu->menu + most_freq_menu_pos;
    CribCipherTuple *current_tuple   = menu->menu;
    BombeNode *current_node = bombe_nodes;

    for(uint8_t tuple_num = 1; tuple_num < menu->len_menu; ++tuple_num)
    {
        if(current_tuple == most_freq_tuple)
        {
            turing_bombe->starting_node = current_node;
        }

        const uint8_t i_first  = current_tuple->first.letter - 'A';
        const uint8_t i_second = current_tuple->second.letter - 'A';
        current_node->scrambler_enigma.in  = turing_bombe->terminal->contacts[i_first];
        current_node->scrambler_enigma.out = turing_bombe->terminal->contacts[i_second];

        set_stubs(turing_bombe, bombe_nodes, current_tuple, current_node);

        current_tuple = menu->menu + tuple_num;

        current_node = bombe_nodes + turing_bombe->scrambler_columns_used;
        turing_bombe->scrambler_columns_used++;
    }

//    turing_bombe->scrambler_columns_used--;
    const uint8_t i_first  = current_tuple->first.letter - 'A';
    const uint8_t i_second = current_tuple->second.letter - 'A';
    current_node->scrambler_enigma.in  = turing_bombe->terminal->contacts[i_first];
    current_node->scrambler_enigma.out = turing_bombe->terminal->contacts[i_second];

    puts("");
}

static int32_t setup_turing_bombe(TuringBombe *turing_bombe, Menu *menu, BombeNode *bombe_nodes)
{
    const uint8_t most_freq_pos = find_most_frequent_menu_pos(menu);

    setup_test_register(menu->menu + most_freq_pos, turing_bombe);

    setup_contact_connections(menu, most_freq_pos, turing_bombe, bombe_nodes);

    return 0;
}

static void setup_scramblers(BombeNode *bombe_nodes,
                             Menu *menu,
                             const enum ROTOR_TYPE rotor_one_type,
                             const enum ROTOR_TYPE rotor_two_type,
                             const enum ROTOR_TYPE rotor_three_type)
{
    //TODO reuse rotors if they dont differ...
    //TODO free_scrambler...
    for(uint8_t tuple = 0; tuple < menu->len_menu; ++tuple)
    {
        BombeNode *current_node = bombe_nodes + tuple;
        CribCipherTuple *current_tuple = menu->menu + tuple;
        // The upper (first) rotor of the Turing Bombe resembled the rightmost (third) rotor of the Enigma.
        // In contrast to the Enigma the Turing Bombe rotated the "third" Enigma rotor the fastest.
        // The Scramblers rotated after one complete turn, independent of the notch, and they checked a menu.
        current_node->scrambler_enigma.rotors[0] = create_rotor_by_type(rotor_three_type, 0, 0);
        current_node->scrambler_enigma.rotors[1] = create_rotor_by_type(rotor_two_type, 0, 0);
        current_node->scrambler_enigma.rotors[2] = create_rotor_by_type(rotor_one_type, current_tuple->position, 0);
    }
}

static void permutate_all_scramblers(TuringBombe *restrict turing_bombe, LinkedList bombe_stack)
{
    BombeNode *current_node = turing_bombe->starting_node;
    //TODO working copy?

}

static void advance_all_scramblers(TuringBombe *restrict turing_bombe, BombeNode *bombe_nodes)
{
    for(uint8_t scrambler = 0; scrambler < turing_bombe->scrambler_columns_used; ++scrambler)
    {
        BombeNode *current_node = bombe_nodes + scrambler;

        Rotor *first_rotor = current_node->scrambler_enigma.rotors[0];
        Rotor *second_rotor = current_node->scrambler_enigma.rotors[1];
        Rotor *third_rotor = current_node->scrambler_enigma.rotors[2];

        first_rotor->position++;
        if(first_rotor->position % ALPHABET_SIZE == 0)
        {
            first_rotor->position = 0;
            second_rotor->position++;
            if(second_rotor->position % ALPHABET_SIZE == 0)
            {
                second_rotor->position = 0;
                third_rotor->position++;
            }
        }
    }
}

static bool should_halt(TuringBombe *restrict turing_bombe)
{
    const uint8_t active_connections = turing_bombe->terminal->test_register->num_active_connections;
    return active_connections == 1 || active_connections == 25;
}

//TODO i dont like to pass the bombe_nodes separately but speed...
static bool traverse_rotor_conf(TuringBombe *restrict turing_bombe, BombeNode *bombe_nodes)
{
    LinkedList nodes_stack = ll_create(sizeof(BombeNode));

    for (uint8_t rotor1_pos = 0; rotor1_pos < ALPHABET_SIZE; ++rotor1_pos)
    {
        for (uint8_t rotor2_pos = 0; rotor2_pos < ALPHABET_SIZE; ++rotor2_pos)
        {
            for (uint8_t rotor3_pos = 0; rotor3_pos < ALPHABET_SIZE; ++rotor3_pos)
            {
                permutate_all_scramblers(turing_bombe, nodes_stack);
                if(should_halt(turing_bombe)) return true;
                advance_all_scramblers(turing_bombe, bombe_nodes);
            }
        }
    }

    ll_destroy(nodes_stack);

    return false;
}

static void build_node_lookup_table(BombeNode *terminal_in_relations[NUM_SCRAMBLERS_PER_ROW][NUM_CONTACTS_PER_COMMON],
                                    uint8_t num_nodes_per_terminal[NUM_SCRAMBLERS_PER_ROW],
                                    BombeNode *bombe_nodes,
                                    const uint8_t scrambler_used)
{
    for(uint8_t i = 0; i < scrambler_used; ++i)
    {
        BombeNode *current_node = bombe_nodes + i;
        const uint8_t contact_num = current_node->scrambler_enigma.in->contact_num;
        terminal_in_relations[contact_num][num_nodes_per_terminal[contact_num]++] = current_node;
    }
}

int32_t start_turing_bombe(const char *restrict crib, const char *restrict ciphertext, const uint32_t crib_offset)
{
    if (!is_valid_crip_position(crib, ciphertext, crib_offset)) return ERR_INVALID_CRIB;

    Contact contacts[ALPHABET_SIZE] = {0};
    Terminal terminal               = {0};
    for (uint8_t contact = 0; contact < ALPHABET_SIZE; ++contact)
    {
        contacts[contact].contact_num = contact;
        terminal.contacts[contact]    = &contacts[contact];
    }
    Reflector *reflector                    = create_reflector_by_type(UKW_B);
    BombeNode nodes[NUM_SCRAMBLERS_PER_ROW] = {0};
    TuringBombe turing_bombe                = {.terminal = &terminal, .reflector = reflector, .scrambler_columns_used = 0};

    Menu *menu = find_longest_menu(crib, ciphertext);
    if (menu == NULL)
    {
        free(reflector);
        return ERR_CYCLE;
    }

    //FIXME some in scrambler contacts are NULL?
    //TODO robustness checks...
    setup_turing_bombe(&turing_bombe, menu, nodes);

    BombeNode *terminal_in_relations[ALPHABET_SIZE][NUM_CONTACTS_PER_COMMON] = {0};
    uint8_t num_nodes_per_terminal[ALPHABET_SIZE] = {0};

    build_node_lookup_table(terminal_in_relations, num_nodes_per_terminal, nodes, turing_bombe.scrambler_columns_used);

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
                setup_scramblers(nodes, menu, rotor_one_type, rotor_two_type, rotor_three_type);
                if(traverse_rotor_conf(&turing_bombe, nodes))
                {
                    //TODO report position
                }
                free_scramblers(&turing_bombe, nodes);
            }
        }
    }

//    free_bombe(&turing_bombe, nodes);
    free_menu(menu);

    return ret_val;
}
