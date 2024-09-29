#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "cycle_finder.h"
#include "helper/helper.h"

//
// Created by Emanuel on 29.09.2024.
//

typedef struct Node Node;

//TODO uint8_t
struct Node
{
    Node **neighbours;
    size_t neighbour_count;
    char data;
    bool visited;
};

static void add_neighbour(Node *restrict node, Node *restrict neighbour)
{
    node->neighbours = realloc(node->neighbours, (node->neighbour_count + 1) * sizeof(Node *));
    assertmsg(node->neighbours != NULL, "realloc failed");
    node->neighbours[node->neighbour_count] = neighbour;
    node->neighbour_count++;
}

static void print_dot(const char *restrict crib, const char *restrict ciphertext)
{
    puts("graph G {");
    puts("\tlayout=neato;");
    for (size_t i = 0; i < strlen(crib); ++i)
    {
        printf("\t\"%c\" -- \"%c\";\n", ciphertext[i], crib[i]);
    }
    puts("}");
}

static bool is_cycle_rec(Node *restrict node, Node *restrict parent) {
    printf("Visiting node: %c\n", node->data);
    if(node->visited) return true;
    node->visited = true;


    for(size_t i = 0; i < node->neighbour_count; ++i)
    {
        if (node->neighbours[i] != parent) {
            if(is_cycle_rec(node->neighbours[i], node))
                return true;
        }
    }

    node->visited = false;
    return false;
}

static bool find_cycle(Node *restrict cipher_nodes, const int len)
{

    for (int i = 0; i < len; ++i)
    {
        if (!cipher_nodes[i].visited)
        {
            if(is_cycle_rec(cipher_nodes + i, NULL))
                return true;
        }
    }

    return false;
}

void free_neighbours(const Node *restrict node)
{
    for (uint8_t i = 0; i < ALPHABET_SIZE; ++i)
    {
        if(node[i].neighbours != NULL)
            free(node[i].neighbours);
    }
}


int32_t find_cycles(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return 1;

    Node nodes[ALPHABET_SIZE] = {0};
    bool node_created[ALPHABET_SIZE] = {false};

    for (size_t i = 0; i < len; ++i)
    {
        const uint8_t i_crib = crib[i] - 'A';
        const uint8_t i_ciphertext = ciphertext[i] - 'A';

        if (!node_created[i_crib]) {
            nodes[i_crib].data = crib[i];
            node_created[i_crib] = true;
        }
        if (!node_created[i_ciphertext]) {
            nodes[i_ciphertext].data = ciphertext[i];
            node_created[i_ciphertext] = true;
        }

        add_neighbour(nodes + i_crib, nodes + i_ciphertext);
        add_neighbour(nodes + i_ciphertext, nodes + i_crib);
    }

    print_dot(crib, ciphertext);
    puts(find_cycle(nodes, len) ? "true" : "false");
    puts("");

    free_neighbours(nodes);

    return 0;
}
