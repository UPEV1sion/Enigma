#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "helper/helper.h"
#include "enigma.h"
#include "plugboard/plugboard.h"
#include "reflector/reflector.h"
#include "rotor/rotor.h"

/**
 * @brief This function is used to traverse the m3 enigma machine.
 *
 * @param enigma The enigma machine to traverse.
 * @param text_in_integer The text to traverse.
 * @param array_size The size of the text.
 * @return uint8_t*: The traversed text as an uint8_t* array.
 */
uint8_t* traverse_m3_enigma(const Enigma *enigma, uint8_t *text_in_integer, const size_t array_size)
{
    uint8_t *output = malloc(array_size * sizeof(uint8_t));
    assertmsg(output != NULL, "output == NULL");
    uint8_t *text = malloc(array_size * sizeof(uint8_t));
    assertmsg(text != NULL, "text == NULL");

    memcpy(text, text_in_integer, array_size * sizeof(uint8_t));

    Rotor *rotor_one   = enigma->rotors[0];
    Rotor *rotor_two   = enigma->rotors[1];
    Rotor *rotor_three = enigma->rotors[2];

    const Plugboard *plugboard = enigma->plugboard;
    assertmsg(plugboard != NULL, "plugboard == NULL");
    const Reflector *reflector = enigma->reflector;

    uint8_t *text_ptr                 = text;
    const uint8_t *plugboard_data_ptr = plugboard->plugboard_data;
    for (size_t i = 0; i < array_size; i++)
    {
        *(text_ptr + i) = *(plugboard_data_ptr + *(text_ptr + i));
    }

    for (size_t i = 0; i < array_size; i++)
    {
        rotor_one->position = (rotor_one->position + 1) % 26;

        if (should_rotate(rotor_one))
        {
            puts("Rot 2");
            rotor_two->position = (rotor_two->position + 1) % 26;

            if (should_rotate(rotor_two))
            {
                puts("Rot 3");
                rotor_three->position = (rotor_three->position + 1) % 26;
            }
        }
        uint8_t character = traverse_rotor(rotor_one, text[i]);
        character         = traverse_rotor(rotor_two, character);
        character         = traverse_rotor(rotor_three, character);
        character         = reflector->wiring[character];
        character         = traverse_rotor_inverse(rotor_three, character);
        character         = traverse_rotor_inverse(rotor_two, character);
        character         = traverse_rotor_inverse(rotor_one, character);
        output[i]         = plugboard->plugboard_data[character];
    }

    free(text_in_integer);
    free(text);

    return output;
}

/**
 * @brief This function is used to traverse the m4 enigma machine.
 *
 * @param enigma The enigma machine to traverse.
 * @param text_in_integer The text to traverse.
 * @param array_size The size of the text.
 * @return uint8_t*: The traversed text as an uint8_t array.
 */
uint8_t* traverse_m4_enigma(const Enigma *enigma, uint8_t *text_in_integer, const size_t array_size)
{
    uint8_t *output = malloc(array_size * sizeof(uint8_t));
    assertmsg(output != NULL, "output == NULL");
    uint8_t *text = malloc(array_size * sizeof(uint8_t));
    assertmsg(text != NULL, "text == NULL");

    memcpy(text, text_in_integer, array_size * sizeof(uint8_t));

    Rotor *rotor_one   = enigma->rotors[0];
    Rotor *rotor_two   = enigma->rotors[1];
    Rotor *rotor_three = enigma->rotors[2];
    Rotor *rotor_four = enigma->rotors[3];

    const Plugboard *plugboard = enigma->plugboard;
    assertmsg(plugboard != NULL, "plugboard == NULL");
    const Reflector *reflector = enigma->reflector;

    uint8_t *text_ptr                 = text;
    const uint8_t *plugboard_data_ptr = plugboard->plugboard_data;
    for (size_t i = 0; i < array_size; i++)
    {
        *(text_ptr + i) = *(plugboard_data_ptr + *(text_ptr + i));
    }

    for (size_t i = 0; i < array_size; i++)
    {
        rotor_one->position = (rotor_one->position + 1) % 26;

        if (should_rotate(rotor_one))
        {
            puts("Rot 2");
            rotor_two->position = (rotor_two->position + 1) % 26;

            if (should_rotate(rotor_two))
            {
                puts("Rot 3");
                rotor_three->position = (rotor_three->position + 1) % 26;
                if(should_rotate(rotor_three))
                {
                    puts("Rot 4");
                    rotor_four->position = (rotor_four->position + 1) % 26;
                }
            }
        }
        uint8_t character = traverse_rotor(rotor_one, text[i]);
        character         = traverse_rotor(rotor_two, character);
        character         = traverse_rotor(rotor_three, character);
        character         = traverse_rotor(rotor_four, character);
        character         = reflector->wiring[character];
        character         = traverse_rotor_inverse(rotor_four, character);
        character         = traverse_rotor_inverse(rotor_three, character);
        character         = traverse_rotor_inverse(rotor_two, character);
        character         = traverse_rotor_inverse(rotor_one, character);
        output[i]         = plugboard->plugboard_data[character];
    }

    free(text_in_integer);
    free(text);

    return output;
}

