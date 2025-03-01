//
// Created by escha on 1/1/25.
//

#include "server_cyclometer.h"
#include <stdint.h>
#include <stdbool.h>

static void sort_cycles(int *cycle_values, int cycle_len)
{
    // Sort the cycles.
    // QSort is NOT beneficial here.
    // Selection Sort -although an O(n^2) algorithm- is the fastest one so far.
    for (int i = 0; i < cycle_len; i++)
    {
        int max_index = i;
        for (int j = i + 1; j < cycle_len; j++)
        {
            if (cycle_values[j] > cycle_values[max_index])
            {
                max_index = j;
            }
        }
        if (max_index != i)
        {
            const int temp = cycle_values[i];
            cycle_values[i] = cycle_values[max_index];
            cycle_values[max_index] = temp;
        }
    }
}

static void calculate_cycle_len(const int *rotor_permutation, int *total_cycle_len, int *cycle_values)
{
    *total_cycle_len = 0;

    bool visited[ALPHABET_SIZE] = {false};

    for (int base = 0; base < ALPHABET_SIZE; base++)
    {
        if (!visited[base])
        {

            visited[base] = true;
            int current = rotor_permutation[base];
            int current_cycle_length = 1;

            while (current != base && current != -1)
            {
                visited[current] = true;
                current          = rotor_permutation[current];
                current_cycle_length++;
            }
            if (current != -1)
            {
                cycle_values[*total_cycle_len] = current_cycle_length;
                (*total_cycle_len)++;
            }
        }
    }

    sort_cycles(cycle_values, *total_cycle_len);
}

void add_daily_key_to_permutations(RotorPermutations *permutations, const uint8_t *daily_key_as_int)
{
    permutations->rotor_one_permutations[daily_key_as_int[0]] = daily_key_as_int[DAILY_KEY_SIZE];
    permutations->rotor_two_permutations[daily_key_as_int[1]] = daily_key_as_int[DAILY_KEY_SIZE + 1];
    permutations->rotor_three_permutations[daily_key_as_int[2]] = daily_key_as_int[DAILY_KEY_SIZE + 2];
}

void server_compute_cycles(const RotorPermutations *permutations, ComputedCycles *restrict cycles)
{
    calculate_cycle_len(permutations->rotor_one_permutations, &cycles->cycles_1_4_len, cycles->cycles_1_4);
    calculate_cycle_len(permutations->rotor_two_permutations, &cycles->cycles_2_5_len, cycles->cycles_2_5);
    calculate_cycle_len(permutations->rotor_three_permutations, &cycles->cycles_3_6_len, cycles->cycles_3_6);
}
