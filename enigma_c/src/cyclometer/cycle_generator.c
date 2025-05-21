#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "cycle_generator.h"
#include "helper/helper.h"
#include "cyclometer.h"
#include "enigma/enigma.h"

//
// Created by Emanuel on 09.10.24.
//

#define ALPHABET "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

typedef struct
{
    char rotor_one_permutations[ALPHABET_SIZE];
    char rotor_two_permutations[ALPHABET_SIZE];
    char rotor_three_permutations[ALPHABET_SIZE];
    uint8_t permutations_stored;
} PermutationTables;

static void add_to_permutation_table(const char *message_key, PermutationTables *restrict tables)
{
//    if(tables->permutations_stored == 26)
//    {
//        puts("There are already enough permutations found");
//        return;
//    }

    tables->rotor_one_permutations[message_key[0] - 'A'] = message_key[3];
    tables->rotor_two_permutations[message_key[1] - 'A'] = message_key[4];
    tables->rotor_three_permutations[message_key[2] - 'A'] = message_key[5];
    tables->permutations_stored++;
}

static char *generate_message_key(void)
{
    char *message_key = malloc(7);

    for (uint8_t i = 0; i < 3; i++)
    {
        message_key[i + 3] = message_key[i] = ALPHABET[rand() % ALPHABET_SIZE];
    }


    message_key[6] = 0;

    return message_key;
}

static void sort_cycles(uint8_t *cycle_lens, const uint8_t len)
{
    for (uint8_t i = 0; i < len; ++i)
    {
        for (uint8_t j = i + 1; j < len; ++j)
        {
            if(cycle_lens[j] > cycle_lens[i])
            {
                uint8_t temp = cycle_lens[i];
                cycle_lens[i] = cycle_lens[j];
                cycle_lens[j] = temp;
            }
        }
    }
}

static void find_cycle_lens(const char *rotor_permutation)
{
    bool visited[ALPHABET_SIZE] = {false};
    uint8_t cycle_lens[ALPHABET_SIZE] = {false};
    uint8_t cycle_count = 0;

    for (uint8_t c = 0; c < ALPHABET_SIZE; ++c)
    {
        printf("%c ", rotor_permutation[c]);
    }
    puts("");

    for (uint8_t base = 0; base < ALPHABET_SIZE; ++base)
    {
        if(!visited[base])
        {
            uint8_t current = rotor_permutation[base] - 'A';
            visited[base] = true;
            int32_t current_cycle_length = 1;
            while (base != current)
            {
                visited[current] = true;
                current = rotor_permutation[current] - 'A';
                current_cycle_length++;
            }
            cycle_lens[cycle_count++] = current_cycle_length;
        }
    }

    sort_cycles(cycle_lens, cycle_count);

    for(uint8_t cycle = 0; cycle < cycle_count; ++cycle)
    {
        printf("%d ", cycle_lens[cycle]);
    }
    puts("");
}

static void print_all_cycle_len(const PermutationTables *permut_tables)
{

    find_cycle_lens(permut_tables->rotor_one_permutations);
    find_cycle_lens(permut_tables->rotor_two_permutations);
    find_cycle_lens(permut_tables->rotor_three_permutations);
}

void test_permut_table_builder(void)
{
    uint8_t rot_pos[3] = {4, 7, 1};
    uint8_t ring_pos[3] = {0, 0, 0};
    enum ROTOR_TYPE rotors[3] = {ROTOR_1, ROTOR_3, ROTOR_2};
    PermutationTables tables = {0};

    for (uint8_t message_count = 0; message_count < 200; ++message_count)
    {
        char *message_key = generate_message_key();

        EnigmaConfiguration conf = {
                .rotor_positions = rot_pos,
                .ring_settings = ring_pos,
                .rotors = rotors,
                .reflector = UKW_A,
                .type = ENIGMA_M3,
                .message = message_key
        };

        Enigma *enigma = create_enigma_from_configuration(&conf);
        uint8_t *output = traverse_enigma(enigma);
        char *mes = get_string_from_int_array(output, 6);
        add_to_permutation_table(mes, &tables);

        free(output);
        free(message_key);
        free(mes);
    }

    print_all_cycle_len(&tables);
}