/**
 * @brief This function is used to traverse the enigma machine.
 *
 * @param enigma The enigma machine to traverse.
 * @return uint8_t*: The traversed text.
 */
uint8_t* traverse_enigma(const Enigma *enigma)
{
    char *plaintext          = enigma->plaintext;
    const size_t array_size = strlen(plaintext);
    to_upper_case(plaintext);
    uint8_t *text_in_numbers = get_int_array_from_string(plaintext);

    if(enigma->type == M3)
        return traverse_m3_enigma(enigma, text_in_numbers, array_size);
    return traverse_m4_enigma(enigma, text_in_numbers, array_size);
}

/**
 * @brief This function is used to create an enigma machine.
 *
 * @param enigma_configuration The configuration of the enigma machine.
 * @return Enigma*: The created enigma machine.
 */
Enigma* create_enigma_from_configuration(const EnigmaConfiguration *enigma_configuration)
{
    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "enigma == NULL");

    enigma->type = enigma_configuration->type;

    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors != NULL, "enigma->rotors == NULL");

    for (uint16_t i = 0; i < (uint16_t) enigma->type; i++)
    {
        const int32_t rotor_type = enigma_configuration->rotors[i];
        const int32_t position   = enigma_configuration->rotor_positions[i];
        const int32_t offset     = enigma_configuration->ring_settings[i];

        Rotor *rotor = create_rotor(rotor_type, position, offset);
        if (rotor == NULL)
        {
            for (uint16_t j = 0; j < i; j++)
            {
                free(enigma->rotors[j]);
            }
            free(enigma->rotors);
            free(enigma);
            return NULL;
        }
        enigma->rotors[i] = rotor;
    }

    enigma->reflector = create_reflector_by_type(enigma_configuration->reflector);

    if (enigma_configuration->plugboard[0] != '\0')
    {
        enigma->plugboard = create_plugboard(enigma_configuration->plugboard);
    }
    else
    {
        enigma->plugboard = create_plugboard(NULL);
    }
    const size_t len_message = strlen(enigma_configuration->message);
    enigma->plaintext        = malloc(len_message + 1);
    if (enigma->plaintext == NULL)
    {
        free(enigma->plugboard);
        free(enigma->reflector);
        for (uint8_t i = 0; i < (uint8_t) enigma->type; i++)
        {
            free(enigma->rotors[i]);
        }
        free(enigma->rotors);
        free(enigma);
        return NULL;
    }
    memcpy(enigma->plaintext, enigma_configuration->message, len_message);
    enigma->plaintext[len_message] = 0;

    return enigma;
}

void free_enigma(Enigma *enigma)
{
    free(enigma->rotors[0]->notch);
    free(enigma->rotors[0]);
    free(enigma->rotors[1]->notch);
    free(enigma->rotors[1]);
    free(enigma->rotors[2]->notch);
    free(enigma->rotors[2]);
    free(enigma->rotors);
    free(enigma->plugboard->plugboard_data);
    free(enigma->plugboard);
    free(enigma->reflector->wiring);
    free(enigma->reflector);
    free(enigma->plaintext);
    free(enigma);
}
