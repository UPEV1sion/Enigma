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
 * @param enigma The enigma machine to traverse.
 * @param text_in_integer The text to traverse.
 * @param array_size The size of the text.
 * @return uint8_t*: The traversed text as an uint8_t* array.
 */
uint8_t* traverse_m3_enigma(const Enigma *enigma, const uint8_t *text_in_integer, const size_t array_size)
{
    uint8_t *output = malloc(array_size * sizeof(uint8_t));
    assertmsg(output != NULL, "malloc failed");

    Rotor *rotor_one   = enigma->rotors[0];
    Rotor *rotor_two   = enigma->rotors[1];
    Rotor *rotor_three = enigma->rotors[2];

    const Plugboard *plugboard    = enigma->plugboard;
    const Reflector *reflector    = enigma->reflector;
    const uint8_t *plugboard_data = plugboard->plugboard_data;

    for (size_t i = 0; i < array_size; i++)
    {
        rotor_one->position = (rotor_one->position + 1) % 26;
        if (should_rotate(rotor_one))
        {
            rotor_two->position = (rotor_two->position + 1) % 26;
            if (should_rotate(rotor_two))
            {
                rotor_three->position = (rotor_three->position + 1) % 26;
            }
        }

        uint8_t character = plugboard_data[text_in_integer[i]];
        character         = traverse_rotor(rotor_one, character);
        character         = traverse_rotor(rotor_two, character);
        character         = traverse_rotor(rotor_three, character);
        character         = reflector->wiring[character];
        character         = traverse_rotor_inverse(rotor_three, character);
        character         = traverse_rotor_inverse(rotor_two, character);
        character         = traverse_rotor_inverse(rotor_one, character);
        output[i]         = plugboard_data[character];
    }

    return output;
}

/**
 * @brief This function is used to traverse the m4 enigma machine.
 * @param enigma The enigma machine to traverse.
 * @param text_in_integer The text to traverse.
 * @param array_size The size of the text.
 * @return uint8_t*: The traversed text as an uint8_t array.
 */
uint8_t* traverse_m4_enigma(const Enigma *enigma, const uint8_t *text_in_integer, const size_t array_size)
{
    uint8_t *output = malloc(array_size * sizeof(uint8_t));
    assertmsg(output != NULL, "malloc failed");

    Rotor *rotor_one   = enigma->rotors[0];
    Rotor *rotor_two   = enigma->rotors[1];
    Rotor *rotor_three = enigma->rotors[2];
    Rotor *rotor_four  = enigma->rotors[3];

    const Plugboard *plugboard    = enigma->plugboard;
    const Reflector *reflector    = enigma->reflector;
    const uint8_t *plugboard_data = plugboard->plugboard_data;

    for (size_t i = 0; i < array_size; i++)
    {
        rotor_one->position = (rotor_one->position + 1) % 26;
        if (should_rotate(rotor_one))
        {
            rotor_two->position = (rotor_two->position + 1) % 26;
            if (should_rotate(rotor_two))
            {
                rotor_three->position = (rotor_three->position + 1) % 26;
                if (should_rotate(rotor_three))
                {
                    rotor_four->position = (rotor_four->position + 1) % 26;
                }
            }
        }

        uint8_t character = plugboard_data[text_in_integer[i]];
        character         = traverse_rotor(rotor_one, character);
        character         = traverse_rotor(rotor_two, character);
        character         = traverse_rotor(rotor_three, character);
        character         = traverse_rotor(rotor_four, character);
        character         = reflector->wiring[character];
        character         = traverse_rotor_inverse(rotor_four, character);
        character         = traverse_rotor_inverse(rotor_three, character);
        character         = traverse_rotor_inverse(rotor_two, character);
        character         = traverse_rotor_inverse(rotor_one, character);
        output[i]         = plugboard_data[character];
    }

    return output;
}

/**
 * @brief Traverses the correct Enigma machine based on the Enigma provided.
 * @param enigma The enigma machine to be traversed.
 * @return uint8_t*: The traversed text.
 */
uint8_t* traverse_enigma(const Enigma *enigma)
{
    char *plaintext         = enigma->plaintext;
    const size_t array_size = strlen(plaintext);

    assertmsg(to_uppercase(plaintext) == 0, "to_uppercase failed");

    uint8_t *text_in_integer = get_int_array_from_string(plaintext);
    assertmsg(text_in_integer != NULL, "string to int[] conversion failed");

    uint8_t *output;

    if (enigma->type == ENIGMA_M3)
    {
        output = traverse_m3_enigma(enigma, text_in_integer, array_size);
    }
    else
    {
        output = traverse_m4_enigma(enigma, text_in_integer, array_size);
    }

    free(text_in_integer);

    return output;
}

/**
 * @brief This function is used to create an enigma machine.
 * @param enigma_configuration The configuration of the enigma machine.
 * @return Enigma*: The created enigma machine.
 */
Enigma* create_enigma_from_configuration(const EnigmaConfiguration *enigma_configuration)
{
    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "malloc failed");

    enigma->type = enigma_configuration->type;

    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors != NULL, "malloc failed");

    for (uint16_t i = 0; i < (uint16_t) enigma->type; i++)
    {
        const enum ROTOR_TYPE rotor_type = enigma_configuration->rotors[i];
        const uint8_t position           = enigma_configuration->rotor_positions[i];
        const uint8_t offset             = enigma_configuration->ring_settings[i];

        Rotor *rotor = create_rotor_by_type(rotor_type, position, offset);
        assertmsg(rotor != NULL, "couldn't create rotor");

        enigma->rotors[i] = rotor;
    }

    enigma->reflector = create_reflector_by_type(enigma_configuration->reflector);
    enigma->plugboard = create_plugboard(enigma_configuration->plugboard);

    enigma->plaintext = strdup(enigma_configuration->message);
    assertmsg(enigma->plaintext != NULL, "strdup failed");

    return enigma;
}

void free_enigma(Enigma *enigma)
{
    for (uint8_t i = 0; i < (uint8_t) enigma->type; ++i)
    {
        free(enigma->rotors[i]->notch);
        free(enigma->rotors[i]);
    }
    free(enigma->rotors);
    free(enigma->plugboard);
    free(enigma->reflector);
    free(enigma->plaintext);
    free(enigma);
}
