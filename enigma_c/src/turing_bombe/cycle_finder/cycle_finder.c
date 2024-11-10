#include <string.h>

#include "cycle_finder.h"

//
// Created by Emanuel on 01.11.2024.
//

/*
 * This is a "graph" approach, but instead of linking them directly, I stored them in a Lookup Table".
 *
 * Why not linking?
 * We have the problem that if we were to link the tuples together,
 * we need to link tuples with the matching char in crib and ciphertext.
 * This results in an incorrect traversal,
 * because a char from the crib must be substituted to the one in ciphertext at the same place and vice versa.
 * If were link them together, a traversal like A-B → A-C → A-D is feasible.
 * A correct traversal could look like this: A-B → C-B → D-C.
 * So we build a "Lookup Table" where the corresponding char is looked up.
 *
 * Runtime:
 * Building the lookup is Θ(n) (linear).
 * DFS traversal: Θ(V + E).
 * When a cycle is found, all nodes in the cycle are not visited_cycle again.
 * But when no cycle is found, we iterate through the whole crib, performing a DFS at each position.
 * So the WC is O(n^2)
 *
 * A more realistic scenario is that a cycle is found, marking the visited_cycle nodes.
 * From testing, I can verify that the runtime is linear
 * for the case ciphertext and crib length is <= 26, which is always the case.
 */

static void build_frequency_table(const char *restrict crib,
                                  const char *restrict ciphertext,
                                  const size_t len,
                                  CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN],
                                  uint8_t *tuples_per_letter,
                                  CribCipherTuple *tuples)
{
    for(uint8_t position = 0; position < (uint8_t) len; ++position)
    {
        const uint8_t i_crib = crib[position] - 'A';
        const uint8_t i_ciphertext = ciphertext[position] - 'A';

        CribCipherTuple *tuple = tuples + position;

        tuple->position      = position;
        tuple->first.letter  = crib[position];
        tuple->second.letter = ciphertext[position];
        tuple->visited       = false;

        relations[i_crib][tuples_per_letter[i_crib]++] = tuple;
        relations[i_ciphertext][tuples_per_letter[i_ciphertext]++] = tuple;
    }
}

static inline bool is_matching_chars_tuple(const CribCipherTuple *first, const CribCipherTuple *second)
{
    return second != NULL &&
    ((first->first.letter == second->first.letter && first->second.letter == second->second.letter) ||
    (first->second.letter == second->first.letter && first->first.letter == second->second.letter));
}

static bool find_cycle_dfs(CribCipherTuple *tuple,
                           const CribCipherTuple *parent,
                           const char last_char,
                           bool *matching_char_tuple,
                           CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN],
                           const uint8_t *tuples_per_letter,
                           Menu *menu)
{
    if(tuple == NULL) return false;
    if(tuple == parent) return false;

    if(is_matching_chars_tuple(tuple, parent))
    {
        *matching_char_tuple = true;
        return false;
    }

    CribCipherTuple *node = menu->menu + menu->len_menu;

    const char lookup_char = (char) ((tuple->first.letter == last_char) ? tuple->second.letter : tuple->first.letter);

    node->first.letter  = last_char;
    node->second.letter = lookup_char;
    node->position      = tuple->position;
    node->visited       = true;
    menu->len_menu++;

    if(tuple->visited)
        return true;
    tuple->visited = true;

    const uint8_t i_lookup_char = lookup_char - 'A';

    for (uint8_t i = 0; i < tuples_per_letter[i_lookup_char]; ++i)
    {
        if(find_cycle_dfs(relations[i_lookup_char][i],
                          tuple,
                          lookup_char,
                          matching_char_tuple,
                          relations,
                          tuples_per_letter,
                          menu))
        {
            return true;
        }
    }

    menu->len_menu--;
    tuple->visited = false;

    return false;
}

