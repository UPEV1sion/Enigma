#include <string.h>

#include "cycle_finder.h"

//
// Created by Emanuel on 01.11.2024.
//

void free_menu(Menu *menu)
{
    for(uint8_t node = 0; node < MAX_CRIB_LEN; ++node)
    {
        free(menu->graph.all_menu_nodes[node]);
    }
    free(menu);
}

static void build_graph(const char *restrict crib, const char *restrict ciphertext, const size_t len, Menu *menu)
{
    for (uint8_t position = 0; position < len; ++position)
    {
        MenuNode *node = malloc(sizeof(MenuNode));
        assertmsg(node != NULL, "malloc failed");
        node->cipher_char                    = ciphertext[position];
        node->crib_char                      = crib[position];
        node->position                       = position;
        node->is_cycle                       = false;
        node->is_stub                        = false;
        menu->graph.all_menu_nodes[position] = node;

        const uint8_t i_crib    = crib[position] - 'A';
        const uint8_t i_cipher  = crib[position] - 'A';
        //TODO maybe fixed array

    }

    menu->graph.len_all_menu_nodes = len;

    puts("");
}

Menu* allocate_menu()
{
    Menu *menu = malloc(sizeof(Menu));
    assertmsg(menu != NULL, "malloc failed");

    menu->graph.len_all_menu_nodes = 0;
    memset(menu->graph.all_menu_nodes, 0, sizeof(MenuNode) * MAX_CRIB_LEN);
    memset(menu->graph.relations, 0, sizeof(MenuNode) * ALPHABET_SIZE);

    menu->nodes_with_stubs    = NULL;
    menu->nodes_without_stubs = NULL;
    menu->len_with_stubs      = 0;
    menu->len_without_stubs   = 0;

    return menu;
}


Menu* find_longest_cycle(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;
    assertmsg(len <= MAX_CRIB_LEN, "Try a shorter crib");
    assertmsg(len >= 2, "Try a longer crib");

    Menu *menu = allocate_menu();
    build_graph(crib, ciphertext, len, menu);


    return menu;
}
