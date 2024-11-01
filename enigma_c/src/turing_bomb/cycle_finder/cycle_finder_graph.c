#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "cycle_finder_graph.h"

//
// Created by Emanuel on 29.09.2024.
//

//TODO bitmask for denoting stubs and cycles?

/*
 * I named this a graph approach, although im not "linking" them together.
 * This is more of a "Hash Map" approach, but with dfs traversal.
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
            printf("[%c : %c, (%02d), v_c: %s, v_s: %s] ",
                   current->crib_char,
                   current->cipher_char,
                   current->position,
                   current->visited_cycle ? "true" : "false",
                   current->visited_stub ? "true" : "false");
        }
        puts("");
    }
}

static inline bool is_matching_chars_tuple(const Node *first, const Node *second)
{
    return second != NULL && first->cipher_char == second->cipher_char
           && first->crib_char == second->crib_char;
}

static bool dfs_find_cycle(Graph *graph, Node *node,
                           const Node *parent, const char last_char,
                           CyclePositions *restrict cycle)
{
    if (node == NULL) return false;
    if (node->crib_char == 0) return false;
    if (node == parent) return false;

    //TODO can be removed?
    if(is_matching_chars_tuple(node, parent)) return false;

    node->cycle_position = cycle->len_wo_stubs;
    // TODO continue cycle search after a tuple of tuples is found.
    // A tuple of tuples is a very powerful way to eliminate invalid plugboard settings
    // Turns out this is a well known compsci problem... maybe just denote it and continue?
    printf("%c : %c\n", node->crib_char, node->cipher_char);
    cycle->chars_w_stubs[cycle->len_w_stubs]         = last_char;
    cycle->chars_wo_stubs[cycle->len_wo_stubs]       = last_char;
    cycle->positions_w_stubs[cycle->len_w_stubs++]   = node->position;
    cycle->positions_wo_stubs[cycle->len_wo_stubs++] = node->position;

    // printf("Visiting node: %c : %c, %u\n", node->crib_char, node->cipher_char, node->position);
    if (node->visited_cycle)
        return true;

    node->visited_cycle = true;

    const char lookup_char      = (char) ((node->crib_char == last_char) ? node->cipher_char : node->crib_char);
    const uint8_t i_lookup_char = lookup_char - 'A';

    for (uint8_t i = 0; i < graph->nodes_per_letter[i_lookup_char]; ++i)
    {
        if (dfs_find_cycle(graph, graph->relations[i_lookup_char][i], node, lookup_char, cycle))
            return true;
    }

    cycle->len_wo_stubs--;
    node->visited_cycle = false;

    return false;
}

static bool find_cycle(Graph *graph, Node *nodes, const uint8_t nodes_len, CyclePositions *restrict cycle)
{
    bool ret = false;

    for (uint8_t i = 0; i < nodes_len; ++i)
    {
        if (!nodes[i].visited_cycle)
        {
            CyclePositions temp = {0};
            Node *current        = nodes + i;
            if (dfs_find_cycle(graph, current, NULL, current->crib_char, &temp))
            {
                if (temp.len_wo_stubs <= 1) continue;
                if (temp.len_w_stubs > cycle->len_w_stubs && temp.len_wo_stubs < NUM_SCRAMBLERS_PER_ROW)
                    memcpy(cycle, &temp, sizeof(CyclePositions));

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
                if(temp.len_wo_stubs >= nodes_len / 2) return ret;
            }
        }
    }

    return ret;
}

static void build_graph(Graph *restrict graph, const char *restrict crib, const char *restrict ciphertext, const size_t len,
                        Node *nodes)
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
 * @brief Recursively marking all adjacent nodes
 * @param graph The graph containing all nodes
 * @param character The current character in the cycle
 */
static void mark_stubs_rec(Graph *restrict graph, const uint8_t character)
{
    for (uint8_t i = 0; i < graph->nodes_per_letter[character]; ++i)
    {
        Node *current_node = graph->relations[character][i];
        if (current_node->visited_cycle || current_node->visited_stub) continue;
        current_node->visited_stub = true;
        const char lookup_char      = (char) ((current_node->crib_char == character) ?
                                                current_node->cipher_char : current_node->crib_char);
        const uint8_t i_lookup_char = lookup_char - 'A';
        mark_stubs_rec(graph, i_lookup_char);
    }
}

/**
 * @brief Marks all stubs adjacent to the cycle
 * @param graph The graph containing all nodes
 * @param cycle The cycle without the stubs found
 */
static void mark_stubs(Graph *restrict graph, const CyclePositions *cycle)
{
    const char *cycle_chars = cycle->chars_wo_stubs;
    uint8_t current_char;
    while((current_char = *cycle_chars++) != 0)
    {
        mark_stubs_rec(graph, current_char - 'A');
    }
    puts("");
}

void free_cycle(Cycle *cycle)
{
    free(cycle->positions);
    for(uint8_t node = 0; node < cycle->len_nodes_wo_stubs; ++node)
    {
        free(cycle->nodes_wo_stubs[node]);
    }
    // for(uint8_t letter = 0; letter < ALPHABET_SIZE; ++letter)
    // {
    //     for (uint8_t node = 0; node < cycle->graph->nodes_per_letter[letter]; ++node)
    //     {
    //         free(cycle->graph->relations[letter][node]);
    //     }
    // }
    free(cycle->graph);
    free(cycle);
}

/**
 * @brief Find cycles between the crib and plain, using a graph and a modified DFS
 * @warning Uses restrict pointers: It must be assured that crib and ciphertext reside in different storage areas.
 * @param crib The crib
 * @param ciphertext The Ciphertext
 * @return Cycle: cycle if cycles where found, NULL for errors and no cycles found.
 */
Cycle* find_longest_cycle_graph(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;
    assertmsg(len <= MAX_CRIB_LEN, "Try a shorter crib");

    Cycle *cycle = malloc(sizeof(Cycle));
    assertmsg(cycle != NULL, "malloc failed");

    Node *nodes  = malloc(sizeof(Node) * len);
    assertmsg(nodes != NULL, "malloc failed");
    Graph *graph = malloc(sizeof (Graph));
    assertmsg(graph != NULL, "malloc failed");
    cycle->graph = graph;

    build_graph(graph, crib, ciphertext, len, nodes);

    CyclePositions *cycle_pos = malloc(sizeof(CyclePositions));
    memset(cycle_pos, 0, sizeof (CyclePositions));
    assertmsg(cycle_pos != NULL, "malloc failed");
    cycle->positions = cycle_pos;


    write_dot_format(crib, ciphertext);
    print_graph(graph);

    puts(find_cycle(graph, nodes, len, cycle_pos) ? "true" : "false");
//    find_cycle(&graph, nodes, len, cycle);
    mark_stubs(graph, cycle_pos);
    print_graph(graph);

    //TODO remove unnecessary nodes or count the necessary ones...

    if (cycle_pos->len_w_stubs == 0)
    {
        fprintf(stderr, "No cycles found. Try a different crib.\n");
        free_cycle(cycle);
        return NULL;
    }
    if(cycle_pos->len_w_stubs > NUM_SCRAMBLERS_PER_ROW)
    {
        fprintf(stderr, "Cycle too long. Try a different crib.\n");
        free_cycle(cycle);
        return NULL;
    }


    return cycle;
}
