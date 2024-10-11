#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cycle_generator.h"
#include "enigma/enigma.h"
#include "helper/hashmap.h"

#define MESSAGE_SIZE               7

/*
 * Advice from Jürgen Wolfs C von A bis Z:
 * Claim is that const or constexpr is more performant than define when a calculation is involved...
 * And MSVC doesnt support that....
 */
//This was initially intended for an array holding all cycles. I leave it here.
#ifdef _MSC_VER
#define TOTAL_CYCLES (26 * 26 * 26 * 3 * 2 * 1)
#else
static const uint32_t TOTAL_CYCLES = 26 * 26 * 26 * 3 * 2 * 1;
#endif

#define NUM_ROTOR_PERMUTATIONS 6
#define BUFFER_SIZE            100

static uint8_t ring_settings[NUM_ROTORS_PER_ENIGMA] = {0, 0, 0};
static const uint8_t possible_rotor_permutations[NUM_ROTOR_PERMUTATIONS][NUM_ROTORS_PER_ENIGMA] = {
        {1, 2, 3},
        {1, 3, 2},
        {2, 1, 3},
        {2, 3, 1},
        {3, 1, 2},
        {3, 2, 1}
};

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
 * @return Cycle: A cycle of the rotor permutation
 */
static void calculate_cycle_lengths(const uint8_t *rotor_permutation, Cycle *restrict cycle)
{
    cycle->length = 0;

    bool visited[ALPHABET_SIZE] = {false};

    for (uint8_t base = 0; base < ALPHABET_SIZE; base++)
    {
        if (!visited[base])
        {
            visited[base] = true;
            uint8_t current = rotor_permutation[base];
            int32_t current_cycle_length = 1;

            while (current != base)
            {
                visited[current] = true;
                current = rotor_permutation[current];
                current_cycle_length++;
            }
            cycle->cycle_values[cycle->length++] = current_cycle_length;
        }
    }

    sort_cycles(cycle);

//    return cycle;
}

static void print_cycle(const Cycle *cycle, FILE *file)
{
    fwrite("( ", sizeof(char), 2, file);
    for (uint8_t i = 0; i < cycle->length; ++i)
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
    print_cycle(cycle->cycles + 0, file);
    fwrite(" / ", sizeof(char), 3, file);
    print_cycle(cycle->cycles + 1, file);
    fwrite(" / ", sizeof(char), 3, file);
    print_cycle(cycle->cycles + 2, file);

    fprintf(file, " : %c %c %c : ",
            cycle->rotor_positions[0] + 'A',
            cycle->rotor_positions[1] + 'A',
            cycle->rotor_positions[2] + 'A');

    fprintf(file, "%d %d %d\n",
            cycle->rotors[0],
            cycle->rotors[1],
            cycle->rotors[2]);
}

static size_t get_cycle_len_string(const Cycle *cycle, char *buffer, size_t offset)
{
    offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "(");

    for (uint8_t i = 0; i < cycle->length; ++i)
    {
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " %d", cycle->cycle_values[i]);
    }

    offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " )");

    return offset;
}

static void get_cycle_whole_lens_string(const CycleOfRotorSetting *cycle, char *buffer)
{
    size_t offset = 0;

    offset = get_cycle_len_string(cycle->cycles + 0, buffer, offset);
    offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " / ");
    offset = get_cycle_len_string(cycle->cycles + 1, buffer, offset);
    offset += snprintf(buffer + offset, BUFFER_SIZE - offset, " / ");
    offset = get_cycle_len_string(cycle->cycles + 2, buffer, offset);
}

static void get_enigma_settings_string(const CycleOfRotorSetting *cycle, char *buffer)
{
    size_t offset = 0;

    offset += snprintf(buffer + offset, BUFFER_SIZE - offset, "%c %c %c : %d %d %d",
                       cycle->rotor_positions[0] + 'A',
                       cycle->rotor_positions[1] + 'A',
                       cycle->rotor_positions[2] + 'A',
                       cycle->rotors[0],
                       cycle->rotors[1],
                       cycle->rotors[2]);
}

