#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "helper/helper.h"
#include "cycle_finder.h"

//
// Created by Emanuel on 30.08.2024.
//

/* After my research, I found that the process of finding cycles between crib and ciphertext was actually done by hand
 * as well as plugging up the diagonal board.
 *
 * Here I solved the "cycle finding problem" using a recursive backtracking algorithm.
 * This algorithm has been extensively optimized and is very fast.
 * It also runs only once in the decryption process.
 * It May run multiple times until the Enigma is cracked due to bad crib placement.
 */

typedef struct
{
    uint8_t first;
    uint8_t second;
} Tuple;

// static void print_cycle(const char *cycle, const size_t cycle_length)
// {
//     printf("Cycle detected: ");
//     for (size_t i = 0; i < cycle_length; i++)
//     {
//         printf("%c -> ", cycle[i]);
//     }
//      printf("%c\n", cycle[0]);
// }

static char* get_cycle_str(const int8_t *cycle, const size_t cycle_length)
{
    char *cycle_str = malloc(cycle_length + 1);
    assertmsg(cycle_str != NULL, "malloc failed");
    for (size_t i = 0; i < cycle_length; i++)
    {
        cycle_str[i] = cycle[i] + 'A';
    }
    cycle_str[cycle_length] = 0;

    return cycle_str;
}

/**
 * @brief finds the length of a -1 terminated string
 * @param str the string literal
 * @return the length
 */
static size_t strlen_neg1(const int8_t *str)
{
    const int8_t *start = str;
    while (*str != -1) str++;

    return str - start;
}

static char* get_stub_str(const int8_t *stub)
{
    const size_t stub_length = strlen_neg1(stub);
    char *stub_str           = malloc(stub_length + 1);
    assertmsg(stub_str != NULL, "malloc failed");
    for (size_t i = 0; i < stub_length; i++)
    {
        stub_str[i] = stub[i] + 'A';
    }
    stub_str[stub_length] = 0;

    return stub_str;
}

/**
 * @brief Test if there is a cycle between plaintext and crib when starting from a particular char in plain
 * using a backtracking algorithm.
 * @note If im referring to letter, I actually mean a digit from 0-25 representing it.
 * @param start the start position of the potential cycle
 * @param c the other letter from the tuple where a letter is matched
 * @param tuples array with all the letter tuples
 * @param tuples_len the array length
 * @param visited_mask the tuples already visited as a bitmask
 * @param cycle_path to annotate the cycle path
 * @param cp_index index for cycle_path
 * @return bool: true or falsehood for cycle or stub
 */
static bool is_cycle(const uint8_t start, const uint8_t c, Tuple *tuples, const size_t tuples_len,
                             uint32_t visited_mask,
                             int8_t *cycle_path, size_t *cp_index)
{
    // Bitmask instead of a bool array for speed and minimizing recursion overhead

    /*  Once again for the interested reader:
     *  Instead of using a boolean array to track which Tuples are visited,
     *  I used a bitmask where each bit corresponds to a Tuple visited.
     *  Note: crib can't be longer than 26 chars.
     *  Before the function call, we mark the corresponding bit of the "entry point" as active using an OR operation.
     *  In the function, we annotate the path and traverse through the tuples,
     *  retrieving the next letter which is not already visited.
     *  For the visited check we bitshift the bit into the wanted position and using an AND operation to see if its toggled.
     *  After we retrieved the next letter, we set the corresponding bit.
     *  If backtracking goes back, we disable the bit with a more complex bit manipulation:
     *  The bit gets shifted into the right place, and then the whole mask gets flipped with a NOT operation.
     *  Now all bits are 1 except the bit we want to disable.
     *  We then use an AND which leaves the whole visited_mask as it is but disables the unwanted bit.
     */

    cycle_path[(*cp_index)++] = c;
    if (c == start)
    {
        return true;
    }
    for (size_t i = 0; i < tuples_len; ++i)
    {
        if (!(visited_mask & (1 << i)))
        {
            // I know...
            const uint8_t next_letter = (tuples[i].first == c)
                                       ? tuples[i].second
                                       : (tuples[i].second == c)
                                             ? tuples[i].first
                                             : 0;
            if (next_letter != 0)
            {
                visited_mask |= (1 << i);
                if (is_cycle(start, next_letter, tuples, tuples_len, visited_mask, cycle_path, cp_index))
                {
                    return true;
                }
                visited_mask &= ~(1 << i);
            }
        }
    }
    (*cp_index)--;
    return false;
}
// TODO marked for removal
// /**
//  * @brief Eliminates duplicate cycles where there loop is the same
//  * but started at a different point or is traversed backwards
//  * @param cycles the char * array containing the cycles
//  * @param num_cycles the number of cycles found
//  * @return size_t: num of cycles left
//  */
// static size_t eliminate_duplicate_cycles(char *cycles[], const size_t num_cycles)
// {
//     size_t valid_cycles = 0;
//     for (size_t i = 0; i < num_cycles; ++i)
//     {
//         if (cycles[i] != NULL)
//         {
//             for (size_t j = i + 1; j < num_cycles; ++j)
//             {
//                 if (is_permutation(cycles[i], cycles[j]))
//                 {
//                     free(cycles[j]);
//                     cycles[j] = NULL;
//                 }
//             }
//             cycles[valid_cycles++] = cycles[i];
//         }
//     }
//     return valid_cycles;
// }

/**
 * @brief Finds cycles between crib and ciphertext
 * @note plain and crib length must be equal
 * @param crib the crib suspected to be the deciphered ciphertext
 * @param ciphertext the enciphered text
 * @param crib_len the length of the crib
 * @return void
 */
Cycles* find_cycles(const uint8_t *crib, const uint8_t *ciphertext, const size_t crib_len)
{

    Cycles *res = malloc(sizeof(Cycles));
    memset(res->cycles, 0, sizeof(res->cycles));
    // memset(res->stubs, 0, sizeof(res->stubs));

    // const size_t crib_len = strlen(crib);

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
        uint32_t visited_mask = 0;
        int8_t path[ALPHABET_SIZE];
        memset(path, -1, ALPHABET_SIZE); //since 'A' is represented by 0, we must use a different terminator.
        visited_mask |= (1 << i);
        size_t cp_index = 0;
        if (is_cycle(tuples[i].first, tuples[i].second, tuples, crib_len, visited_mask, path, &cp_index))
        {
            res->cycles[cycle_counter++] = get_cycle_str(path, cp_index);
        }
        else
        {
            res->stubs[stubs_counter++] = get_stub_str(path);
        }
    }
    // cycle_counter = eliminate_duplicate_cycles(res->cycles, cycle_counter);
    puts("stubs");
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
