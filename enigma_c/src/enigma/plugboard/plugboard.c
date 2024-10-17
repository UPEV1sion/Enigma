#include <stdlib.h>
#include <string.h>

#include "plugboard.h"

/**
 * @brief Creates a plugboard with letters plugged specified in input.
 * Standard plugboard if input is empty or an invalid input.
 * @note Only accepts uppercase letters
 * @param input The letters that should be plugged
 * @return Plugboard*
 */
Plugboard* create_plugboard(const char *input)
{
    Plugboard *plugboard = malloc(sizeof(Plugboard));
    assertmsg(plugboard != NULL, "malloc failed");

    for (uint8_t i = 0; i < ALPHABET_SIZE; i++)
    {
        plugboard->plugboard_data[i] = i;
    }

    size_t input_length;

    if (input == NULL || *input == 0 || (input_length = strlen(input)) % 2 != 0)
    {
        return plugboard;
    }

    for (uint8_t i = 0; i < (uint8_t) input_length; i += 2)
    {
        const uint8_t input_left  = input[i] - 'A';
        const uint8_t input_right = input[i + 1] - 'A';

        plugboard->plugboard_data[input_left]  = input_right;
        plugboard->plugboard_data[input_right] = input_left;
    }

    return plugboard;
}
