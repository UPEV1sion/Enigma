//
// Created by escha on 01.03.25.
//

#include <string.h>

#include "cyclometer_adapter.h"

enum
{
    MAX_DAILY_KEYS = 1024,
    MESSAGE_HEADER_LEN = 6
};

static void generate_random_message_header(char *buffer)
{
    for(size_t i = 0; i < DAILY_KEY_SIZE; ++i)
    {
        buffer[i] = (char) ((random() % 26) + 'A');
    }

    memmove(buffer + DAILY_KEY_SIZE, buffer, DAILY_KEY_SIZE); // Duplicate key
}

static void compute_cycles(const EnigmaConfiguration *config, const int daily_key_count, ComputedCycles *computed_cycles)
{
    Enigma *enigma = create_enigma_from_configuration(config);

    RotorPermutations permutations;
    memset(&permutations, -1, sizeof (RotorPermutations));

    for(int key_num = 0; key_num < daily_key_count; ++key_num)
    {
        char current_key[MESSAGE_HEADER_LEN + 1] = {0};
        generate_random_message_header(current_key);
        uint8_t *encrypted_key = enigma_get_int_array_from_message(enigma, current_key);
        add_daily_key_to_permutations(&permutations, encrypted_key);

        reset_enigma(enigma, config);
        free(encrypted_key);
    }

    server_compute_cycles(&permutations, computed_cycles);
    free_enigma(enigma);
}

int cyclometer_create_cycles(EnigmaConfiguration *config, int daily_key_count, ComputedCycles *computed_cycles)
{
    if (daily_key_count > MAX_DAILY_KEYS) daily_key_count = MAX_DAILY_KEYS;

    compute_cycles(config, daily_key_count, computed_cycles);

    return 0;
}

int manual_cyclometer_create_cycles(EnigmaConfiguration *config, int daily_key_count, char** manual_keys, ComputedCycles *computed_cycles) {

    Enigma *enigma = create_enigma_from_configuration(config);

    RotorPermutations permutations;
    memset(&permutations, -1, sizeof (RotorPermutations));

    
    int man_key = 0;

    while (manual_keys[man_key] != NULL || man_key > MAX_DAILY_KEYS)
    {
        uint8_t *encrypted_key = enigma_get_int_array_from_message(enigma, manual_keys[man_key]);
        add_daily_key_to_permutations(&permutations, encrypted_key);

        reset_enigma(enigma, config);
        free(encrypted_key);

        man_key++;
    }
    if (daily_key_count + man_key > MAX_DAILY_KEYS) daily_key_count = MAX_DAILY_KEYS - man_key;


    for(int key_num = 0; key_num < daily_key_count; ++key_num)
    {
        char current_key[MESSAGE_HEADER_LEN + 1] = {0};
        generate_random_message_header(current_key);
        uint8_t *encrypted_key = enigma_get_int_array_from_message(enigma, current_key);
        add_daily_key_to_permutations(&permutations, encrypted_key);

        reset_enigma(enigma, config);
        free(encrypted_key);
    }
    

    server_compute_cycles(&permutations, computed_cycles);
    free_enigma(enigma);

    return 0;
}
