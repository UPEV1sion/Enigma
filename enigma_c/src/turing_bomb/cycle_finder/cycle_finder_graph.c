#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "cycle_finder_graph.h"
#include "helper/helper.h"
#include "turing_bomb/turing_bomb.h"

//
// Created by Emanuel on 29.09.2024.
//

//TODO bitmask for crib or cycle

/*
 *  I named this a graph approach, but im not linking them together.
 *  This is more of a "HashMap" approach but with dfs traversal
 */

typedef struct Node Node;

struct Node
{
    Node **neighbours;
    size_t neighbour_count;

    struct
    {
        char crib_char, cipher_char;
        uint8_t position;
        // For building graph
        bool visited;
    } data;

    // For traversing
    bool visited;
};

typedef struct
{
    Node *relations[ALPHABET_SIZE][ALPHABET_SIZE];
    uint8_t indexing[ALPHABET_SIZE];
} Graph;

/**
 * @brief Establishes a connection from node to neighbour. Unidirectional
 * @warning deprecated, unused
 * @param node The node where the connection to neighbour should be established
 * @param neighbour The node with node should be linked to.
 */
static void add_neighbour(Node *restrict node, Node *restrict neighbour)
{
    node->neighbours = realloc(node->neighbours, (node->neighbour_count + 1) * sizeof(Node *));
    assertmsg(node->neighbours != NULL, "realloc failed");
    node->neighbours[node->neighbour_count++] = neighbour;
}

static void write_dot_format(const char *restrict crib, const char *restrict ciphertext)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_CRIB_CIPHER_CYCLE, "w")) != NULL, "cant open" FILE_PATH_CRIB_CIPHER_CYCLE);

    fputs("graph G {\n", file);
    fputs("\tlayout=neato;\n", file);
    const size_t len = strlen(crib);
    for (size_t i = 0; i < len; ++i)
    {
        fprintf(file, "\t\"%c\" -- \"%c\";\n", ciphertext[i], crib[i]);
    }
    fputs("}\n", file);

    fclose(file);
}

static bool dfs_find_cycles(Graph *restrict graph, Node *node, const Node *parent, const char last_char, CycleCribCipher *restrict cycle)
{
    if (node->data.crib_char == 0) return false;
    if (node == parent) return false;

    // printf("Visiting node: %c : %c, %u\n", node->data.crib_char, node->data.cipher_char, node->data.position);
    if (node->visited)
    {
        return true;
    }
    node->visited = true;

    cycle->chars_w_stubs[cycle->len_w_stubs] = last_char;
    cycle->chars_wo_stubs[cycle->len_wo_stubs] = last_char;
    cycle->positions_w_stubs[cycle->len_w_stubs++]   = node->data.position;
    cycle->positions_wo_stubs[cycle->len_wo_stubs++] = node->data.position;

    const char lookup_char = node->data.crib_char == last_char ? node->data.cipher_char : node->data.crib_char;

    for (uint8_t i = 0; i < graph->indexing[lookup_char - 'A']; ++i)
    {
        if (dfs_find_cycles(graph, graph->relations[lookup_char - 'A'][i], node, lookup_char, cycle))
            return true;
    }

    cycle->len_wo_stubs--;
    node->visited = false;

    return false;
}

static bool find_cycle(Graph *restrict graph, Node *restrict nodes, const uint8_t nodes_len, CycleCribCipher *restrict cycle)
{
    bool ret = false;

    for (uint8_t i = 0; i < nodes_len; ++i)
    {
        if (!nodes[i].visited)
        {
            CycleCribCipher temp = {0};
            Node *current = nodes + i;
            if (dfs_find_cycles(graph, current, NULL, current->data.crib_char, &temp))
            {
                if (temp.len_wo_stubs <= 1) continue;
                if (temp.len_w_stubs > cycle->len_w_stubs && temp.len_wo_stubs < NUM_SCRAMBLERS_PER_ROW)
                    memcpy(cycle, &temp, sizeof(CycleCribCipher));

                puts("\n\nCycle found");
                puts("W Stubs:");
                printf("Length: %d\n", temp.len_w_stubs);
                for (uint8_t y = 0; y < temp.len_w_stubs; ++y)
                {
                    printf("%u ", temp.positions_w_stubs[y]);
                }

                puts("\n\nWO Stubs:");
                printf("Length: %d\n", temp.len_wo_stubs);
                for (uint8_t y = 0; y < temp.len_wo_stubs; ++y)
                {
                    printf("%u ", temp.positions_wo_stubs[y]);
                }
                puts("\n");

                ret = true;
            }
        }
    }

    return ret;
}

void free_neighbours(const Node *restrict node)
{
    for (uint8_t i = 0; i < ALPHABET_SIZE; ++i)
    {
        if (node[i].neighbours != NULL)
            free(node[i].neighbours);
    }
}

void build_graph(Graph *restrict graph, const char *restrict crib, const char *restrict ciphertext, const size_t len, Node *nodes)
{
    for (uint8_t i = 0; i < len; ++i)
    {
        nodes[i].data.crib_char   = crib[i];
        nodes[i].data.cipher_char = ciphertext[i];
        nodes[i].data.position    = i;

        const uint8_t i_crib   = crib[i] - 'A';
        const uint8_t i_cipher = ciphertext[i] - 'A';

        graph->relations[i_crib][graph->indexing[i_crib]++]     = nodes + i;
        graph->relations[i_cipher][graph->indexing[i_cipher]++] = nodes + i;
    }
}

/**
 * @brief Find cycles between the crib and plain, using a graph and a modified DFS
 * @warning Uses restrict pointers: It must be assured that crib and ciphertext reside in different storage areas.
 * @param crib The crib
 * @param ciphertext The Ciphertext
 * @return Cycle: cycle if cycles where found, NULL for errors and no cycles found.
 */
CycleCribCipher* find_cycles_graph(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;

    Node nodes[ALPHABET_SIZE] = {0};
    Graph graph = {0};

    build_graph(&graph, crib, ciphertext, len, nodes);

    CycleCribCipher *cycle = malloc(sizeof(CycleCribCipher));
    assertmsg(cycle != NULL, "malloc failed");
    memset(cycle, 0, sizeof(CycleCribCipher));

    write_dot_format(crib, ciphertext);

    puts(find_cycle(&graph, nodes, len, cycle) ? "true" : "false");

    free_neighbours(nodes);

    if (cycle->len_w_stubs == 0)
    {
        free(cycle);
        return NULL;
    }

    return cycle;
}
