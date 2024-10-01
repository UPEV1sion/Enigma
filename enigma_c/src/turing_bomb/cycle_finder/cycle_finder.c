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
 * In cycle_finder_graph I tested an approach with a graph and DFS,
 * but it turns out that setting up the graph correctly isn't as easy as it seems, and it comes with great overhead.
 * cycle_finder_graph was actually the second method I tested, after this one.
 * But in the end, I settled for this one.
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
} CharTuple;

/**
 * @brief Test if there is a cycle between plaintext and crib when starting from a particular char in plain
 * using a backtracking algorithm.
 * @note If im referring to letter, I actually mean a digit from 0-25 representing it.
 * @param start the start position of the potential cycle
 * @param c the other letter from the tuple where a letter is matched
 * @param tuples array with all the letter tuples
 * @param tuples_len the array length
 * @param visited_mask the tuples already visited as a bitmask
 * @param cycle to note down the cycle path and positions
 * @return bool: true or falsehood for cycle or stub
 */
static bool find_cycle(const uint8_t start, const uint8_t c,
                       CharTuple *restrict tuples, const uint8_t tuples_len,
                       uint32_t visited_mask, CycleCribCipher *restrict cycle)
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

    cycle->chars_w_stubs[cycle->len_w_stubs]   = c;
    cycle->chars_wo_stubs[cycle->len_wo_stubs] = c;

    if (c == start && cycle->len_w_stubs > 1)
    {
        return true;
    }

    for (uint8_t i = 0; i < tuples_len; ++i)
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

                cycle->positions_w_stubs[++cycle->len_w_stubs]   = i;
                cycle->positions_wo_stubs[++cycle->len_wo_stubs] = i;

                if (find_cycle(start, next_letter, tuples, tuples_len, visited_mask, cycle))
                {
                    return true;
                }

                visited_mask &= ~(1 << i);
            }
        }
    }

    cycle->len_wo_stubs--;

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
 * @brief Writes the cycles to FILE_PATH_CRIB_CIPHER_CYCLE
 * @param crib The crib
 * @param ciphertext The ciphertext
 */
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


/**
 * @brief Finds cycles between crib and ciphertext
 * @note plain and crib length must be equal
 * @param crib the crib suspected to be the deciphered ciphertext
 * @param ciphertext the enciphered text
 * @return Cycles*
 */
CyclesCribCipher* find_cycles(const char *restrict crib, const char *restrict ciphertext)
{
    write_dot_format(crib, ciphertext);

    CyclesCribCipher *cycles = malloc(sizeof(CyclesCribCipher));
    assertmsg(cycles != NULL, "malloc failed");
    memset(cycles, 0, sizeof(CyclesCribCipher));

    const size_t crib_len = strlen(crib);

    CharTuple tuples[ALPHABET_SIZE];
    for (size_t i = 0; i < crib_len; i++)
    {
        tuples[i].first  = crib[i] - 'A';
        tuples[i].second = ciphertext[i] - 'A';
    }

    size_t cycle_counter = 0;

    for (size_t i = 0; i < crib_len; i++)
    {
        //TODO marking the already visited ones, so that they dont get traversed again -> reference maybe
        const uint32_t visited_mask = (1 << i);

        CycleCribCipher temp               = {0};
        temp.positions_w_stubs[0]   = i;
        temp.positions_wo_stubs[0]  = i;
        temp.chars_w_stubs[0]       = tuples[i].first;
        temp.chars_wo_stubs[0]      = tuples[i].first;
        temp.len_w_stubs = 1;
        temp.len_wo_stubs = 1;

        if (find_cycle(tuples[i].first, tuples[i].second, tuples, crib_len, visited_mask, &temp))
        {
            if (temp.len_wo_stubs <= 1) continue;
            cycles->cycles_positions[cycle_counter] = malloc(sizeof(CycleCribCipher));
            assertmsg(cycles->cycles_positions[cycle_counter] != NULL, "malloc failed");

            memcpy(cycles->cycles_positions[cycle_counter], &temp, sizeof(CycleCribCipher));
            puts("\nFound cycle:");
            puts("WO STUBS:");
            for (uint8_t cycle_pos = 0; cycle_pos <= temp.len_wo_stubs; ++cycle_pos)
            {
                printf("%d (%c) -> ", temp.positions_wo_stubs[cycle_pos], temp.chars_wo_stubs[cycle_pos] + 'A');
            }
            puts("\nW STUBS:");
            for (uint8_t cycle_pos = 0; cycle_pos <= temp.len_w_stubs; ++cycle_pos)
            {
                printf("%d (%c) -> ", temp.positions_w_stubs[cycle_pos], temp.chars_w_stubs[cycle_pos] + 'A');
            }

            cycle_counter++;
        }
    }
    cycles->num_cycles = cycle_counter;

    return cycles;
}
