#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "diagonal_board.h"
#include "helper/helper.h"

//
// Created by Emanuel on 30.08.2024.
//

bool cycles_equals(const Cycles *candidate_cycles, const Cycles *current_cycles)
{
    if (candidate_cycles->num_cycles != current_cycles->num_cycles) return false;

    bool *visited = calloc(candidate_cycles->num_cycles, sizeof(bool));
    assertmsg(visited != NULL, "calloc failed");

    for (uint16_t i = 0; i < candidate_cycles->num_cycles; ++i)
    {
        bool match = false;
        for (uint16_t j = 0; j < current_cycles->num_cycles; ++j)
        {
            if (!visited[j] && strlen(candidate_cycles->cycles[j]) == strlen(current_cycles->cycles[i]))
            {
                visited[j] = true;
                match      = true;
                break;
            }
        }
        if (!match)
        {
            free(visited);
            return false;
        }
    }
    free(visited);
    return true;
}

bool stubs_equals(const Cycles *candidate_stubs, const Cycles *current_stubs)
{
    if (candidate_stubs->num_stubs != current_stubs->num_stubs) return false;

    bool *visited = calloc(candidate_stubs->num_stubs, sizeof(bool));
    assertmsg(visited != NULL, "calloc failed");

    for (uint16_t i = 0; i < candidate_stubs->num_stubs; ++i)
    {
        bool match = false;
        for (uint16_t j = 0; j < current_stubs->num_stubs; ++j)
        {
            if (!visited[j] && strlen(candidate_stubs->stubs[j]) == strlen(current_stubs->stubs[i]))
            {
                visited[j] = true;
                match      = true;
                break;
            }
        }
        if (!match)
        {
            free(visited);
            return false;
        }
    }
    free(visited);
    return true;
}

bool is_candidate(const Cycles *candidate_cycles, const Cycles *current_cycles)
{
    return cycles_equals(candidate_cycles, current_cycles) &&
           stubs_equals(candidate_cycles, current_cycles);
}

bool passes_welchman_test(const Cycles *candidate_cycles, const Cycles *current_cycles)
{
    if (!is_candidate(candidate_cycles, current_cycles)) return false;
    char graph_compare[ALPHABET_SIZE] = {0};



    // TODO Order cycles?
    return false;
}
