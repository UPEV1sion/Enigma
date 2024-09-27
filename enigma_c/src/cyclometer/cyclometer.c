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

/*
 * Advice from Juergen Wolfs C von A bis Z:
 * Claim is that const or constexpr is more performant than define when a calculation is involved...
 * And MSVC doesnt support that....
 */
//This was initially intended for an array holding all cycles. I leave it here.
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
    uint8_t *cycle_values;
    int32_t length;
} Cycle;

typedef struct
{
    Cycle cycles[NUM_ROTORS_PER_ENIGMA];
    uint8_t rotor_positions[NUM_ROTORS_PER_ENIGMA];
    enum ROTOR_TYPE rotors[NUM_ROTORS_PER_ENIGMA];
} CycleOfRotorSetting;

typedef struct
{
    enum REFLECTOR_TYPE reflector;
    uint8_t rotor_one_position;
    uint8_t rotor_two_position;
    uint8_t rotor_three_position;
    uint8_t rotor_permutation;
} CycleConfiguration;

static void sort_cycles(const Cycle *cycle)
{
    // Sort the cycles.
    // QSort is NOT beneficial here.
    // Selection Sort -although it's O(n^2) algorithm- is the fastest one so far.
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
}


/**
 * @brief Get cycles by hopping through the rotor permutation using the next
 * index
 * @param rotor_permutation: Array containing the next index of the cycle
 * @return Cycle: A cycle of the rotor permutation
 */
static Cycle get_cycle_count(const uint8_t *rotor_permutation)
{
    Cycle cycle;
    cycle.length       = 0;
    cycle.cycle_values = malloc(ALPHABET_SIZE * sizeof(uint8_t));
    assertmsg(cycle.cycle_values != NULL, "cycle.cycle_values == NULL");

    bool visited[26] = {false};

    for (uint8_t i = 0; i < ALPHABET_SIZE; i++)
    {
        if (!visited[i])
        {
            const uint8_t base          = i;
            visited[base]                = true;
            uint8_t current              = rotor_permutation[base];
            int32_t current_cycle_length = 1;

            while (current != base)
            {
                visited[current] = true;
                current          = rotor_permutation[current];
                current_cycle_length++;
            }
            cycle.cycle_values[cycle.length++] = current_cycle_length;
        }
    }

    sort_cycles(&cycle);

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
    fwrite("( ", sizeof(char), 2, file);
    for (uint16_t i = 0; i < cycle->length; i++)
    {
        fprintf(file, "%d", cycle->cycle_values[i]);
        if (i < cycle->length - 1)
        {
            fwrite(" ", sizeof(char), 1, file);
        }
    }
    fwrite(" )", sizeof(char), 2, file);
}

static void print_whole_cycle(const CycleOfRotorSetting *cycle, FILE *file)
{
    print_cycle(&cycle->cycles[0], file);
    fwrite(" / ", sizeof(char), 3, file);
    print_cycle(&cycle->cycles[1], file);
    fwrite(" / ", sizeof(char), 3, file);
    print_cycle(&cycle->cycles[2], file);

    fprintf(file, " : %c %c %c : ",
            cycle->rotor_positions[0] + 'A',
            cycle->rotor_positions[1] + 'A',
            cycle->rotor_positions[2] + 'A');

    fprintf(file, "%d %d %d\n",
            cycle->rotors[0],
            cycle->rotors[1],
            cycle->rotors[2]);
}

static void create_cycle(const CycleConfiguration *cycle_configuration,
                         const uint8_t *rotor_permutation, CycleOfRotorSetting *cycle)
{
    enum ROTOR_TYPE rotors[NUM_ROTORS_PER_ENIGMA] = {
        rotor_permutation[0],
        rotor_permutation[1],
        rotor_permutation[2]
    };

    uint8_t rotor_positions[NUM_ROTORS_PER_ENIGMA] = {
        cycle_configuration->rotor_one_position,
        cycle_configuration->rotor_two_position,
        cycle_configuration->rotor_three_position
    };

    // Plugboard is implicitly the normal one
    char message[MESSAGE_SIZE]              = {0};
    const EnigmaConfiguration configuration = {
        .rotors = rotors,
        .rotor_positions = rotor_positions,
        .ring_settings = ring_settings,
        .type = ENIGMA_M3,
        .reflector = cycle_configuration->reflector,
        .message = message
    };

    memcpy(cycle->rotor_positions, rotor_positions, sizeof(rotor_positions));
    memcpy(cycle->rotors, rotors, NUM_ROTORS_PER_ENIGMA * sizeof(enum ROTOR_TYPE));

    uint8_t rotor_one_permutation[ALPHABET_SIZE]   = {0};
    uint8_t rotor_two_permutation[ALPHABET_SIZE]   = {0};
    uint8_t rotor_three_permutation[ALPHABET_SIZE] = {0};

    for (uint16_t letter = 0; letter < ALPHABET_SIZE; letter++)
    {
        memset(message, letter + 'A', 6);

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
}

static void free_cycle(const CycleOfRotorSetting *cycle)
{
    for (uint8_t j = 0; j < NUM_ROTORS_PER_ENIGMA; ++j)
    {
        free(cycle->cycles[j].cycle_values);
    }
}

void create_cycles(void)
{
    // 3 rotors and 26 possible settings for each rotor, 5 * 4 * 3 rotor
    // permutations, 2 reflectors

    // CycleOfRotorSetting *cycles[TOTAL_CYCLES];

    FILE *file                = open_file();
    CycleOfRotorSetting cycle = {0};

    for (uint8_t rotor_one_position = 0; rotor_one_position < ALPHABET_SIZE; ++rotor_one_position)
    {
        for (uint8_t rotor_two_position = 0; rotor_two_position < ALPHABET_SIZE; ++rotor_two_position)
        {
            for (uint8_t rotor_three_position = 0; rotor_three_position < ALPHABET_SIZE; ++rotor_three_position)
            {
                for (uint8_t rotor_permutation = 0; rotor_permutation < 3 * 2 * 1;
                     rotor_permutation++)
                {
                    CycleConfiguration cycle_configuration = {
                        .rotor_one_position = rotor_one_position,
                        .rotor_two_position = rotor_two_position,
                        .rotor_three_position = rotor_three_position,
                        .rotor_permutation = rotor_permutation,
                        .reflector = UKW_A
                    };

                    create_cycle(
                        &cycle_configuration,
                        possible_rotor_permutations[rotor_permutation],
                        &cycle);

                    print_whole_cycle(&cycle, file);
                    free_cycle(&cycle);
                }
            }
        }
    }

    fclose(file);
    puts("Cycles have been written to: " FILE_PATH_CYCLO);
    printf("Total cycles: %d\n", TOTAL_CYCLES);
}