static bool find_cycle(const size_t len,
                       CribCipherTuple *tuples,
                       CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN],
                       const uint8_t *tuples_per_letter,
                       Menu *menu)
{
    bool ret = false;
    CribCipherTuple longest_menu_nodes[MAX_CRIB_LEN];
    Menu longest_menu = {.menu = longest_menu_nodes, .len_menu = 0};

    for(uint8_t position = 0; position < (uint8_t) len; ++position)
    {
        CribCipherTuple temp_nodes[MAX_CRIB_LEN];
        Menu temp_menu = {.menu = temp_nodes, .len_menu = 0};

        CribCipherTuple *current_tuple = tuples + position;
        bool matching_chars_tuple = false;

        const char starting_char = current_tuple->first.letter;
        const uint8_t i_starting_char = starting_char - 'A';

        if(tuples_per_letter[i_starting_char] <= 1) continue; //tuple is a stub

        if(find_cycle_dfs(current_tuple, NULL, starting_char,
                          &matching_chars_tuple, relations, tuples_per_letter,&temp_menu)
                          || matching_chars_tuple)
        {
            if(temp_menu.len_menu <= 2 || temp_menu.len_menu > NUM_SCRAMBLERS_PER_ROW) continue;
            if(temp_menu.len_menu > longest_menu.len_menu)
            {
                memcpy(longest_menu_nodes, temp_nodes, sizeof(CribCipherTuple) * temp_menu.len_menu);
                longest_menu.len_menu = temp_menu.len_menu;
            }
            ret = true;
            if(temp_menu.len_menu >= len / 2) break;
        }
    }

    if(ret == true)
    {
        menu->menu = malloc(sizeof(CribCipherTuple) * longest_menu.len_menu);
        assertmsg(menu->menu != NULL, "malloc failed");
        memcpy(menu->menu, longest_menu_nodes, sizeof(CribCipherTuple) * longest_menu.len_menu);
        menu->len_menu = longest_menu.len_menu;
    }

    return ret;
}

//FIXME
static void free_node(const MenuNode *node)
{
    for(uint8_t stub = 0; stub < node->num_stubs; ++stub)
    {
        free(node->stubs[stub]);
    }
    free(node->stubs);
}

void free_menu(Menu *menu)
{
    CribCipherTuple *tuple = menu->menu;
    free_node(&tuple->first);

    for(uint8_t node = 0; node < menu->len_menu; ++node)
    {
        tuple = menu->menu + node;
        free_node(&tuple->second);
    }

    free(menu->menu);
    free(menu);
}

static bool set_stubs(MenuNode *node,
                      const CribCipherTuple *current_tuple,
                      CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN],
                      const uint8_t *tuples_per_letter,
                      Menu *menu)
{
    const uint8_t current_i = node->letter - 'A';
    const uint8_t num_stubs = tuples_per_letter[current_i] - 1; //ignoring the own one

    node->stubs = malloc(sizeof(CribCipherTuple) * num_stubs); //TODO fix the wasteful allocation of unnecessary nodes... maybe realloc?
    assertmsg(node->stubs != NULL, "malloc failed");

    uint8_t stubs_count = 0;
    for(uint8_t stub = 0; stub < tuples_per_letter[current_i]; ++stub)
    {
        if(stub >= NUM_CONTACTS_PER_COMMON - 1) break;
        if(menu->len_menu >= NUM_SCRAMBLERS_PER_ROW - 1) break;
        const CribCipherTuple *current_stub = relations[current_i][stub];
        if (current_tuple == current_stub) continue;
        if(current_stub->visited)
        {
            continue;
        }
        node->stubs[stubs_count] = malloc(sizeof(CribCipherTuple));
        assertmsg(node->stubs[stubs_count] != NULL, "malloc failed");
        memcpy(node->stubs[stubs_count], current_stub, sizeof(CribCipherTuple));
        stubs_count++;
        menu->len_menu++;
    }
    node->num_stubs = stubs_count;

    return stubs_count > 0;
}

static void find_stubs(CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN],
                       const uint8_t *tuples_per_letter,
                       Menu *menu)
{
    CribCipherTuple *last_tuple = menu->menu;

    set_stubs(&last_tuple->first, last_tuple, relations, tuples_per_letter, menu);
    set_stubs(&last_tuple->second, last_tuple, relations, tuples_per_letter, menu);

    uint8_t common_counter = 0;

    for(uint8_t position = 1; position < menu->len_menu; ++position)
    {
        if(common_counter >= NUM_COMMONS - 1) break;
        CribCipherTuple *current_tuple = menu->menu + position;

        if(set_stubs(&current_tuple->second, current_tuple, relations, tuples_per_letter, menu))
        {
            common_counter++;
        }
        current_tuple->first.stubs = last_tuple->second.stubs;
        current_tuple->first.num_stubs = last_tuple->second.num_stubs;
        last_tuple = current_tuple;
    }
}

Menu* find_longest_menu(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;
    assertmsg(len <= MAX_CRIB_LEN, "Try a shorter crib");
    assertmsg(len >= 2, "Try a longer crib");

    Menu *menu = malloc(sizeof(Menu));
    assertmsg(menu != NULL, "malloc failed");
    menu->len_menu = 0;

    CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN];
    uint8_t tuples_per_letter[ALPHABET_SIZE] = {0};
    CribCipherTuple tuples[MAX_CRIB_LEN];

    build_frequency_table(crib, ciphertext, len, relations, tuples_per_letter, tuples);

    if(!find_cycle(len, tuples, relations, tuples_per_letter, menu))
    {
        free(menu);
        fprintf(stderr, "No cycle found");
        return NULL;
    }

    find_stubs(relations, tuples_per_letter, menu);

    return menu;
}
