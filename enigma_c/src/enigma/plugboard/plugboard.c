#include <stdlib.h>
#include <string.h>

#include "plugboard.h"
#include "helper/helper.h"

Plugboard* create_plugboard(const char *input)
{
    Plugboard *plugboard = malloc(sizeof(Plugboard));
    assertmsg(plugboard != NULL, "plugboard == NULL");

    if (input == NULL || strlen(input) == 0 || strlen(input) % 2 != 0)
    {
        plugboard->plugboard_data = malloc(ALPHABET_SIZE * sizeof(uint8_t));
        assertmsg(plugboard->plugboard_data != NULL, "plugboard->plugboard_data == NULL");
        for (uint8_t i = 0; i < ALPHABET_SIZE; i++)
        {
            plugboard->plugboard_data[i] = i;
        }
        return plugboard;
    }

    const size_t input_length = strlen(input);

    int8_t plugboard_data[ALPHABET_SIZE];
    memset(plugboard_data, -1, sizeof(plugboard_data));

    for (uint16_t i = 0; i < input_length; i += 2)
    {
        if (input[i] < 'A' || input[i] > 'Z' || input[i + 1] < 'A' || input[i + 1] > 'Z')
        {
            free(plugboard);
            return NULL;
        }
        const int32_t input_left  = input[i] - 'A';
        const int32_t input_right = input[i + 1] - 'A';

        plugboard_data[input_left]  = input_right;
        plugboard_data[input_right] = input_left;
    }

    plugboard->plugboard_data = malloc(ALPHABET_SIZE * sizeof(uint8_t));
    assertmsg(plugboard->plugboard_data != NULL, "plugboard->plugboard_data == NULL");

    for (uint16_t i = 0; i < ALPHABET_SIZE; i++)
    {
        plugboard->plugboard_data[i] = (plugboard_data[i] == -1) ? i : plugboard_data[i];
    }

    return plugboard;
}
