//
// Created by escha on 01.12.24.
//

#include <string.h>
#include <stdlib.h>

#include "graph_test.h"
#include "turing_bombe/turing_bombe.h"

//TODO communicate these codes to the outside world somehow...
enum
{
    ERR_NULL_STR = -1,
    ERR_INVALID_OFFSET = -2,
    ERR_CRIB_TOO_LONG = -3,
    ERR_CRIB_TOO_SHORT = -4
};

typedef struct
{
    MNode crib_node;
    MNode cipher_node;
    bool visited;
} Tuple;

static void write_dot_format(const char *restrict crib, const char *restrict ciphertext)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_CRIB_CIPHER_CYCLE, "w")) != NULL, "cant open" FILE_PATH_CRIB_CIPHER_CYCLE);

    fputs("graph G {\n", file);
    fputs("\tlayout=neato;\n", file);
    fputs("\tedge [fontsize=11];\n", file);
    const size_t len = strlen(crib);
    for (size_t i = 0; i < len; ++i)
    {
        fprintf(file, "\t\"%c\" -- \"%c\" [label=\"%zu\"];\n", ciphertext[i], crib[i], i);
    }
    fputs("}\n", file);

    fclose(file);
}

static int32_t validate_input(const char *restrict crib, const char *ciphertext, size_t offset)
{
    if (crib == NULL || ciphertext == NULL)
    {
        fprintf(stderr, "Menu Error: Null pointer string");
        return ERR_NULL_STR;
    }

    size_t crib_len = strlen(crib);

    if (crib_len + offset > strlen(ciphertext))
    {
        fprintf(stderr, "Menu Error: Invalid offset");
        return ERR_INVALID_OFFSET;
    }
    if (crib_len > MAX_CRIB_LEN)
    {
        fprintf(stderr, "Menu Error: Crib too long");
        return ERR_CRIB_TOO_LONG;
    }
    if (crib_len <= 2)
    {
        fprintf(stderr, "Menu Error: Crib too short");
        return ERR_CRIB_TOO_SHORT;
    }

    return 0;
}

static void build_lookup_table(const char *restrict crib,
                               const char *ciphertext,
                               const size_t offset,
                               const size_t len,
                               Tuple *lookup_table[ALPHABET_SIZE][MAX_CRIB_LEN],
                               uint8_t *tuples_per_letter,
                               Tuple *tuples)
{
    const char *ciphertext_start = ciphertext + offset;

    for (uint8_t position = 0; position < (uint8_t) len; ++position)
    {
        Tuple *current_tuple = tuples + position;

        const char crib_letter = crib[position];
        const char cipher_letter = ciphertext_start[position];
        const uint8_t crib_i = crib_letter - 'A';
        const uint8_t cipher_i = cipher_letter - 'A';

        current_tuple->crib_node.position = position;
        current_tuple->cipher_node.position = position;

        current_tuple->crib_node.letter = crib_letter;
        current_tuple->cipher_node.letter = cipher_letter;

        lookup_table[crib_i][tuples_per_letter[crib_i]] = current_tuple;
        lookup_table[cipher_i][tuples_per_letter[cipher_i]] = current_tuple;

        tuples_per_letter[crib_i]++;
        tuples_per_letter[cipher_i]++;
    }
}

static inline bool is_matching_chars_tuple(const Tuple *first, const Tuple *second)
{
    return second != NULL &&
           ((first->crib_node.letter == second->crib_node.letter &&
             first->cipher_node.letter == second->cipher_node.letter) ||
            (first->cipher_node.letter == second->crib_node.letter &&
             first->crib_node.letter == second->cipher_node.letter));
}

static bool find_cycle_dfs(Tuple *tuple,
                           Tuple *parent,
                           const char last_char,
                           bool *is_matching_char_tuple,
                           Tuple *lookup_table[ALPHABET_SIZE][MAX_CRIB_LEN],
                           const uint8_t *tuples_per_letter,
                           MenuGraph *menu_graph)
{
    if (tuple == NULL) return false;
    if (tuple == parent) return false;

    if (is_matching_chars_tuple(tuple, parent))
    {
        *is_matching_char_tuple = true;
        return false;
    }

    MNode *node = menu_graph->nodes + menu_graph->len_menu;

    const char lookup_char = (char) ((tuple->crib_node.letter == last_char) ? tuple->cipher_node.letter
                                                                            : tuple->crib_node.letter);

    node->letter = last_char;
    node->position = tuple->crib_node.position;
    menu_graph->len_menu++;

    if (tuple->visited) return true;
    tuple->visited = true;

    const uint8_t i_lookup_char = lookup_char - 'A';

    for (uint8_t i = 0; i < tuples_per_letter[i_lookup_char]; ++i)
    {
        if (find_cycle_dfs(lookup_table[i_lookup_char][i],
                           tuple,
                           lookup_char,
                           is_matching_char_tuple,
                           lookup_table,
                           tuples_per_letter,
                           menu_graph))
        {
            return true;
        }
    }

    menu_graph->len_menu--;
    tuple->visited = false;

    return false;
}

