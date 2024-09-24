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

/* A very useful resource was a YouTube video by Gustav Vogels named
 * TURING WELCHMAN BOMBE - Beschreibung des kompletten Entschlüsselungsverfahrens der ENIGMA.
 * In which explained the inner workings of the bombe in great depth and detail.
 * https://youtu.be/7pOsBhwwmhI
 */

/* This aims to be an "authentic" turing implementation.
 * The turing bomb back in the day, of course, used no software.
 * But this implementation aims to mimic the inner workings of the Turing-Welchman Bomb as close as possible.
 * I've always programmed with performance in mind and made this as fast as possible.
 * The "Hill-climbing" will possibly be a faster alternative, but this is not the goal of this implementation.
 */

/* One more note from me: if you don't like bit manipulations, I apologize, but
 * the algorithms that I developed like which are using bit manipulations like "is_permutation"
 * and "is_cycle" are faster by several magnitudes than there corresponding "normal" approaches.
 * I've had the goal to provide a fast user experience, and that does sometimes interfere with readable code.
 */

#define NUM_ROTORS             5
#define PLUGBOARD              "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

static bool is_valid_crip_position(const char *crib, const char *encrypted_text, const uint32_t crib_pos)
{
    const size_t crip_len = strlen(crib);
    if (crip_len + crib_pos > strlen(encrypted_text)) return false;
    for (size_t i = 0; i < crip_len; ++i)
    {
        if (*(encrypted_text + crib_pos + i) == *(crib + i)) return false;
    }

    return true;
}

static uint8_t* traverse_m3_enigma_at_position(const EnigmaConfiguration *conf, const uint32_t crib_pos,
                                               const size_t crib_len)
{
    uint8_t *text = get_int_array_from_string(conf->message);
    assertmsg(text != NULL, "string to int[] conversion failed");
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
    return output;
}

int32_t start_turing_bomb(const char *crib, const char *ciphertext, const uint32_t crib_pos)
{
    if (!is_valid_crip_position(crib, ciphertext, crib_pos)) return 1;
    const size_t crib_len = strlen(crib);
    if (crib_len > NUM_SCRAMBLERS_PER_ROW)
    {
        fprintf(stderr, "Try a shorter crib\n");
        return 1;
    }
    if (crib_pos + crib_len > strlen(ciphertext))
    {
        fprintf(stderr, "Plain outside crib\n");
        return 1;
    }
    uint8_t *crib_as_ints       = get_int_array_from_string(crib);
    uint8_t *ciphertext_as_ints = get_int_array_from_string(ciphertext);

    TuringBomb turing_bomb;
    create_bomb_menu(&turing_bomb, crib_as_ints, ciphertext_as_ints, crib_len);
    // const Cycles *candidate_cycles = find_cycles(crib_as_ints, ciphertext_as_ints + crib_pos, crib_len);

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
                enum ROTOR_TYPE rotors[NUM_ROTORS_PER_ENIGMA]  = {rotor_one, rotor_two, rotor_three};
                for (uint8_t rotor_one_pos = 0; rotor_one_pos < ALPHABET_SIZE; ++rotor_one_pos)
                {
                    for (uint8_t rotor_two_pos = 0; rotor_two_pos < ALPHABET_SIZE; ++rotor_two_pos)
                    {
                        for (uint8_t rotor_three_pos = 0; rotor_three_pos < ALPHABET_SIZE; ++rotor_three_pos)
                        {
                            uint8_t rotor_positions[NUM_ROTORS_PER_ENIGMA] = {
                                rotor_one_pos, rotor_two_pos, rotor_three_pos
                            };


                        }
                    }
                }
            }
        }
    }
    free(crib_as_ints);
    free(ciphertext_as_ints);

    return 1;
}


/*
 EnigmaConfiguration conf                     = {
    //     .rotors = rotors, .rotor_positions = rotor_positions, .ring_settings = ring_settings,
    //     .type = ENIGMA_M3, .reflector = UKW_B, .message = crib
    // };
    // memcpy(conf.plugboard, PLUGBOARD, sizeof(PLUGBOARD));
    // uint8_t *output = traverse_m3_enigma_at_position(&conf, crib_pos, crib_len);

    const Cycles *current_cycles = find_cycles(crib_as_ints, output, crib_len);


    if (passes_welchman_test(candidate_cycles, current_cycles))
    {
        printf("Possible match found: ");
        printf("%d : %d : %d at %d : %d : %d", conf.rotors[0], conf.rotors[1], conf.rotors[2],
               conf.rotor_positions[0], conf.rotor_positions[1], conf.rotors[2]);
        return 0;
    }
    free(output);*/