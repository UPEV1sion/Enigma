#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cyclometer.h"
#include "helper/helper.h"
#include "enigma/enigma.h"

#define NUM_ROTORS      3
#define MESSAGE_SIZE    7
#define PLUGBOARD       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/*
 * Advice from Juergen Wolfs C von A bis Z:
 * Claim is that const or constexpr is more performant than define when a calculation is involved
 */
static const uint32_t TOTAL_CYCLES                 = 26 * 26 * 26 * 3 * 2 * 1;
static const uint8_t ring_settings[3]                  = {0, 0, 0};
static const uint8_t possible_rotor_permutations[6][3] = {
    {1, 2, 3}, {1, 3, 2}, {2, 1, 3},
    {2, 3, 1}, {3, 1, 2}, {3, 2, 1}
};

typedef struct
{
    int32_t length;
    int32_t *cycle_values;
} Cycle;

typedef struct
{
    Cycle *cycles[3];
    int32_t rotor_positions[3];
    int32_t rotors[3];
} CycleOfRotorSetting;

typedef struct
{
    int32_t rotor_one;
    int32_t rotor_two;
    int32_t rotor_three;
    int32_t rotor_permutation;
    char reflector_index;
} CycleConfiguration;


/**
 * @brief Get cycles by hopping through the rotor permutation using the next
 * index
 * @param rotor_permutation: Array containing the next index of the cycle
 * @return Cycle: A cycle of the rotor permutation
 */
static Cycle* get_cycle_count(const int32_t *rotor_permutation)
{
    Cycle *cycle = malloc(sizeof(Cycle));
    assertmsg(cycle != NULL, "malloc failed");
    cycle->length       = 0;
    cycle->cycle_values = malloc(ALPHABET_SIZE * sizeof(int32_t));
    assertmsg(cycle->cycle_values != NULL, "cycle.cycle_values == NULL");

    bool visited[26] = {false};

    for (uint16_t i = 0; i < 26; i++)
    {
        if (!visited[i])
        {
            const int32_t base           = i;
            visited[base]                = true;
            int32_t current              = rotor_permutation[base];
            int32_t current_cycle_length = 1;
            // TODO: cycles should be sorted

            while (current != base)
            {
                visited[current] = true;
                current          = rotor_permutation[current];
                current_cycle_length++;
            }
            cycle->cycle_values[cycle->length] = current_cycle_length;
            cycle->length++;
        }
    }

    // Sort the cycles. QSort is NOT beneficial here. Selection Sort is the fastest one so far.
    for (uint16_t i = 0; i < cycle->length; i++)
    {
        for (uint16_t j = i + 1; j < cycle->length; j++)
        {
            if (cycle->cycle_values[i] < cycle->cycle_values[j])
            {
                const int32_t temp     = cycle->cycle_values[i];
                cycle->cycle_values[i] = cycle->cycle_values[j];
                cycle->cycle_values[j] = temp;
            }
        }
    }
    return cycle;
}

static FILE* open_file(void)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_CYCLO, "w")) != NULL, "file == NULL");

    return file;
}

static void print_cycle(const Cycle cycle, FILE *file)
{
    fprintf(file, "( ");
    for (uint16_t i = 0; i < cycle.length; i++)
    {
        fprintf(file, "%d", cycle.cycle_values[i]);

        if (i < cycle.length - 1)
        {
            fprintf(file, " ");
        }
    }
    fprintf(file, " )");
}

