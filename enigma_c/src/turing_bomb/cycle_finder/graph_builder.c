#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "graph_builder.h"
#include "turing_bomb/turing_bomb.h"

//
// Created by Emanuel on 22.10.24.
//

static void print_graph(const Graph *graph)
{
    for (uint8_t i = 0; i < ALPHABET_SIZE; ++i)
    {
        printf("%c: ", i + 'A');
        for (int j = 0; j < graph->nodes_per_letter[i]; ++j)
        {
            const Node *current = graph->relations[i][j];
            printf("[%c : %c, (%02d)] ", current->crib_char, current->cipher_char, current->position);
        }
        puts("");
    }
}

static inline bool is_matching_chars_tuple(const Node *first, const Node *second)
{
    return second != NULL && first->cipher_char == second->cipher_char
           && first->crib_char == second->crib_char;
}

static void build_graph(Graph *restrict graph, const char *restrict crib, const char *restrict ciphertext, const size_t len,
                        Node *nodes)
{
    for (uint8_t i = 0; i < ALPHABET_SIZE; ++i)
    {
        Node *node_array[MAX_CRIB_LEN] = {0};
        graph->relations[i] = node_array;
    }

    for (uint8_t i = 0; i < (uint8_t) len; ++i)
    {
        nodes[i].crib_char   = crib[i];
        nodes[i].cipher_char = ciphertext[i];
        nodes[i].position    = i;

        const uint8_t i_crib   = crib[i] - 'A';
        const uint8_t i_cipher = ciphertext[i] - 'A';

        graph->relations[i_crib][graph->nodes_per_letter[i_crib]++]     = nodes + i;
        graph->relations[i_cipher][graph->nodes_per_letter[i_cipher]++] = nodes + i;
    }
}

/**
 * @brief Find cycles between the crib and plain, using a graph and a modified DFS
 * @warning Uses restrict pointers: It must be assured that crib and ciphertext reside in different storage areas.
 * @param crib The crib
 * @param ciphertext The Ciphertext
 * @return Cycle: cycle if cycles where found, NULL for errors and no cycles found.
 */
Graph* build_best_scrambler_graph(const char *restrict crib,
                                  const char *restrict ciphertext,
                                  TuringBomb *restrict turing_bomb)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;
    assertmsg(len <= MAX_CRIB_LEN, "Try a shorter crib");

    Node nodes[MAX_CRIB_LEN] = {0};
    Graph temp_graph         = {0};

    Node *relations[ALPHABET_SIZE][MAX_CRIB_LEN] = {0};
    temp_graph.relations = relations;

    return NULL;
}
