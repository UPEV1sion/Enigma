//
// Created by escha on 1/1/25.
//

#include "server_cyclometer.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "enigma/enigma.h"

#define MESSAGE_SIZE               7

/*
 * Advice from Jürgen Wolfs C von A bis Z:
 * Claim is that const or constexpr is more performant than define when a calculation is involved...
 * And MSVC doesnt support that....
 */
//This was initially intended for an array holding all cycles. I leave it here.

#define BUFFER_SIZE            100

static void sort_cycles(Cycle *restrict cycle)
{
    // Sort the cycles.
    // QSort is NOT beneficial here.
    // Selection Sort -although an O(n^2) algorithm- is the fastest one so far.
    for (uint16_t i = 0; i < cycle->length; i++)
    {
        uint16_t max_index = i;
        for (uint16_t j = i + 1; j < cycle->length; j++)
        {
            if (cycle->cycle_values[j] > cycle->cycle_values[max_index])
            {
                max_index = j;
            }
        }
        if (max_index != i)
        {
            const int32_t temp = cycle->cycle_values[i];
            cycle->cycle_values[i] = cycle->cycle_values[max_index];
            cycle->cycle_values[max_index] = temp;
        }
    }
}

/**
 * @brief Get cycles by hopping through the rotor_permutation using the next
 * index
 * @param rotor_permutation: Array containing the next index of the cycle
 * @param cycle: struct, where the values should be stored
 * @return Cycle: A cycle of the rotor permutation
 */
static void calculate_cycle_lengths(const int16_t *rotor_permutation, Cycle *restrict cycle)
{
    cycle->length = 0;

    bool visited[ALPHABET_SIZE] = {false};

    for (int16_t base = 0; base < ALPHABET_SIZE; base++)
    {
        if (!visited[base])
        {
            visited[base] = true;
            int16_t current = rotor_permutation[base];
            int32_t current_cycle_length = 1;

            while (current != base && current != -1)
            {
                visited[current] = true;
                current          = rotor_permutation[current];
                current_cycle_length++;
            }
            if (current != -1)
                cycle->cycle_values[cycle->length++] = current_cycle_length;
        }
    }
    sort_cycles(cycle);
}

static void print_cycle(const Cycle *cycle, FILE *file)
{
    fwrite("(", sizeof(char), 1, file);
    for (uint8_t i = 0; i < cycle->length; ++i)
    {
        fprintf(file, " %d", cycle->cycle_values[i]);
    }
    fwrite(" )", sizeof(char), 2, file);
}

static void print_whole_cycle(const Cycle *cycle, FILE *file)
{
    print_cycle(cycle + 0, file);
    fwrite(" / ", sizeof(char), 3, file);
    print_cycle(cycle + 1, file);
    fwrite(" / ", sizeof(char), 3, file);
    print_cycle(cycle + 2, file);
}

// static size_t get_cycle_len_string(const Cycle *cycle, char *buffer, size_t offset)
// {
//     offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "(");
//
//     for (uint8_t i = 0; i < cycle->length; ++i)
//     {
//         offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " %d", cycle->cycle_values[i]);
//     }
//
//     offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " )");
//
//     return offset;
// }

// static void get_cycle_whole_lens_string(const Cycle *cycle, char *buffer)
// {
//     size_t offset = 0;
//
//     offset = get_cycle_len_string(cycle + 0, buffer, offset);
//     offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " / ");
//     offset = get_cycle_len_string(cycle + 1, buffer, offset);
//     offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " / ");
//     offset = get_cycle_len_string(cycle + 2, buffer, offset);
// }

Cycle* server_create_cycles(char **enc_daily_keys, const int32_t daily_key_count)
{
    Cycle *cycles = malloc(sizeof(Cycle) * 3);
    assertmsg(cycles != NULL, "malloc failed");

    int16_t rotor_one_permutation[ALPHABET_SIZE];
    int16_t rotor_two_permutation[ALPHABET_SIZE];
    int16_t rotor_three_permutation[ALPHABET_SIZE];
    memset(rotor_one_permutation, -1, ALPHABET_SIZE * sizeof(int16_t));
    memset(rotor_two_permutation, -1, ALPHABET_SIZE * sizeof(int16_t));
    memset(rotor_three_permutation, -1, ALPHABET_SIZE * sizeof(int16_t));

    for (int32_t i = 0; i < daily_key_count; ++i)
    {
        uint8_t *key_as_int = get_int_array_from_string(enc_daily_keys[i]);

        rotor_one_permutation[key_as_int[0]] = (int16_t) (key_as_int[DAILY_KEY_SIZE]);
        // printf("Rot 1: %c -> %c\n", key_as_int[0] + 'A', key_as_int[DAILY_KEY_SIZE] + 'A');
        rotor_two_permutation[key_as_int[1]] = (int16_t) (key_as_int[DAILY_KEY_SIZE + 1]);
        // printf("Rot 2: %c -> %c\n", key_as_int[1] + 'A', key_as_int[DAILY_KEY_SIZE + 1] + 'A');
        rotor_three_permutation[key_as_int[2]] = (int16_t) (key_as_int[DAILY_KEY_SIZE + 2]);
        // printf("Rot 3: %c -> %c\n", key_as_int[2] + 'A', key_as_int[DAILY_KEY_SIZE + 2] + 'A');
        free(key_as_int);
    }

    calculate_cycle_lengths(rotor_one_permutation, cycles + 0);
    calculate_cycle_lengths(rotor_two_permutation, cycles + 1);
    calculate_cycle_lengths(rotor_three_permutation, cycles + 2);
    print_whole_cycle(cycles, stdout);
    fflush(stdout);
    return cycles;
}
