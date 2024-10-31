//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdbool.h>
//
//#include "helper/helper.h"
//#include "cycle_finder.h"
//
//#include "turing_bomb/turing_bomb.h"
//
////
//// Created by Emanuel on 30.08.2024.
////
//
///* After my research, I found that the process of finding cycles between crib and ciphertext was actually done by hand
// * as well as plugging up the diagonal board.
// *
// * Here I solved the "cycle finding problem" using a recursive backtracking algorithm.
// * This algorithm has explosive runtime and shall not be used.
// * Use cycle_finder_graph for almost linear runtime.
// */
//
//typedef struct
//{
//    uint8_t first;
//    uint8_t second;
//} CharTuple;
//
///**
// * @brief Test if there is a cycle between plaintext and crib when starting from a particular char in plain
// * using a backtracking algorithm.
// * @note If im referring to letter, I actually mean a digit from 0-25 representing it.
// * @param start the start position of the potential cycle
// * @param c the other letter from the tuple where a letter is matched
// * @param tuples array with all the letter tuples
// * @param tuples_len the array length
// * @param visited_mask the tuples already visited_cycle as a bitmask
// * @param cycle to note down the cycle path and positions
// * @return bool: true or falsehood for cycle or stub
// */
//static bool find_cycle(const uint8_t start, const uint8_t c,
//                       CharTuple *restrict tuples, const uint8_t tuples_len,
//                       uint32_t visited_mask, CycleCribCipher *restrict cycle)
//{
//    // Bitmask instead of a bool array for speed and minimizing recursion overhead
//
//    /*  Once again for the interested reader:
//     *  Instead of using a boolean array to track which Tuples are visited_cycle,
//     *  I used a bitmask where each bit corresponds to a Tuple visited_cycle.
//     *  Note: crib can't be longer than 26 chars.
//     *  Before the function call, we mark the corresponding bit of the "entry point" as active using an OR operation.
//     *  In the function, we annotate the path and traverse through the tuples,
//     *  retrieving the next letter which is not already visited_cycle.
//     *  For the visited_cycle check we bitshift the bit into the wanted position and using an AND operation to see if its toggled.
//     *  After we retrieved the next letter, we set the corresponding bit.
//     *  If backtracking goes back, we disable the bit with a more complex bit manipulation:
//     *  The bit gets shifted into the right place, and then the whole mask gets flipped with a NOT operation.
//     *  Now all bits are 1 except the bit we want to disable.
//     *  We then use an AND which leaves the whole visited_mask as it is but disables the unwanted bit.
//     */
//
//    cycle->chars_w_stubs[cycle->len_w_stubs]   = c;
//    cycle->chars_wo_stubs[cycle->len_wo_stubs] = c;
//
//    if (c == start && cycle->len_w_stubs > 1)
//    {
//        return true;
//    }
//
//    for (uint8_t i = 0; i < tuples_len; ++i)
//    {
//        if (!(visited_mask & (1 << i)))
//        {
//            // I know...
//            const uint8_t next_letter = (tuples[i].first == c)
//                                            ? tuples[i].second
//                                            : (tuples[i].second == c)
//                                                  ? tuples[i].first
//                                                  : 0;
//            if (next_letter != 0)
//            {
//                visited_mask |= (1 << i);
//
//                cycle->positions_w_stubs[cycle->len_w_stubs++]   = i;
//                cycle->positions_wo_stubs[cycle->len_wo_stubs++] = i;
//
//                if (find_cycle(start, next_letter, tuples, tuples_len, visited_mask, cycle))
//                {
//                    return true;
//                }
//
//                visited_mask &= ~(1 << i);
//            }
//        }
//    }
//
//    cycle->len_wo_stubs--;
//
//    return false;
//}
//
///**
// * @brief Writes the cycles to FILE_PATH_CRIB_CIPHER_CYCLE
// * @param crib The crib
// * @param ciphertext The ciphertext
// */
//static void write_dot_format(const char *restrict crib, const char *restrict ciphertext)
//{
//    FILE *file;
//    assertmsg((file = fopen(FILE_PATH_CRIB_CIPHER_CYCLE, "w")) != NULL, "cant open" FILE_PATH_CRIB_CIPHER_CYCLE);
//
//    fputs("graph G {\n", file);
//    fputs("\tlayout=neato;\n", file);
//    const size_t len = strlen(crib);
//    for (size_t i = 0; i < len; ++i)
//    {
//        fprintf(file, "\t\"%c\" -- \"%c\";\n", ciphertext[i], crib[i]);
//    }
//    fputs("}\n", file);
//
//    fclose(file);
//}
//
//
///**
// * @brief Finds cycles between crib and ciphertext
// * @note plain and crib length must be equal
// * @warning Deprecated. Do not use this! This has an explosive runtime with longer cribs. Use cycle_finder_graph
// * @param crib the crib suspected to be the deciphered ciphertext
// * @param ciphertext the enciphered text
// * @return Cycles*
// */
//DEPRECATED("This function is deprecated. Use find_longest_cycle_graph() instead.")
//CyclesCribCipher* find_cycles(const char *restrict crib, const char *restrict ciphertext)
//{
//    write_dot_format(crib, ciphertext);
//
//    CyclesCribCipher *cycles = malloc(sizeof(CyclesCribCipher));
//    assertmsg(cycles != NULL, "malloc failed");
//    memset(cycles, 0, sizeof(CyclesCribCipher));
//
//    const size_t crib_len = strlen(crib);
//
//    CharTuple tuples[MAX_CRIB_LEN];
//    for (size_t i = 0; i < crib_len; i++)
//    {
//        tuples[i].first  = crib[i] - 'A';
//        tuples[i].second = ciphertext[i] - 'A';
//    }
//
//    size_t cycle_counter = 0;
//
//    for (size_t i = 0; i < crib_len; i++)
//    {
//        //TODO marking the already visited_cycle ones, so that they dont get traversed again -> reference maybe
//        const uint32_t visited_mask = (1 << i);
//
//        CycleCribCipher temp               = {0};
//        temp.positions_w_stubs[0]   = i;
//        temp.positions_wo_stubs[0]  = i;
//        temp.chars_w_stubs[0]       = tuples[i].first;
//        temp.chars_wo_stubs[0]      = tuples[i].first;
//        temp.len_w_stubs = 1;
//        temp.len_wo_stubs = 1;
//
//        if (find_cycle(tuples[i].first, tuples[i].second, tuples, crib_len, visited_mask, &temp))
//        {
//            if (temp.len_wo_stubs <= 1) continue;
//            cycles->cycles_positions[cycle_counter] = malloc(sizeof(CycleCribCipher));
//            assertmsg(cycles->cycles_positions[cycle_counter] != NULL, "malloc failed");
//
//            memcpy(cycles->cycles_positions[cycle_counter], &temp, sizeof(CycleCribCipher));
//            // puts("\n\nFound cycle:");
//            // puts("WO STUBS:");
//            // printf("Length %d\n", temp.len_wo_stubs);
//            // for (uint8_t cycle_pos = 0; cycle_pos < temp.len_wo_stubs; ++cycle_pos)
//            // {
//            //     printf("%d (%c) -> ", temp.positions_wo_stubs[cycle_pos], temp.chars_wo_stubs[cycle_pos] + 'A');
//            // }
//            // puts("\n\nW STUBS:");
//            // printf("Length %d\n", temp.len_w_stubs);
//            // for (uint8_t cycle_pos = 0; cycle_pos < temp.len_w_stubs; ++cycle_pos)
//            // {
//            //     printf("%d (%c) -> ", temp.positions_w_stubs[cycle_pos], temp.chars_w_stubs[cycle_pos] + 'A');
//            // }
//            // puts("\n");
//
//            cycle_counter++;
//        }
//    }
//    cycles->num_cycles = cycle_counter;
//
//    return cycles;
//}
