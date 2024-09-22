#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "helper/helper.h"
#include "cycle_finder.h"

//
// Created by Emanuel on 30.08.2024.
//

/* A very useful resource was a YouTube video by Gustav Vogels named
 * TURING WELCHMAN BOMBE - Beschreibung des kompletten Entschlüsselungsverfahrens der ENIGMA.
 * In which explained the inner workings of the bombe in great depth and detail.
 * https://youtu.be/7pOsBhwwmhI
*/

// This is the heart of the turing bomb. And currently solved with a recursive backtracking algorithm.
// TODO find a iterable way of doing thinks because of speed benefits.
// TODO cycle approach like cyclometer?

typedef struct
{
    char first;
    char second;
} Tuple;

static void print_cycle(const char *cycle, const size_t cycle_length)
{
    printf("Cycle detected: ");
    for (size_t i = 0; i < cycle_length; i++)
    {
        printf("%c -> ", cycle[i]);
    }
    // printf("%c\n", cycle[0]);
}

static char* get_cycle_str(const char *cycle, const size_t cycle_length)
{
    char *cycle_str = malloc(cycle_length + 1);
    assertmsg(cycle_str != NULL, "malloc failed");
    for (size_t i = 0; i < cycle_length; i++)
    {
        cycle_str[i] = cycle[i];
    }
    cycle_str[cycle_length] = 0;

    return cycle_str;
}

static char* get_stub_str(const char *stub)
{
    const size_t stub_length = strlen(stub);
    char *stub_str           = malloc(stub_length + 1);
    assertmsg(stub_str != NULL, "malloc failed");
    for (size_t i = 0; i < stub_length; i++)
    {
        stub_str[i] = stub[i];
    }
    stub_str[stub_length] = 0;

    return stub_str;
}

/**
 * @brief Test if there is a cycle between plaintext and crib when starting from a particular char in plain
 * using a backtracking algorithm.
 * @param start the start position of the potential cycle
 * @param c the other letter from the tuple where a letter is matched
 * @param tuples array with all the letter tuples
 * @param tuples_len the array length
 * @param visited the tuples already visited
 * @param cycle_path to annotate the cycle path
 * @param cp_index index for cycle_path
 * @return bool: true or falsehood for cycle or stub
 */
static bool is_cycle(const char start, const char c, Tuple *tuples, const size_t tuples_len, bool *visited,
                     char *cycle_path,
                     size_t *cp_index)
{
    cycle_path[(*cp_index)++] = c;
    if (c == start)
    {
        // printing for debugging
        // print_cycle(cycle_path, *cp_index);
        return true;
    }
    for (size_t i = 0; i < tuples_len; ++i)
    {
        if (!visited[i])
        {
            if (tuples[i].first == c)
            {
                visited[i] = true;
                if (is_cycle(start, tuples[i].second, tuples, tuples_len, visited, cycle_path, cp_index))
                    return true;
                visited[i] = false;
            }
            if (tuples[i].second == c)
            {
                visited[i] = true;
                if (is_cycle(start, tuples[i].first, tuples, tuples_len, visited, cycle_path, cp_index))
                    return true;
                visited[i] = false;
            }
        }
    }
    (*cp_index)--;
    return false;
}


/**
 * @brief Test if there is a cycle between plaintext and crib when starting from a particular char in plain
 * using a mix of iterative and recursive approach.
 * @param start the start position of the potential cycle
 * @param c the other letter from the tuple where a letter is matched
 * @param tuples array with all the letter tuples
 * @param tuples_len the array length
 * @param visited_bits the visited chars as a bitmask
 * @param cycle_length the cycle length
 * @return bool: true or falsehood for cycle or stub
 */
static bool is_cycle_iter(const char start, const char c, Tuple *tuples, const size_t tuples_len, uint32_t *visited_bits, size_t *cycle_length)
{
    uint32_t visited_mask = *visited_bits;
    visited_mask |= 1 << (c - 'A');
    *visited_bits = visited_mask;
    (*cycle_length)++;

    if (c == start)
        {
        return true;
    }

    for (size_t i = 0; i < tuples_len; ++i)
    {
        const char next_char = (tuples[i].first == c) ? tuples[i].second :
                         (tuples[i].second == c) ? tuples[i].first : 0;

        if (next_char != 0 && !(visited_mask & 1 << (next_char - 'A'))) {
            if (is_cycle_iter(start, next_char, tuples, tuples_len, visited_bits, cycle_length)) {
                return true;
            }
        }
    }

    *cycle_length = 0;
    return false;
}

/**
 * @brief Eliminates duplicate cycles where there loop is the same
 * but started at a different point or is traversed backwards
 * @param cycles the char * array containing the cycles
 * @param num_cycles the number of cycles found
 * @return size_t: num of cycles left
 */
static size_t eliminate_duplicates(char *cycles[], const size_t num_cycles)
{
    size_t valid_cycles = 0;
    for (size_t i = 0; i < num_cycles; ++i)
    {
        if (cycles[i] != NULL)
        {
            for (size_t j = i + 1; j < num_cycles; ++j)
            {
                if (is_permutation(cycles[i], cycles[j]))
                {
                    free(cycles[j]);
                    cycles[j] = NULL;
                }
            }
            cycles[valid_cycles++] = cycles[i];
        }
    }
    return valid_cycles;
}

/**
 * @brief Finds cycles between crib and ciphertext
 * @note plain and crib length must be equal
 * @param crib the crib suspected to be the deciphered ciphertext
 * @param ciphertext the enciphered text
 * @return void
 */
Cycles* find_cycles(const char *crib, const char *ciphertext)
{
    Cycles *res = malloc(sizeof(Cycles));
    memset(res->cycles, 0, sizeof(res->cycles));
    memset(res->stubs, 0, sizeof(res->stubs));

    const size_t crib_len = strlen(crib);

    Tuple tuples[ALPHABET_SIZE];
    for (size_t i = 0; i < crib_len; i++)
    {
        tuples[i].first  = crib[i];
        tuples[i].second = ciphertext[i];
    }

    size_t cycle_counter = 0;
    size_t stubs_counter = 0;
    for (size_t i = 0; i < crib_len; i++)
    {
        bool visited[ALPHABET_SIZE] = {false};
        char path[ASCII_SIZE]       = {0};
        visited[i]                  = true;
        size_t cp_index             = 0;
        if (is_cycle(tuples[i].first, tuples[i].second, tuples, crib_len, visited, path, &cp_index))
        {
            res->cycles[cycle_counter++] = get_cycle_str(path, cp_index);
        }
        else
        {
            res->stubs[stubs_counter++] = get_stub_str(path);
        }
    }
    cycle_counter = eliminate_duplicates(res->cycles, cycle_counter);
    for (size_t i = 0; i < stubs_counter; i++)
    {
        puts(res->stubs[i]);
    }
    puts("cycles");
    for (size_t i = 0; i < cycle_counter; i++)
    {
        puts(res->cycles[i]);
    }
    res->num_cycles = cycle_counter;
    res->num_stubs  = stubs_counter;
    return res;
}
