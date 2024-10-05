#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rotor.h"
#include "helper/helper.h"

#define ROTOR_I_WIRING          "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
#define ROTOR_I_INV_WIRING      "UWYGADFPVZBECKMTHXSLRINQOJ"
#define ROTOR_I_NOTCH           'Q'
#define ROTOR_II_WIRING         "AJDKSIRUXBLHWTMCQGZNPYFVOE"
#define ROTOR_II_INV_WIRING     "AJPCZWRLFBDKOTYUQGENHXMIVS"
#define ROTOR_II_NOTCH          'E'
#define ROTOR_III_WIRING        "BDFHJLCPRTXVZNYEIWGAKMUSQO"
#define ROTOR_III_INV_WIRING    "TAGBPCSDQEUFVNZHYIXJWLRKOM"
#define ROTOR_III_NOTCH         'V'
#define ROTOR_IV_WIRING         "ESOVPZJAYQUIRHXLNFTGKDCMWB"
#define ROTOR_IV_INV_WIRING     "HZWVARTNLGUPXQCEJMBSKDYOIF"
#define ROTOR_IV_NOTCH          'J'
#define ROTOR_V_WIRING          "VZBRGITYUPSDNHLXAWMJQOFECK"
#define ROTOR_V_INV_WIRING      "QCYLXWENFTZOSMVJUDKGIARPHB"
#define ROTOR_V_NOTCH           'Z'

#define ROTOR_VI_WIRING         "JPGVOUMFYQBENHZRDKASXLICTW"
#define ROTOR_VI_INV_WIRING     "SKXQLHCNWARVGMEBJPTYFDZUIO"
#define ROTOR_VI_NOTCH_ONE      'Z'
#define ROTOR_VI_NOTCH_TWO      'M'
#define ROTOR_VII_WIRING        "NZJHGRCXMYSWBOUFAIVLPEKQDT"
#define ROTOR_VII_INV_WIRING    "QMGYVPEDRCWTIANUXFKZOSLHJB"
#define ROTOR_VII_NOTCH_ONE     'Z'
#define ROTOR_VII_NOTCH_TWO     'M'
#define ROTOR_VIII_WIRING       "FKQHTLXOCBJSPDZRAMEWNIUYGV"
#define ROTOR_VIII_INV_WIRING   "QJINSAYDVKBFRUHMCPLEWZTGXO"
#define ROTOR_VIII_NOTCH_ONE    'Z'
#define ROTOR_VIII_NOTCH_TWO    'M'

#define ROTOR_BETA_WIRING       "LEYJVCNIXWPBQMDRTAKZGFUHOS"
#define ROTOR_BETA_INV_WIRING   "RLFOBVUXHDSANGYKMPZQWEJICT"
#define ROTOR_GAMMA_WIRING      "FSOKANUERHMBTIYCWLQPZXVGJD"
#define ROTOR_GAMMA_INV_WIRING  "ELPZHAXJNYDRKFCTSIBMGWQVOU"


/**
 * @brief Makes sure a value is >= 0 and < 26
 * @param value The value to be processed
 * @return int8_t
 */
static inline uint8_t mod26(const int8_t value)
{
    return  (value + 26) % 26;
}

/**
 * @brief Determines if the turning point for the rotor to the left has been reached.
 * @param rotor The rotor to the right from the rotor, which is in question to be rotated.
 * @return bool
 */
bool should_rotate(const Rotor *rotor)
{
    const uint8_t position = rotor->position - 1;
    if (rotor->notch_count == 1)
    {
        return position == rotor->notch[0];
    }
    if (rotor->notch_count == 2)
    {
        return position == rotor->notch[0] || position == rotor->notch[1];
    }
    return false;
}

/**
 * @brief Creates a one-notched rotor. (ROTOR_1 - ROTOR_5)
 * @param wiring The wiring of the rotor.
 * @param inverse_wiring The inverse wiring of the rotor.
 * @param notch The initial notch position of the Rotor without offset.
 * @param position The rotor position.
 * @param offset The rotor offset / ring setting.
 * @return Rotor*
 */
Rotor* create_one_notch_rotor(const char *wiring, const char *inverse_wiring, const char notch,
                              const uint8_t position, const uint8_t offset)
{
    Rotor *rotor = malloc(sizeof(Rotor));
    assertmsg(rotor != NULL, "malloc failed");

    const size_t wiring_length = strlen(wiring);
    for (size_t i = 0; i < wiring_length; i++)
    {
        rotor->wiring[i]         = wiring[i] - 'A';
        rotor->inverse_wiring[i] = inverse_wiring[i] - 'A';
    }

    rotor->position    = mod26(position - offset);
    rotor->notch_count = 1;

    rotor->notch       = malloc(sizeof(uint8_t));
    assertmsg(rotor->notch != NULL, "malloc failed");

    rotor->notch[0] = mod26(notch - 'A' - offset);

    return rotor;
}