static void reset_enigma(Enigma *restrict enigma, const uint8_t *rotor_positions)
{
    for(uint8_t rotor = 0; rotor < NUM_ROTORS_PER_ENIGMA; ++rotor)
    {
        enigma->rotors[rotor]->position = rotor_positions[rotor];
    }
}

static void create_cycle(const CycleConfiguration *cycle_configuration, CycleOfRotorSetting *restrict cycle)
{
    const uint8_t *rotor_permutation = possible_rotor_permutations[cycle_configuration->rotor_permutation];

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
    char message[MESSAGE_SIZE] = "AAAAAA\0";
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

    uint8_t rotor_one_permutation[ALPHABET_SIZE] = {0};
    uint8_t rotor_two_permutation[ALPHABET_SIZE] = {0};
    uint8_t rotor_three_permutation[ALPHABET_SIZE] = {0};

    Enigma *enigma = create_enigma_from_configuration(&configuration);

    for (uint16_t letter = 0; letter < ALPHABET_SIZE; ++letter)
    {
        memset(enigma->plaintext, letter + 'A', 6);

        reset_enigma(enigma, rotor_positions);

        uint8_t *output = traverse_enigma(enigma);
        rotor_one_permutation[output[0]] = output[3];
        rotor_two_permutation[output[1]] = output[4];
        rotor_three_permutation[output[2]] = output[5];
        free(output);
    }

    free_enigma(enigma);
    calculate_cycle_lengths(rotor_one_permutation, cycle->cycles + 0);
    calculate_cycle_lengths(rotor_two_permutation, cycle->cycles + 1);
    calculate_cycle_lengths(rotor_three_permutation, cycle->cycles + 2);
}

void create_cycles(void)
{
    // 3 rotors and 26 possible settings for each rotor, 3 * 2 * 1 rotor permutations, 1 reflectors

    // CycleOfRotorSetting *cycles[TOTAL_CYCLES];

//    FILE *file;
//    assertmsg((file = fopen(FILE_PATH_CYCLO, "w")) != NULL, "can't open " FILE_PATH_CYCLO);

    CycleOfRotorSetting cycle = {0};

    HashMap hm = hm_create(150000);
    char cycle_len_buffer[BUFFER_SIZE];
    char enigma_settings_buffer[BUFFER_SIZE];

    for (uint8_t rotor_one_position = 0; rotor_one_position < ALPHABET_SIZE; ++rotor_one_position)
    {
        for (uint8_t rotor_two_position = 0; rotor_two_position < ALPHABET_SIZE; ++rotor_two_position)
        {
            for (uint8_t rotor_three_position = 0; rotor_three_position < ALPHABET_SIZE; ++rotor_three_position)
            {
                for (uint8_t rotor_permutation = 0; rotor_permutation < NUM_ROTOR_PERMUTATIONS;
                     rotor_permutation++)
                {
                    CycleConfiguration cycle_configuration = {
                            .rotor_one_position = rotor_one_position,
                            .rotor_two_position = rotor_two_position,
                            .rotor_three_position = rotor_three_position,
                            .rotor_permutation = rotor_permutation,
                            .reflector = UKW_A,
                    };

                    create_cycle(&cycle_configuration, &cycle);
                    get_cycle_whole_lens_string(&cycle, cycle_len_buffer);
                    get_enigma_settings_string(&cycle, enigma_settings_buffer);
                    hm_put(hm, cycle_len_buffer, enigma_settings_buffer);
//                    print_whole_cycle(&cycle, file);
                }
            }
        }
    }

    ValueList *vl = hm_get(hm, "( 10 10 1 1 1 1 1 1 ) / ( 12 12 1 1 ) / ( 10 10 2 2 1 1 )");
    assertmsg(vl != NULL, "couldn't retrieve value");

    for(size_t i = 0; i < vl->list_size; ++i)
    {
        puts(vl->values[i]);
    }

//    fclose(file);
    hm_destroy(hm);
    vl_destroy(vl);
    puts("Cycles have been written to: " FILE_PATH_CYCLO);
    printf("Total cycles: %d\n", TOTAL_CYCLES);
}
