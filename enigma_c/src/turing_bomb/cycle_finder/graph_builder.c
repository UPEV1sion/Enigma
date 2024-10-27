#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "graph_builder.h"
#include "cycle_finder_graph.h"

//
// Created by Emanuel on 22.10.24.
//

static void build_graph(GraphOfLongestCycle *restrict graph, const char *restrict crib, const char *restrict ciphertext, const size_t len,
                        GraphNode *nodes)
{
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
GraphOfLongestCycle* build_best_scrambler_graph(const char *restrict crib,
                                                const char *restrict ciphertext,
                                                TuringBomb *restrict turing_bomb)
{

    // CycleCribCipher *cycle = find_longest_cycle_graph(crib, ciphertext);
    // if (cycle == NULL)
    // {
    //     fprintf(stderr, "No cycles found\n");
    //     return NULL;
    // }

    return NULL;
}