/**
 * @brief Creates a two-notched rotor. (ROTOR_6 - ROTOR_8)
 * @param wiring The wiring of the rotor.
 * @param inverse_wiring The inverse wiring of the rotor.
 * @param notch1 The initial notch one position of the Rotor without offset.
 * @param notch2 The initial notch two position of the Rotor without offset.
 * @param position The rotor position.
 * @param offset The rotor offset / ring setting.
 * @return Rotor*
 */
Rotor* create_two_notch_rotor(const char *wiring, const char *inverse_wiring, const char notch1,
                              const char notch2, const uint8_t position, const uint8_t offset)
{
    Rotor *rotor = malloc(sizeof(Rotor));
    assertmsg(rotor != NULL, "malloc failed");

    const size_t wiring_length = strlen(wiring);
    for (size_t i = 0; i < wiring_length; i++)
    {
        rotor->wiring[i]         = wiring[i] - 'A';
        rotor->inverse_wiring[i] = inverse_wiring[i] - 'A';
    }

    rotor->position    = mod26(position - offset);
    rotor->notch_count = 2;

    rotor->notch       = malloc(2 * sizeof(uint8_t));
    assertmsg(rotor->notch != NULL, "malloc failed");

    rotor->notch[0] = mod26(notch1 - 'A' - offset);
    rotor->notch[1] = mod26(notch2 - 'A' - offset);

    return rotor;
}

/**
 * @brief Factory method for creating Rotors
 * @param type The Rotor type.
 * @param position The rotor position.
 * @param offset The rotor offset.
 * @return Rotor*
 */
Rotor* create_rotor_by_type(const enum ROTOR_TYPE type, const uint8_t position, const uint8_t offset)
{
    switch (type)
    {
        case ROTOR_1:
            return create_one_notch_rotor(ROTOR_I_WIRING, ROTOR_I_INV_WIRING, ROTOR_I_NOTCH, position, offset);
        case ROTOR_2:
            return create_one_notch_rotor(ROTOR_II_WIRING, ROTOR_II_INV_WIRING, ROTOR_II_NOTCH, position, offset);
        case ROTOR_3:
            return create_one_notch_rotor(ROTOR_III_WIRING, ROTOR_III_INV_WIRING, ROTOR_III_NOTCH, position, offset);
        case ROTOR_4:
            return create_one_notch_rotor(ROTOR_IV_WIRING, ROTOR_IV_INV_WIRING, ROTOR_IV_NOTCH, position, offset);
        case ROTOR_5:
            return create_one_notch_rotor(ROTOR_V_WIRING, ROTOR_V_INV_WIRING, ROTOR_V_NOTCH, position, offset);
        case ROTOR_6:
            return create_two_notch_rotor(ROTOR_VI_WIRING, ROTOR_VI_INV_WIRING,
                                          ROTOR_VI_NOTCH_ONE, ROTOR_VI_NOTCH_TWO, position, offset);
        case ROTOR_7:
            return create_two_notch_rotor(ROTOR_VII_WIRING, ROTOR_VII_INV_WIRING,
                                          ROTOR_VII_NOTCH_ONE, ROTOR_VII_NOTCH_TWO, position, offset);
        case ROTOR_8:
            return create_two_notch_rotor(ROTOR_VIII_WIRING, ROTOR_VIII_INV_WIRING,
                                          ROTOR_VIII_NOTCH_ONE, ROTOR_VIII_NOTCH_TWO, position, offset);
        case ROTOR_BETA:
            return create_one_notch_rotor(ROTOR_BETA_WIRING, ROTOR_BETA_INV_WIRING, '\0', position, offset);
        case ROTOR_GAMMA:
            return create_one_notch_rotor(ROTOR_GAMMA_WIRING, ROTOR_GAMMA_INV_WIRING, '\0', position, offset);
        default:
            fprintf(stderr, "Error, Rotor definition not found: %d\n", type);
            exit(1);
    }
}

/**
 * @brief Traverses a Rotor from right to left.
 * @param rotor The Rotor.
 * @param character The char to be encrypted/decrypted. (0-25)
 * @return uint8_t The encoded char. (0-25)
 */
uint8_t traverse_rotor(const Rotor *rotor, const uint8_t character)
{
    const int8_t index_from_right = mod26(character + rotor->position);
    const int8_t index_from_left  = mod26(rotor->wiring[index_from_right] - rotor->position);

    return index_from_left;
}

/**
 * @brief Traverses a Rotor from left to right.
 * @param rotor The Rotor.
 * @param character The char to be encrypted/decrypted. (0-25)
 * @return uint8_t The encoded char. (0-25)
 */
uint8_t traverse_rotor_inverse(const Rotor *rotor, const uint8_t character)
{
    const int8_t index_from_left  = mod26(character + rotor->position);
    const int8_t index_from_right = mod26(rotor->inverse_wiring[index_from_left] - rotor->position);

    return index_from_right;
}
