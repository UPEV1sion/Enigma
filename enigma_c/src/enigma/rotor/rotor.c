#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rotor.h"
#include "helper/helper.h"

//TODO probably should append _WIRING to the wirings
#define ROTOR_I             "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
#define ROTOR_I_INV         "UWYGADFPVZBECKMTHXSLRINQOJ"
#define ROTOR_I_NOTCH       'Q'
#define ROTOR_II            "AJDKSIRUXBLHWTMCQGZNPYFVOE"
#define ROTOR_II_INV        "AJPCZWRLFBDKOTYUQGENHXMIVS"
#define ROTOR_II_NOTCH      'E'
#define ROTOR_III           "BDFHJLCPRTXVZNYEIWGAKMUSQO"
#define ROTOR_III_INV       "TAGBPCSDQEUFVNZHYIXJWLRKOM"
#define ROTOR_III_NOTCH     'V'
#define ROTOR_IV            "ESOVPZJAYQUIRHXLNFTGKDCMWB"
#define ROTOR_IV_INV        "HZWVARTNLGUPXQCEJMBSKDYOIF"
#define ROTOR_IV_NOTCH      'J'
#define ROTOR_V             "VZBRGITYUPSDNHLXAWMJQOFECK"
#define ROTOR_V_INV         "QCYLXWENFTZOSMVJUDKGIARPHB"
#define ROTOR_V_NOTCH       'Z'

#define ROTOR_VI                "JPGVOUMFYQBENHZRDKASXLICTW"
#define ROTOR_VI_INV            "SKXQLHCNWARVGMEBJPTYFDZUIO"
#define ROTOR_VI_NOTCH_ONE      'Z'
#define ROTOR_VI_NOTCH_TWO      'M'
#define ROTOR_VII               "NZJHGRCXMYSWBOUFAIVLPEKQDT"
#define ROTOR_VII_INV           "QMGYVPEDRCWTIANUXFKZOSLHJB"
#define ROTOR_VII_NOTCH_ONE     'Z'
#define ROTOR_VII_NOTCH_TWO     'M'
#define ROTOR_VIII              "FKQHTLXOCBJSPDZRAMEWNIUYGV"
#define ROTOR_VIII_INV          "QJINSAYDVKBFRUHMCPLEWZTGXO"
#define ROTOR_VIII_NOTCH_ONE    'Z'
#define ROTOR_VIII_NOTCH_TWO    'M'


//DO NOT MAKE THIS UINT8_T! This was the reason for one of the longest and most painful debugging sessions!
static inline int8_t mod26(const int8_t value)
{
    return (int8_t) ((value + 26) % 26);
}

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
    return 0;
}

Rotor* create_one_notch_rotor(const char *wiring, const char *inverse_wiring, const char notch,
                              const uint8_t position, const uint8_t offset)
{
    Rotor *rotor = malloc(sizeof(Rotor));
    assertmsg(rotor != NULL, "rotor == NULL");

    const size_t wiring_length = strlen(wiring);
    for (size_t i = 0; i < wiring_length; i++)
    {
        rotor->wiring[i]         = wiring[i] - 'A';
        rotor->inverse_wiring[i] = inverse_wiring[i] - 'A';
    }

    rotor->position    = mod26(position - offset);
    rotor->notch_count = 1;
    rotor->notch       = malloc(sizeof(uint8_t));
    assertmsg(rotor->notch != NULL, "rotor->notch == NULL");
    rotor->notch[0] = mod26(notch - 'A' - offset);

    return rotor;
}

Rotor* create_two_notch_rotor(const char *wiring, const char *inverse_wiring, const char notch1,
                              const char notch2, const uint8_t position, const uint8_t offset)
{
    Rotor *rotor = malloc(sizeof(Rotor));
    assertmsg(rotor != NULL, "rotor == NULL");

    const size_t wiring_length = strlen(wiring);
    for (size_t i = 0; i < wiring_length; i++)
    {
        rotor->wiring[i]         = wiring[i] - 'A';
        rotor->inverse_wiring[i] = inverse_wiring[i] - 'A';
    }

    rotor->position    = mod26(position - offset);
    rotor->notch_count = 2;
    rotor->notch       = malloc(2 * sizeof(uint8_t));
    assertmsg(rotor->notch != NULL, "rotor->notch == NULL");
    rotor->notch[0] = mod26(notch1 - 'A' - offset);
    rotor->notch[1] = mod26(notch2 - 'A' - offset);


    return rotor;
}

Rotor* create_rotor_by_type(const enum ROTOR_TYPE type, const uint8_t position, const uint8_t offset)
{
    switch (type)
    {
        case ROTOR_1:
            return create_one_notch_rotor(ROTOR_I, ROTOR_I_INV, ROTOR_I_NOTCH,
                                          position, offset);
        case ROTOR_2:
            return create_one_notch_rotor(ROTOR_II, ROTOR_II_INV, ROTOR_II_NOTCH,
                                          position, offset);
        case ROTOR_3:
            return create_one_notch_rotor(ROTOR_III, ROTOR_III_INV, ROTOR_III_NOTCH,
                                          position, offset);
        case ROTOR_4:
            return create_one_notch_rotor(ROTOR_IV, ROTOR_IV_INV, ROTOR_IV_NOTCH,
                                          position, offset);
        case ROTOR_5:
            return create_one_notch_rotor(ROTOR_V, ROTOR_V_INV, ROTOR_V_NOTCH,
                                          position, offset);
        case ROTOR_6:
            return create_two_notch_rotor(ROTOR_VI, ROTOR_VI_INV,
                                          ROTOR_VI_NOTCH_ONE, ROTOR_VI_NOTCH_TWO, position, offset);
        case ROTOR_7:
            return create_two_notch_rotor(ROTOR_VII, ROTOR_VII_INV,
                                          ROTOR_VII_NOTCH_ONE, ROTOR_VII_NOTCH_TWO, position, offset);
        case ROTOR_8:
            return create_two_notch_rotor(ROTOR_VIII, ROTOR_VIII_INV,
                                          ROTOR_VIII_NOTCH_ONE,
                                          ROTOR_VIII_NOTCH_TWO, position, offset);
        default:
            fprintf(stderr, "Error, Rotor definition not found: %d", type);
            exit(1);
    }
}

uint8_t traverse_rotor(const Rotor *rotor, const uint8_t character)
{
    const int8_t index_from_right = mod26(character + rotor->position);
    const int8_t index_from_left  = mod26(rotor->wiring[index_from_right] - rotor->position);

    return index_from_left;
}

uint8_t traverse_rotor_inverse(const Rotor *rotor, const uint8_t character)
{
    const int8_t index_from_left  = mod26(character + rotor->position);
    const int8_t index_from_right = mod26(rotor->inverse_wiring[index_from_left] - rotor->position);

    return index_from_right;
}
