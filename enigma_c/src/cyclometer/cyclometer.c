// ReSharper disable CppDFAMemoryLeak
// This to disable a false positive CLion memory leak warning. Tested multiple times with Valgrind.
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cyclometer.h"
#include "helper/helper.h"
#include "enigma/enigma.h"

#define NUM_ROTORS_PER_ENIGMA      3
#define MESSAGE_SIZE               7
#define PLUGBOARD                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/*
 * Advice from Juergen Wolfs C von A bis Z:
 * Claim is that const or constexpr is more performant than define when a calculation is involved...
 * And MSVC doesnt support that....
 */
#ifdef _MSC_VER
#define TOTAL_CYCLES (26 * 26 * 26 * 3 * 2 * 1)
#else
static const uint32_t TOTAL_CYCLES = 26 * 26 * 26 * 3 * 2 * 1;
#endif

static uint8_t ring_settings[3]                        = {0, 0, 0};
static const uint8_t possible_rotor_permutations[6][3] = {
    {1, 2, 3}, {1, 3, 2}, {2, 1, 3},
    {2, 3, 1}, {3, 1, 2}, {3, 2, 1}
};

typedef struct
{
    int32_t *cycle_values;
    int32_t length;
} Cycle;

typedef struct
{
    Cycle *cycles[3];
    uint8_t rotor_positions[3];
    enum ROTOR_TYPE rotors[3];
} CycleOfRotorSetting;

typedef struct
{
    enum REFLECTOR_TYPE reflector;
    uint8_t rotor_one_position;
    uint8_t rotor_two_position;
    uint8_t rotor_three_position;
    uint8_t rotor_permutation;
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

static void print_cycle(const Cycle *cycle, FILE *file)
{
    fprintf(file, "( ");
    for (uint16_t i = 0; i < cycle->length; i++)
    {
        fprintf(file, "%d", cycle->cycle_values[i]);

        if (i < cycle->length - 1)
        {
            fprintf(file, " ");
        }
    }
    fprintf(file, " )");
}

static void print_whole_cycle(const CycleOfRotorSetting *cycle, FILE *file)
{
    print_cycle(cycle->cycles[0], file);
    fprintf(file, " / ");
    print_cycle(cycle->cycles[1], file);
    fprintf(file, " / ");
    print_cycle(cycle->cycles[2], file);

    fprintf(file, ": %c %c %c: ", cycle->rotor_positions[0] + 'A',
            cycle->rotor_positions[1] + 'A', cycle->rotor_positions[2] + 'A');
    fprintf(file, "%d %d %d\n", cycle->rotors[0],
            cycle->rotors[1], cycle->rotors[2]);
}

static CycleOfRotorSetting* create_cycle(const CycleConfiguration *cycle_configuration,
                                         const uint8_t *rotor_permutation)
{
    CycleOfRotorSetting *cycle = malloc(sizeof(CycleOfRotorSetting));
    assertmsg(cycle != NULL, "cycle == NULL");

    enum ROTOR_TYPE rotors[NUM_ROTORS_PER_ENIGMA]  = {rotor_permutation[0], rotor_permutation[1], rotor_permutation[2]};
    uint8_t rotor_positions[NUM_ROTORS_PER_ENIGMA] = {
        cycle_configuration->rotor_one_position, cycle_configuration->rotor_two_position,
        cycle_configuration->rotor_three_position
    };
    char message[MESSAGE_SIZE]        = {0};
    EnigmaConfiguration configuration = {
        .rotors = rotors, .rotor_positions = rotor_positions, .ring_settings = ring_settings, .type = ENIGMA_M3,
        .reflector = cycle_configuration->reflector, .message = message
    };
    memcpy(configuration.plugboard, PLUGBOARD, sizeof(PLUGBOARD));

    memcpy(cycle->rotor_positions, rotor_positions, sizeof(rotor_positions));
    memcpy(cycle->rotors, rotors, NUM_ROTORS_PER_ENIGMA * sizeof(enum ROTOR_TYPE));

    int32_t rotor_one_permutation[ALPHABET_SIZE]   = {0};
    int32_t rotor_two_permutation[ALPHABET_SIZE]   = {0};
    int32_t rotor_three_permutation[ALPHABET_SIZE] = {0};

    for (uint16_t letter = 0; letter < 26; letter++)
    {
        memset(configuration.message, letter + 'A', 6);

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

    return cycle;
}

static void free_cycle(CycleOfRotorSetting *cycle)
{
    for (uint8_t j = 0; j < NUM_ROTORS_PER_ENIGMA; ++j)
    {
        free(cycle->cycles[j]->cycle_values);
        free(cycle->cycles[j]);
    }
    free(cycle);
}

void create_cycles(void)
{
    // 3 rotors and 26 possible settings for each rotor, 5 * 4 * 3 rotor
    // permutations, 2 reflectors

    // CycleOfRotorSetting *cycles[TOTAL_CYCLES];

    FILE *file         = open_file();
    int32_t iterations = 0;

    for (uint16_t rotor_one_position = 0; rotor_one_position < ALPHABET_SIZE; ++rotor_one_position)
    {
        for (uint16_t rotor_two_position = 0; rotor_two_position < ALPHABET_SIZE; ++rotor_two_position)
        {
            for (uint16_t rotor_three_position = 0; rotor_three_position < ALPHABET_SIZE; ++rotor_three_position)
            {
                for (uint16_t rotor_permutation = 0; rotor_permutation < 3 * 2 * 1;
                     rotor_permutation++)
                {
                    CycleConfiguration cycle_configuration = {
                        .rotor_one_position = rotor_one_position, .rotor_two_position = rotor_two_position,
                        .rotor_three_position = rotor_three_position,
                        .rotor_permutation = rotor_permutation, .reflector = UKW_A
                    };
                    CycleOfRotorSetting *cycle = create_cycle(
                        &cycle_configuration,
                        possible_rotor_permutations[rotor_permutation]);
                    print_whole_cycle(cycle, file);
                    free_cycle(cycle);
                    iterations++;
                }
            }
        }
    }

    fclose(file);
}
