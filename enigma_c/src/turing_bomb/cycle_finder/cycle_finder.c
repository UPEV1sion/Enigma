#include <string.h>

#include "cycle_finder.h"

//
// Created by Emanuel on 01.11.2024.
//

//TODO more layer of stubs than 1 needed?

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
                           CribCipherTuple *parent,
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

    CribCipherTuple *node = menu->cycle + menu->len_cycle;

    const char lookup_char = (char) ((tuple->first.letter == last_char) ? tuple->second.letter : tuple->first.letter);

    node->first.letter  = last_char;
    node->second.letter = lookup_char;
    node->position      = tuple->position;
    node->visited       = true;
    menu->len_cycle++;

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

    menu->len_cycle--;
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
    Menu longest_menu = {.cycle = longest_menu_nodes, .len_cycle = 0};

    for(uint8_t position = 0; position < (uint8_t) len; ++position)
    {
        CribCipherTuple temp_nodes[MAX_CRIB_LEN];
        Menu temp_menu = {.cycle = temp_nodes, .len_cycle = 0};

        CribCipherTuple *current_tuple = tuples + position;
        bool matching_chars_tuple = false;

        if(find_cycle_dfs(current_tuple,NULL,current_tuple->first.letter,
                          &matching_chars_tuple, relations, tuples_per_letter,&temp_menu)
                          || matching_chars_tuple)
        {
            if(temp_menu.len_cycle <= 2) continue;
            if(temp_menu.len_cycle > longest_menu.len_cycle)
            {
                memcpy(longest_menu_nodes, temp_nodes, sizeof(CribCipherTuple) * temp_menu.len_cycle);
                longest_menu.len_cycle = temp_menu.len_cycle;
            }
            ret = true;
            if(temp_menu.len_cycle >= len / 2) break;
        }
    }

    if(ret == true)
    {
        menu->cycle = malloc(sizeof(CribCipherTuple) * longest_menu.len_cycle);
        assertmsg(menu->cycle != NULL, "malloc failed");
        memcpy(menu->cycle, longest_menu_nodes, sizeof(CribCipherTuple) * longest_menu.len_cycle);
        menu->len_cycle = longest_menu.len_cycle;
    }

    return ret;
}

static void free_node(MenuNode *node)
{
    for(uint8_t stub = 0; stub < node->num_stubs; ++stub)
    {
        free(node->stubs[stub]);
    }
    free(node->stubs);
}

void free_menu(Menu *menu)
{
    CribCipherTuple *tuple = menu->cycle;
    free_node(&tuple->first);

    for(uint8_t node = 0; node < menu->len_cycle; ++node)
    {
        tuple = menu->cycle + node;
        free_node(&tuple->second);
    }

    free(menu->cycle);
    free(menu);
}

static void set_stubs(MenuNode *node,
                      const CribCipherTuple *current_tuple,
                      CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN],
                      const uint8_t *tuples_per_letter)
{
    const uint8_t current_i = node->letter - 'A';
    uint8_t num_stubs = tuples_per_letter[current_i] - 1; //ignoring the own one

    node->stubs = malloc(sizeof(CribCipherTuple) * num_stubs); //TODO fix the wasteful allocation of unnecessary nodes... maybe realloc?
    assertmsg(node->stubs != NULL, "malloc failed");

    uint8_t stubs_count = 0;
    for(uint8_t stub = 0; stub < tuples_per_letter[current_i]; ++stub)
    {
        CribCipherTuple *current_stub = relations[current_i][stub];
        if (current_tuple == current_stub) continue;
        if(current_stub->visited)
        {
            continue;
        }
        node->stubs[stubs_count] = malloc(sizeof(CribCipherTuple));
        assertmsg(node->stubs[stubs_count] != NULL, "malloc failed");
        memcpy(node->stubs[stubs_count], current_stub, sizeof(CribCipherTuple));
        stubs_count++;
    }
    node->num_stubs = stubs_count;
}

static void find_stubs(CribCipherTuple *relations[ALPHABET_SIZE][MAX_CRIB_LEN],
                       const uint8_t *tuples_per_letter,
                       Menu *menu)
{
    CribCipherTuple *last_tuple = menu->cycle;

    set_stubs(&last_tuple->first, last_tuple, relations, tuples_per_letter);
    set_stubs(&last_tuple->second, last_tuple, relations, tuples_per_letter);

    for(uint8_t position = 1; position < menu->len_cycle; ++position)
    {
        CribCipherTuple *current_tuple = menu->cycle + position;

        set_stubs(&current_tuple->second, current_tuple, relations, tuples_per_letter);
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
    menu->len_cycle = 0;

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