static CycleOfRotorSetting* create_cycle(const CycleConfiguration *cycle_configuration,
                                         const uint8_t *rotor_permutation, FILE *file)
{
    CycleOfRotorSetting *cycle = malloc(sizeof(CycleOfRotorSetting));
    assertmsg(cycle != NULL, "cycle == NULL");

    uint8_t rotors[NUM_ROTORS]              = {0};
    uint8_t rotor_positions[NUM_ROTORS]     = {0};
    char message[MESSAGE_SIZE]              = {0};
    const EnigmaConfiguration configuration = {
        rotors, rotor_positions, ring_settings, M3, 'B', .message = message
    };
    memcpy(configuration.plugboard, PLUGBOARD, sizeof(PLUGBOARD));

    configuration.rotor_positions[0] = cycle_configuration->rotor_one;
    configuration.rotor_positions[1] = cycle_configuration->rotor_two;
    configuration.rotor_positions[2] = cycle_configuration->rotor_three;
    cycle->rotor_positions[0]        = cycle_configuration->rotor_one;
    cycle->rotor_positions[1]        = cycle_configuration->rotor_two;
    cycle->rotor_positions[2]        = cycle_configuration->rotor_three;

    // FIXME: Rotor 1,1,1 should not be possible, values have to be unique
    configuration.rotors[0] = rotor_permutation[0];
    configuration.rotors[1] = rotor_permutation[1];
    configuration.rotors[2] = rotor_permutation[2];
    cycle->rotors[0]        = rotor_permutation[0];
    cycle->rotors[1]        = rotor_permutation[1];
    cycle->rotors[2]        = rotor_permutation[2];

    int32_t rotor_one_permutation[ALPHABET_SIZE]   = {0};
    int32_t rotor_two_permutation[ALPHABET_SIZE]   = {0};
    int32_t rotor_three_permutation[ALPHABET_SIZE] = {0};

    for (uint16_t letter = 0; letter < 26; letter++)
    {
        configuration.message[0] = (char) (letter + 'A');
        configuration.message[1] = (char) (letter + 'A');
        configuration.message[2] = (char) (letter + 'A');
        configuration.message[3] = (char) (letter + 'A');
        configuration.message[4] = (char) (letter + 'A');
        configuration.message[5] = (char) (letter + 'A');

        Enigma *enigma = create_enigma_from_configuration(&configuration);

        uint8_t *output                    = traverse_enigma(enigma);
        rotor_one_permutation[output[0]]   = output[3];
        rotor_two_permutation[output[1]]   = output[4];
        rotor_three_permutation[output[2]] = output[5];
        free(output);
        free_enigma(enigma);
    }

    cycle->cycles[0] = get_cycle_count(rotor_one_permutation);
    cycle->cycles[1] = get_cycle_count(rotor_two_permutation);
    cycle->cycles[2] = get_cycle_count(rotor_three_permutation);

    //TODO: implement mmap files platform independent to yield faster results
    print_cycle(*cycle->cycles[0], file);
    fprintf(file, " / ");
    print_cycle(*cycle->cycles[1], file);
    fprintf(file, " / ");
    print_cycle(*cycle->cycles[2], file);

    fprintf(file, ": %d %d %d: ", cycle_configuration->rotor_one,
            cycle_configuration->rotor_two, cycle_configuration->rotor_three);
    fprintf(file, "%d %d %d\n", configuration.rotors[0],
            configuration.rotors[1], configuration.rotors[2]);

    return cycle;
}

void create_cycles(void)
{
    // 3 rotors and 26 possible settings for each rotor, 5 * 4 * 3 rotor
    // permutations, 2 reflectors

    CycleOfRotorSetting *cycles[TOTAL_CYCLES];

    FILE *file         = open_file();
    int32_t iterations = 0;

    for (uint16_t rotor_one = 0; rotor_one < ALPHABET_SIZE; rotor_one++)
    {
        for (uint16_t rotor_two = 0; rotor_two < ALPHABET_SIZE; rotor_two++)
        {
            for (uint16_t rotor_three = 0; rotor_three < ALPHABET_SIZE; rotor_three++)
            {
                for (uint16_t rotor_permutation = 0; rotor_permutation < 3 * 2 * 1;
                     rotor_permutation++)
                {
                    CycleConfiguration cycle_configuration = {
                        rotor_one, rotor_two, rotor_three, rotor_permutation, 'B'
                    };
                    cycles[iterations] = create_cycle(
                        &cycle_configuration,
                        possible_rotor_permutations[rotor_permutation], file);
                    // free(cycle_configuration);
                    iterations++;
                }
            }
        }
    }

    fclose(file);

    for (uint32_t i = 0; i < TOTAL_CYCLES; i++)
    {
        for (uint32_t j = 0; j < NUM_ROTORS; ++j)
        {
            free(cycles[i]->cycles[j]->cycle_values);
            free(cycles[i]->cycles[j]);
        }
        free(cycles[i]);
    }
}
