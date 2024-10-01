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

struct Node
{
    Node **neighbours;
    size_t neighbour_count;
    struct
    {
        char crib_char, cipher_char;
        uint8_t position;
    } data;
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

static bool find_cycle_rec(Node *restrict node, const Node *restrict parent, CycleGraph *restrict cycle)
{
    //FIXME doesnt recognize cycles correctly

    if (node->data.crib_char == 0) return false;

    printf("Visiting node: %c : %c, %u\n", node->data.crib_char, node->data.cipher_char, node->data.position);
    if (node->visited)
    {
        printf("Cycle found at:  %c : %c, %u\n", node->data.crib_char, node->data.cipher_char, node->data.position);
        return true;
    }
    node->visited = true;

    cycle->pos_cycle_w_stubs[cycle->len_w_stubs++]   = node->data.position;
    cycle->pos_cycle_wo_stubs[cycle->len_wo_stubs++] = node->data.position;

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

static bool find_cycle(Node *restrict nodes, const uint8_t nodes_len, CycleGraph *restrict cycle)
{
    bool ret = false;

    for (uint8_t i = 7; i < nodes_len; ++i)
    {
        if (!nodes[i].visited)
        {
            CycleGraph temp = {0};
            puts("New");
            if (find_cycle_rec(nodes + i, NULL, &temp))
            {
                if (temp.len_wo_stubs <= 1) continue;
                if (temp.len_w_stubs > cycle->len_w_stubs && temp.len_wo_stubs < NUM_SCRAMBLERS_PER_ROW)
                    memcpy(cycle, &temp, sizeof(CycleGraph));

                puts("W Stubs:");
                printf("Length: %d\n", temp.len_w_stubs);
                for (uint8_t y = 0; y < temp.len_w_stubs; ++y)
                {
                    printf("%u ", temp.pos_cycle_w_stubs[y]);
                }

                puts("\nWO Stubs:");
                printf("Length: %d\n", temp.len_wo_stubs);
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
    // This turns out to be a lot more complicated than I anticipated
    Node *relations_crib[ALPHABET_SIZE][ALPHABET_SIZE] = {0};
    Node *relations_cipher[ALPHABET_SIZE][ALPHABET_SIZE] = {0};
    uint8_t indexing_crib[ALPHABET_SIZE] = {0};
    uint8_t indexing_cipher[ALPHABET_SIZE] = {0};

    for (size_t i = 0; i < len; ++i)
    {
        Node *node = nodes + i;

        const char crib_char = crib[i];
        const char cipher_char = ciphertext[i];

        node->data.crib_char = crib_char;
        node->data.cipher_char = cipher_char;
        node->data.position = i;
        node->neighbours = NULL;
        node->neighbour_count = 0;
        node->visited = false;

        const uint8_t i_crib = crib_char - 'A';
        const uint8_t i_cipher = cipher_char - 'A';

        relations_crib[i_crib][indexing_crib[i_crib]++] = node;
        relations_cipher[i_cipher][indexing_cipher[i_cipher]++]= node;
    }

    for (size_t letter = 0; letter < len; ++letter)
    {
        for (uint8_t crib_node_i = 0; crib_node_i < indexing_crib[letter]; ++crib_node_i)
        {
            Node *current_crib_node = relations_crib[letter][crib_node_i];
            if(current_crib_node == NULL) continue;


            // Mapping every tuple with the same cipher char together.
            const uint8_t i_cipher = current_crib_node->data.cipher_char - 'A';
            for (uint8_t i = 0; i < indexing_cipher[i_cipher]; ++i)
            {
                Node *current_cipher_node = relations_cipher[i_cipher][i];
                if(current_cipher_node->data.position == current_crib_node->data.position) continue;
                add_neighbour(current_crib_node, current_cipher_node);
            }

            // Mapping every tuple with the same crib char together.
            const uint8_t i_crib = current_crib_node->data.crib_char - 'A';
            for (uint8_t i = 0; i < indexing_crib[i_crib]; ++i)
            {
                Node *current_cipher_node = relations_crib[i_crib][i];
                if(current_cipher_node->data.position == current_crib_node->data.position) continue;
                add_neighbour(current_crib_node, current_cipher_node);
            }

            // Mapping every tuple together, where crib and cipher char is swapped.
            const uint8_t i_crib_cipher = current_crib_node->data.crib_char - 'A';
            for (uint8_t i = 0; i < indexing_cipher[i_crib_cipher]; ++i)
            {
                Node *current_cipher_crib_node = relations_cipher[i_crib_cipher][i];
                if(current_cipher_crib_node->data.position == current_crib_node->data.position) continue;
                add_neighbour(current_crib_node, current_cipher_crib_node);
            }
        }
    }
    puts("");
}

/**
 * @brief Find cycles between the crib and plain, using a graph and a modified DFS
 * @warning Uses restrict pointers: It must be assured that crib and ciphertext reside in different storage areas.
 * @param crib The crib
 * @param ciphertext The Ciphertext
 * @return Cycle: cycle if cycles where found, NULL for error
 */
CycleGraph* find_cycles_graph(const char *restrict crib, const char *restrict ciphertext)
{
    size_t len;

    if ((len = strlen(ciphertext)) != strlen(crib)) return NULL;
    // if (len > NUM_SCRAMBLERS_PER_ROW) return NULL;

    Node nodes[ALPHABET_SIZE] = {0};

    build_graph(crib, ciphertext, len, nodes);

    // for (int i = 0; i < ALPHABET_SIZE; ++i)
    // {
    //     printf("Neigbours: %c : %c", nodes[i].data.crib_char, nodes[i].data.cipher_char);
    //     for(int j = 0; j < nodes[i].neighbour_count; ++j)
    //         printf("\t%c : %c", nodes[i].neighbours[j]->data.crib_char, nodes[i].neighbours[j]->data.cipher_char);
    //     puts("");
    // }

    CycleGraph *cycle = malloc(sizeof(CycleGraph));
    assertmsg(cycle != NULL, "malloc failed");
    memset(cycle, 0, sizeof(CycleGraph));

    write_dot_format(crib, ciphertext);

    puts(find_cycle(nodes, len, cycle) ? "true" : "false");

    free_neighbours(nodes);


    if (cycle->len_w_stubs == 0)
    {
        return NULL;
    }


    return cycle;
}
