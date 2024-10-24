#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "cycle_finder_graph.h"
#include "turing_bomb/turing_bomb.h"

//
// Created by Emanuel on 29.09.2024.
//

//TODO bitmask for denoting stubs and cycles?

/*
 * I named this a graph approach, although im not "linking" them together.
 * This is more of a "HashMap" approach, but with dfs traversal.
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
 * When a cycle is found, all nodes in the cycle are not visited again.
 * But when no cycle is found, we iterate through the whole crib, performing a DFS at each position.
 * So the WC is O(n^2)
 *
 * A more realistic scenario is that a cycle is found, marking the visited nodes.
 * From testing, I can verify that the runtime is linear
 * for the case ciphertext and crib length is <= 26, which is always the case.
 */

typedef struct Node Node;

struct Node
{
    Node **neighbours;
    size_t neighbour_count;

    struct
    {
        char crib_char, cipher_char;
        uint8_t position, cycle_position;
    } data;

    bool visited;
};

typedef struct
{
    // Fixed size for now. Since this only resides on the Stack, I don't see any reason to dynamically allocate this.
    Node *relations[ALPHABET_SIZE][MAX_CRIB_LEN];
    uint8_t nodes_per_letter[ALPHABET_SIZE];
} Graph;

/**
 * @brief Establishes a connection from node to neighbour. Unidirectional
 * @warning deprecated, unused
 * @param node The node where the connection to neighbour should be established
 * @param neighbour The node with node should be linked to.
 */
//DEPRECATED("This function is deprecated & unused. Marked for removal")
//static void add_neighbour(Node *restrict node, Node *restrict neighbour)
//{
//    node->neighbours = realloc(node->neighbours, (node->neighbour_count + 1) * sizeof(Node *));
//    assertmsg(node->neighbours != NULL, "realloc failed");
//    node->neighbours[node->neighbour_count++] = neighbour;
//}

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
        // const char rotor1_char = i + 'A';
        // const char rotor2_char = i / 26 + 'A';
        // const char rotor3_char = i / 26 * 26 + 'A';
        // fprintf(file, "\t\"%c\" -- \"%c\" [label=\"%c%c%c\"];\n",
        //         ciphertext[i],
        //         crib[i],
        //         rotor3_char,
        //         rotor2_char,
        //         rotor1_char);
        fprintf(file, "\t\"%c\" -- \"%c\" [label=\"%zu\"];\n", ciphertext[i], crib[i], i);
    }
    fputs("}\n", file);

    fclose(file);
}

static void print_graph(const Graph *graph)
{
    for (uint8_t i = 0; i < ALPHABET_SIZE; ++i)
    {
        printf("%c: ", i + 'A');
        for (int j = 0; j < graph->nodes_per_letter[i]; ++j)
        {
            const Node *current = graph->relations[i][j];
            printf("[%c : %c, (%02d)] ", current->data.crib_char, current->data.cipher_char, current->data.position);
        }
        puts("");
    }
}

static inline bool is_matching_chars_tuple(const Node *first, const Node *second)
{
    return second != NULL && first->data.cipher_char == second->data.cipher_char
           && first->data.crib_char == second->data.crib_char;
}

static bool dfs_find_cycle(Graph *graph, Node *node,
                           const Node *parent, const char last_char,
                           CycleCribCipher *restrict cycle)
{
    if (node == NULL) return false;
    if (node->data.crib_char == 0) return false;
    if (node == parent) return false;

    if(is_matching_chars_tuple(node, parent)) return false;

    node->data.cycle_position = cycle->len_wo_stubs;
    // TODO continue cycle search after a tuple of tuples is found.
    // A tuple of tuples is a very powerful way to eliminate invalid plugboard settings
    // Turns out this is a well known compsci problem... maybe just denote it and continue?
    cycle->chars_w_stubs[cycle->len_w_stubs]         = last_char;
    cycle->chars_wo_stubs[cycle->len_wo_stubs]       = last_char;
    cycle->positions_w_stubs[cycle->len_w_stubs++]   = node->data.position;
    cycle->positions_wo_stubs[cycle->len_wo_stubs++] = node->data.position;

    if(is_matching_chars_tuple(node, parent))
    {
        return false;
    }

    // printf("Visiting node: %c : %c, %u\n", node->data.crib_char, node->data.cipher_char, node->data.position);
    if (node->visited)
        return true;

    node->visited = true;

    const char lookup_char      = (char) ((node->data.crib_char == last_char) ? node->data.cipher_char : node->data.crib_char);
    const uint8_t i_lookup_char = lookup_char - 'A';

    for (uint8_t i = 0; i < graph->nodes_per_letter[i_lookup_char]; ++i)
    {
        if (dfs_find_cycle(graph, graph->relations[i_lookup_char][i], node, lookup_char, cycle))
            return true;
    }

    cycle->len_wo_stubs--;
    node->visited = false;

    return false;
}

static bool find_cycle(Graph *graph, Node *nodes, const uint8_t nodes_len, CycleCribCipher *restrict cycle)
{
    bool ret = false;

    for (uint8_t i = 0; i < nodes_len; ++i)
    {
        if (!nodes[i].visited)
        {
            CycleCribCipher temp = {0};
            Node *current        = nodes + i;
            if (dfs_find_cycle(graph, current, NULL, current->data.crib_char, &temp))
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
                puts("");
                for (uint8_t y = 0; y < temp.len_w_stubs; ++y)
                {
                    printf("%c ", temp.chars_w_stubs[y]);
                }

                puts("\n\nWO Stubs:");
                printf("Length: %d\n", temp.len_wo_stubs);
                for (uint8_t y = 0; y < temp.len_wo_stubs; ++y)
                {
                    printf("%u ", temp.positions_wo_stubs[y]);
                }
                puts("");
                for (uint8_t y = 0; y < temp.len_wo_stubs; ++y)
                {
                    printf("%c ", temp.chars_wo_stubs[y]);
                }
                puts("\n");

                ret = true;
            }
        }
    }

    return ret;
}

DEPRECATED("Im not linking(allocating) them, so i must no free them.")
void free_neighbours(const Node *node)
{
    for (uint8_t i = 0; i < MAX_CRIB_LEN; ++i)
    {
        if (node[i].neighbours != NULL)
            free(node[i].neighbours);
    }
}

static void build_graph(Graph *restrict graph, const char *restrict crib, const char *restrict ciphertext, const size_t len,
                        Node *nodes)
{
    for (uint8_t i = 0; i < (uint8_t) len; ++i)
    {
        nodes[i].data.crib_char   = crib[i];
        nodes[i].data.cipher_char = ciphertext[i];
        nodes[i].data.position    = i;

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
CycleCribCipher* find_best_cycle_graph(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;
    assertmsg(len <= MAX_CRIB_LEN, "Try a shorter crib");

    Node nodes[MAX_CRIB_LEN] = {0};
    Graph graph              = {0};

    build_graph(&graph, crib, ciphertext, len, nodes);

    CycleCribCipher *cycle = malloc(sizeof(CycleCribCipher));
    assertmsg(cycle != NULL, "malloc failed");
    memset(cycle, 0, sizeof(CycleCribCipher));

    write_dot_format(crib, ciphertext);
    print_graph(&graph);

    puts(find_cycle(&graph, nodes, len, cycle) ? "true" : "false");
//    find_cycle(&graph, nodes, len, cycle);

    if (cycle->len_w_stubs == 0)
    {
        free(cycle);
        return NULL;
    }

    return cycle;
}
