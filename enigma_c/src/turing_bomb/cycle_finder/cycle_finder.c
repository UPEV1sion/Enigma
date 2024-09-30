#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "cycle_finder.h"
#include "helper/helper.h"
#include "turing_bomb/turing_bomb.h"

//
// Created by Emanuel on 29.09.2024.
//


struct Node
{
    Node **neighbours;
    size_t neighbour_count;
    char data;
    uint8_t crib_pos;
    bool visited;
};

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

static bool find_cycle_rec(Node *restrict node, const Node *restrict parent, Cycle *restrict cycle)
{
    if (node->data == 0) return false;

    printf("Visiting node: %c, %u\n", node->data, node->crib_pos);
    if (node->visited) return true;
    node->visited                                    = true;
    cycle->pos_cycle_w_stubs[cycle->len_w_stubs++]   = node->crib_pos;
    cycle->pos_cycle_wo_stubs[cycle->len_wo_stubs++] = node->crib_pos;

    for (size_t i = 0; i < node->neighbour_count; ++i)
    {
        if (node->neighbours[i] != parent)
        {
            if (find_cycle_rec(node->neighbours[i], node, cycle))
                return true;
        }
    }

    cycle->len_wo_stubs--;
    node->visited = false;

    return false;
}

static bool find_cycle(Node *restrict nodes, const uint8_t nodes_len, Cycle *restrict cycle)
{
    bool ret = false;

    for (uint8_t i = 0; i < nodes_len; ++i)
    {
        if (!nodes[i].visited)
        {
            Cycle temp = {0};
            if (find_cycle_rec(nodes + i, NULL, &temp))
            {
                if(temp.len_wo_stubs <= 1) continue;
                if(temp.len_w_stubs > cycle->len_w_stubs && temp.len_wo_stubs < NUM_SCRAMBLERS_PER_ROW)
                    memcpy(cycle, &temp, sizeof(Cycle));

                puts("W Stubs:");
                printf("%d\n", temp.len_w_stubs);
                for (uint8_t y = 0; y < temp.len_w_stubs; ++y)
                {
                    printf("%u ", temp.pos_cycle_w_stubs[y]);
                }

                puts("\nWO Stubs:");
                printf("%d\n", temp.len_wo_stubs);
                for (uint8_t y = 0; y < temp.len_wo_stubs; ++y)
                {
                    printf("%u ", temp.pos_cycle_wo_stubs[y]);
                }
                puts("");

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

void build_graph(const char *restrict crib, const char *restrict ciphertext, const size_t len, Node *nodes)
{
    bool node_created[ALPHABET_SIZE] = {false};

    for (size_t i = 0; i < len; ++i)
    {
        const uint8_t i_crib       = crib[i] - 'A';
        const uint8_t i_ciphertext = ciphertext[i] - 'A';

        if (!node_created[i_crib])
        {
            nodes[i_crib].data     = crib[i];
            nodes[i_crib].crib_pos = i;
            node_created[i_crib]   = true;
        }

        if (!node_created[i_ciphertext])
        {
            nodes[i_ciphertext].data     = ciphertext[i];
            nodes[i_ciphertext].crib_pos = i;
            node_created[i_ciphertext]   = true;
        }

        add_neighbour(nodes + i_crib, nodes + i_ciphertext);
        add_neighbour(nodes + i_ciphertext, nodes + i_crib);
    }
}

/**
 * @brief Find cycles between the crib and plain, using a graph and a modified DFS
 * @warning Uses restrict pointers: It must be assured that crib and ciphertext reside in different storage areas.
 * @param crib The crib
 * @param ciphertext The Ciphertext
 * @return Cycle: cycle if cycles where found, NULL for error
 */
Cycle* find_cycles(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;
    // if (len > NUM_SCRAMBLERS_PER_ROW) return NULL;

    Node nodes[ALPHABET_SIZE]        = {0};

    build_graph(crib, ciphertext, len, nodes);

    Cycle *cycle = malloc(sizeof(Cycle));
    assertmsg(cycle != NULL, "malloc failed");
    memset(cycle, 0, sizeof(Cycle));

    write_dot_format(crib, ciphertext);

    puts(find_cycle(nodes, len, cycle) ? "true" : "false");
    puts("");

    free_neighbours(nodes);

    if(cycle->len_w_stubs == 0)
    {
        free(cycle);
        return NULL;
    }


    return cycle;
}