static int32_t build_menu(const size_t len,
                          Tuple *lookup_table[ALPHABET_SIZE][MAX_CRIB_LEN],
                          uint8_t *tuples_per_letter,
                          Tuple *tuples,
                          MenuGraph *menu_graph)
{
    bool ret = false;
    MNode longest_menu_nodes[MAX_CRIB_LEN];
    MenuGraph longest_menu = {.nodes = longest_menu_nodes, .len_menu = 0};

    for (uint8_t position = 0; position < (uint8_t) len; ++position)
    {
        MNode temp_nodes[MAX_CRIB_LEN];
        MenuGraph temp_menu = {.nodes = temp_nodes, .len_menu = 0};

        Tuple *current_tuple = tuples + position;
        bool is_matching_chars_tuple = false;

        const char starting_char = current_tuple->crib_node.letter;

        if (find_cycle_dfs(current_tuple, NULL, starting_char,
                           &is_matching_chars_tuple, lookup_table, tuples_per_letter, &temp_menu)
            || is_matching_chars_tuple)
        {
            if (temp_menu.len_menu <= 2) continue;
            if (temp_menu.len_menu > longest_menu.len_menu)
            {
                memcpy(longest_menu_nodes, temp_nodes, sizeof(Tuple) * temp_menu.len_menu);
                longest_menu.len_menu = temp_menu.len_menu;
            }
            ret = true;
            if (temp_menu.len_menu >= len / 2) break;
        }
    }

    if (ret == true)
    {
        longest_menu.len_menu--; //One too far because of the visited position in the dfs
        menu_graph->nodes = malloc(sizeof(MNode) * longest_menu.len_menu);
        assertmsg(menu_graph->nodes != NULL, "malloc failed");
        memcpy(menu_graph->nodes, longest_menu_nodes, sizeof(Tuple) * longest_menu.len_menu);
        menu_graph->len_menu = longest_menu.len_menu;
    }

    return ret;
}

static bool set_stubs_for_node(MNode *node,
                               const Tuple *lookup_table[ALPHABET_SIZE][MAX_CRIB_LEN],
                               const uint8_t *tuples_per_letter,
                               MenuGraph *menu_graph)
{
    const uint8_t current_i = node->letter - 'A';
    uint8_t num_stubs = tuples_per_letter[current_i] - 2; //ignoring the own and adjacent one

    node->stubs = malloc(sizeof(MNode) * num_stubs);
    assertmsg(node->stubs != NULL, "malloc failed");

    uint8_t stubs_count = 0;
    for(uint8_t stub = 0; stub < tuples_per_letter[current_i]; ++stub)
    {
        if(stubs_count >= NUM_CONTACTS_PER_COMMON - 1) break;
        const Tuple *current_stub_tuple = lookup_table[current_i][stub];
        if (node->position == current_stub_tuple->crib_node.position) continue;
        if(current_stub_tuple->visited) continue;
        MNode stub_node = (node->letter == current_stub_tuple->crib_node.letter) ? current_stub_tuple->cipher_node : current_stub_tuple->crib_node;
        //TODO
//        if(menu_graph->len_menu + menu_graph->num_stubs < NUM_SCRAMBLERS_PER_ROW - 1 ||
//           is_matching_chars_tuple(current_tuple, current_stub_tuple)) //TODO again make this not greedy. Loop through and only check for tuple of tuples?
//        {
//            menu_graph->num_stubs++;
//            memcpy(node->stubs + stubs_count, &stub_node, sizeof(MNode));
//            stubs_count++;
//        }
    }
    node->num_stubs = stubs_count;

    return stubs_count > 0;
}

static void set_stubs(MenuGraph *menu_graph,
                      Tuple *lookup_table[ALPHABET_SIZE][MAX_CRIB_LEN],
                      uint8_t *tuples_per_letter)
{


}

MenuGraph *find_menu(const char *restrict crib, const char *ciphertext, size_t offset)
{
    if (validate_input(crib, ciphertext, offset) != 0) return NULL;

    MenuGraph *menu_graph = malloc(sizeof(MenuGraph));
    assertmsg(menu_graph != NULL, "malloc failed");
    menu_graph->len_menu = 0;
    menu_graph->num_stubs = 0;

    write_dot_format(crib, ciphertext);

    Tuple *lookup_table[ALPHABET_SIZE][MAX_CRIB_LEN] = {0};
    uint8_t tuples_per_letter[ALPHABET_SIZE] = {0};
    Tuple tuples[MAX_CRIB_LEN] = {0};

    const size_t len = strlen(crib);

    build_lookup_table(crib, ciphertext, offset, len, lookup_table, tuples_per_letter, tuples);
    build_menu(len, lookup_table, tuples_per_letter, tuples, menu_graph);

    return menu_graph;
}


