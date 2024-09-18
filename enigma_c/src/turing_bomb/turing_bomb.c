#include <stdint.h>
#include <string.h>

#include "turing_bomb.h"

#include "enigma/enigma.h"
#include "helper/helper.h"
#include "cycle_finder.h"
#include "diagonal_board.h"

//
// Created by Emanuel on 07.09.2024.
//

#define  NUM_ROTORS             5
#define  NUM_ROTORS_PER_ENIGMA  3
#define  PLUGBOARD              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

static bool is_valid_crip_position(const char *crib, const char *encrypted_text, const int32_t crib_pos)
{
    const size_t crip_len = strlen(crib);
    if (crip_len + crib_pos > strlen(encrypted_text)) return false;
    for (size_t i = 0; i < crip_len; ++i)
    {
        if (*(encrypted_text + crib_pos + i) == *(crib + i)) return false;
    }

    return true;
}

static char* traverse_m3_enigma_at_position(const EnigmaConfiguration *conf, const int32_t crib_pos,
                                            const int32_t crib_len)
{
    uint8_t *text   = get_int_array_from_string(conf->message);
    uint8_t *output = malloc(sizeof(uint8_t) * crib_len);
    assertmsg(output != NULL, "malloc failed");

    Enigma *enigma = create_enigma_from_configuration(conf);
    assertmsg(enigma != NULL, "enigma == NULL");
    Rotor *rotorOne            = enigma->rotors[0];
    const Rotor *rotorTwo      = enigma->rotors[1];
    const Rotor *rotorThree    = enigma->rotors[2];
    const Reflector *reflector = enigma->reflector;
    const Plugboard *plugboard = enigma->plugboard;

    // The crib was placed so that there were no contradictions with the encrypted text
    // and in a place where it was assumed that only the first rotor was turned
    const size_t crip_end = crib_pos + crib_len;
    for (size_t i = crib_pos; i < crip_end; ++i)
    {
        rotorOne->position = (rotorOne->position + 1) % 26;
        uint8_t character  = traverse_rotor(rotorOne, text[i]);
        character          = traverse_rotor(rotorTwo, character);
        character          = traverse_rotor(rotorThree, character);
        character          = reflector->wiring[character];
        character          = traverse_rotor_inverse(rotorThree, character);
        character          = traverse_rotor_inverse(rotorTwo, character);
        character          = traverse_rotor_inverse(rotorOne, character);
        output[i]          = plugboard->plugboard_data[character];
    }
    free(text);
    free_enigma(enigma);
    char *result = get_string_from_int_array(output, crib_len);
    free(output);
    return result;
}

int32_t start_turing_bomb(char *crib, const char *ciphertext, const int32_t crib_pos)
{
    if (!is_valid_crip_position(crib, ciphertext, crib_pos)) return 1;
    const size_t plain_len = strlen(crib);
    if (plain_len >= 26)
    {
        fprintf(stderr, "Try a shorter crib\n");
        return 1;
    }
    if (crib_pos + plain_len > strlen(ciphertext))
    {
        fprintf(stderr, "Plain outside crib\n");
        return 1;
    }

    const Cycles *candidate_cycles = find_cycles(crib, ciphertext + crib_pos);

    // Different rotor types
    // 60 * 26 * 26 * 26 = 1054560 Permutations
    for (uint8_t rotor_one = 1; rotor_one <= NUM_ROTORS; ++rotor_one)
    {
        for (uint8_t rotor_two = 1; rotor_two <= NUM_ROTORS; ++rotor_two)
        {
            if (rotor_one == rotor_two)
            {
                continue;
            }
            for (uint8_t rotor_three = 1; rotor_three <= NUM_ROTORS; ++rotor_three)
            {
                if (rotor_one == rotor_three || rotor_two == rotor_three)
                {
                    continue;
                }
                for (uint8_t rotor_one_pos = 0; rotor_one_pos < ALPHABET_SIZE; ++rotor_one_pos)
                {
                    for (uint8_t rotor_two_pos = 0; rotor_two_pos < ALPHABET_SIZE; ++rotor_two_pos)
                    {
                        for (uint8_t rotor_three_pos = 0; rotor_three_pos < ALPHABET_SIZE; ++rotor_three_pos)
                        {
                            uint8_t rotors[NUM_ROTORS_PER_ENIGMA]          = {rotor_one, rotor_two, rotor_three};
                            uint8_t rotor_positions[NUM_ROTORS_PER_ENIGMA] = {0};
                            uint8_t ring_settings[NUM_ROTORS_PER_ENIGMA]   = {0};
                            EnigmaConfiguration conf                       = {
                                rotors, rotor_positions, ring_settings, M3, 'B', .message = crib
                            };
                            memcpy(conf.plugboard, PLUGBOARD, sizeof(PLUGBOARD));
                            char *output                 = traverse_m3_enigma_at_position(&conf, crib_pos, plain_len);
                            const Cycles *current_cycles = find_cycles(crib, output);
                            free(output);

                            if (passes_welchman_test(candidate_cycles, current_cycles))
                            {
                                printf("Possible match found: ");
                                printf("%d : %d : %d at %d : %d : %d", conf.rotors[0], conf.rotors[1], conf.rotors[2],
                                       conf.rotor_positions[0], conf.rotor_positions[1], conf.rotors[2]);
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }

    return 1;
}
