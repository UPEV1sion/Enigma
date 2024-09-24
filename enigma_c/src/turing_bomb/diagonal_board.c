#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "diagonal_board.h"
#include "helper/helper.h"
#include "cycle_finder.h"

//
// Created by Emanuel on 30.08.2024.
//

#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVXYZ"

#define ERR_NO_CYCLES_FOUND 1

// static bool cycles_equals(const Cycles *candidate_cycles, const Cycles *current_cycles)
// {
//     if (candidate_cycles->num_cycles != current_cycles->num_cycles) return false;
//
//     bool *visited = calloc(candidate_cycles->num_cycles, sizeof(bool));
//     assertmsg(visited != NULL, "calloc failed");
//
//     for (uint16_t i = 0; i < candidate_cycles->num_cycles; ++i)
//     {
//         bool match = false;
//         for (uint16_t j = 0; j < current_cycles->num_cycles; ++j)
//         {
//             if (!visited[j] && strlen(candidate_cycles->cycles[j]) == strlen(current_cycles->cycles[i]))
//             {
//                 visited[j] = true;
//                 match      = true;
//                 break;
//             }
//         }
//         if (!match)
//         {
//             free(visited);
//             return false;
//         }
//     }
//     free(visited);
//     return true;
// }
//
// static bool stubs_equals(const Cycles *candidate_stubs, const Cycles *current_stubs)
// {
//     if (candidate_stubs->num_stubs != current_stubs->num_stubs) return false;
//
//     bool *visited = calloc(candidate_stubs->num_stubs, sizeof(bool));
//     assertmsg(visited != NULL, "calloc failed");
//
//     for (uint16_t i = 0; i < candidate_stubs->num_stubs; ++i)
//     {
//         bool match = false;
//         for (uint16_t j = 0; j < current_stubs->num_stubs; ++j)
//         {
//             if (!visited[j] && strlen(candidate_stubs->stubs[j]) == strlen(current_stubs->stubs[i]))
//             {
//                 visited[j] = true;
//                 match      = true;
//                 break;
//             }
//         }
//         if (!match)
//         {
//             free(visited);
//             return false;
//         }
//     }
//     free(visited);
//     return true;
// }
//
// static bool is_candidate(const Cycles *candidate_cycles, const Cycles *current_cycles)
// {
//     return cycles_equals(candidate_cycles, current_cycles) &&
//            stubs_equals(candidate_cycles, current_cycles);
// }

// static is_valid_graph_compare(char graph_compare[ALPHABET_SIZE])
// {
//     for(uint8_t i = 0; i < ALPHABET_SIZE; ++i)
//     {
//         // if(graph_compare[i] )
//     }
// }

static void init_diagonal_board(TuringBomb *turing_bomb)
{
    for(int8_t wire = 0; wire < ALPHABET_SIZE; ++wire)
    {
        memcpy(turing_bomb->diagonal_board[wire], ALPHABET, ALPHABET_SIZE);
    }
}

int32_t create_bomb_menu(TuringBomb *turing_bomb, const uint8_t *crib, const uint8_t *ciphertext, const size_t crib_len)
{
    init_diagonal_board(turing_bomb);

    Cycles *cycles = find_cycles(crib, ciphertext, crib_len);
    if(cycles == NULL || cycles->num_cycles == 0) return ERR_NO_CYCLES_FOUND;
    free(cycles);

    char *crib_str = get_string_from_int_array(crib, crib_len);
    char *ciphertext_str = get_string_from_int_array(ciphertext, crib_len);

    printf("");
    // char *longest_cycle = cycles->cycles[0];
    // size_t longest_cycle_length = 0;
    // for(uint8_t cycle = 1; cycle < cycles->num_cycles; ++cycle)
    // {
    //     char *current_cycle = cycles->cycles[cycle];
    //     const size_t current_cycle_length = strlen(current_cycle);
    //     if (current_cycle_length > longest_cycle_length)
    //     {
    //         longest_cycle_length = current_cycle_length;
    //         longest_cycle = current_cycle;
    //     }
    // }


    // char last_cycle_char = longest_cycle[0];

    // for(uint8_t i = 1; i < longest_cycle_length; ++i)
    // {
    //     // const uint8_t scrambler_column = longest_cycle.indexes_cycle[i];
    //     const char cycle_char = longest_cycle[i];
    //     // turing_bomb->diagonal_board[last_scrambler_column][(int) last_cycle_char] = cycle_char;
    //     // turing_bomb->diagonal_board[scrambler_column][(int) cycle_char] = last_cycle_char;
    //
    //     last_cycle_char = cycle_char;
    //     // last_scrambler_column = scrambler_column;
    // }


    return 0;
}

bool passes_welchman_test(const Cycles *candidate_cycles, const Cycles *current_cycles)
{
    // if (!is_candidate(candidate_cycles, current_cycles)) return false;
    char graph_compare[ALPHABET_SIZE] = {0};
    for(uint16_t cycle = 0; cycle < candidate_cycles->num_cycles; ++cycle)
    {

    }

    return false;
}
