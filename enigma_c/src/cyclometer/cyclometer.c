#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "cyclometer.h"
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

#define NUM_ROTORS             5
#define NUM_ROTOR_PERMUTATIONS 6
#define BUFFER_SIZE            100

static uint8_t ring_settings[NUM_ROTORS_PER_ENIGMA] = {0, 0, 0};

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
    uint8_t *rotor_permutation;
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
 * @param cycle: struct, where the values should be stored
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
}

static void print_cycle(const Cycle *cycle, FILE *file)
{
    fprintf(file, "{%d", cycle->cycle_values[0]);
    for (uint8_t i = 1; i < cycle->length; ++i)
    {
        fprintf(file, ", %d", cycle->cycle_values[i]);
    }
    fwrite("};", sizeof(char), 2, file);
}

static void print_whole_cycle(const CycleOfRotorSetting *cycle, FILE *file)
{
    print_cycle(cycle->cycles + 0, file);
    print_cycle(cycle->cycles + 1, file);
    print_cycle(cycle->cycles + 2, file);

    fprintf(file, "%c%c%c;",
            cycle->rotor_positions[0] + 'A',
            cycle->rotor_positions[1] + 'A',
            cycle->rotor_positions[2] + 'A');

    fprintf(file, "{%d, %d, %d}\n",
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
    get_cycle_len_string(cycle->cycles + 2, buffer, offset);
}

static void get_enigma_settings_string(const CycleOfRotorSetting *cycle, char *buffer)
{
    snprintf(buffer, BUFFER_SIZE, "%c %c %c : %d %d %d",
             cycle->rotor_positions[0] + 'A',
             cycle->rotor_positions[1] + 'A',
             cycle->rotor_positions[2] + 'A',
             cycle->rotors[0],
             cycle->rotors[1],
             cycle->rotors[2]);
}

static void reset_enigma(const Enigma *restrict enigma, const uint8_t *rotor_positions)
{
    for (uint8_t rotor = 0; rotor < NUM_ROTORS_PER_ENIGMA; ++rotor)
    {
        enigma->rotors[rotor]->position = rotor_positions[rotor];
    }
}

static void create_cycle(const CycleConfiguration *cycle_configuration, CycleOfRotorSetting *restrict cycle)
{

    enum ROTOR_TYPE rotors[NUM_ROTORS_PER_ENIGMA] = {
            cycle_configuration->rotor_permutation[0],
            cycle_configuration->rotor_permutation[1],
            cycle_configuration->rotor_permutation[2]
    };

    uint8_t rotor_positions[NUM_ROTORS_PER_ENIGMA] = {
            cycle_configuration->rotor_one_position,
            cycle_configuration->rotor_two_position,
            cycle_configuration->rotor_three_position
    };

    // Plugboard is implicitly the normal one
    char message[MESSAGE_SIZE] = "AAAAAA";
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

static void swap(uint8_t *a, uint8_t *b)
{
    uint8_t temp = *a;
    *a = *b;
    *b = temp;
}

static void generate_permutations(uint8_t data[], uint8_t left, uint8_t right, FILE *file)
{
    if(left == right)
    {
        CycleOfRotorSetting cycle = {0};

        for (uint8_t rotor_one_position = 0; rotor_one_position < ALPHABET_SIZE; ++rotor_one_position)
        {
            for (uint8_t rotor_two_position = 0; rotor_two_position < ALPHABET_SIZE; ++rotor_two_position)
            {
                for (uint8_t rotor_three_position = 0;
                     rotor_three_position < ALPHABET_SIZE; ++rotor_three_position)
                {

                    char enigma_settings_buffer[BUFFER_SIZE];
                    char cycle_len_buffer[BUFFER_SIZE];
                    CycleConfiguration cycle_configuration = {
                            .rotor_one_position = rotor_one_position,
                            .rotor_two_position = rotor_two_position,
                            .rotor_three_position = rotor_three_position,
                            .rotor_permutation = data,
                            .reflector = UKW_B,
                    };

                    create_cycle(&cycle_configuration, &cycle);
                    get_cycle_whole_lens_string(&cycle, cycle_len_buffer);
                    get_enigma_settings_string(&cycle, enigma_settings_buffer);
                    print_whole_cycle(&cycle, file);
                }
            }
        }

        return;
    }

    for (int i = left; i <= right; ++i)
    {
        swap( data + left, data + i);
        generate_permutations(data, left + 1, right, file);
        swap(data + left, data + i);
    }
}

static void generate_combinations(const uint8_t rotors[], uint8_t data[], uint8_t start, uint8_t index)
{
    if(index == NUM_ROTORS_PER_ENIGMA)
    {
        char path_buffer[512];
        snprintf(path_buffer, sizeof path_buffer, "%s/cycle_%d_%d_%d.txt", FILE_PATH_CYCLO, data[0], data[1], data[2]);

        FILE *file = fopen(path_buffer, "w");
        if (!file) {
            fprintf(stderr, "Can't open file %s\n", path_buffer);
            return;
        }

        generate_permutations(data, 0, NUM_ROTORS_PER_ENIGMA - 1, file);
        fclose(file);
        return;
    }

    for (int i = start; i < NUM_ROTORS; ++i)
    {
        data[index] = rotors[i];
        generate_combinations(rotors, data, i + 1, index + 1);
    }
}

void create_cycles(void)
{
    // 3 rotors and 26 possible settings for each rotor, 3 * 2 * 1 rotor permutations, 1 reflectors

    const uint8_t rotors[NUM_ROTORS] = {1, 2, 3, 4, 5};
    uint8_t data[NUM_ROTORS_PER_ENIGMA];

    generate_combinations(rotors, data, 0, 0);

    puts("Cycles have been written to: " FILE_PATH_CYCLO);
    printf("Total cycles: %d\n", TOTAL_CYCLES);
}
