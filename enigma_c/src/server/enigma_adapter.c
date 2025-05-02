//
// Created by escha on 10.02.25.
//

#include "enigma_adapter.h"
#include "string.h"
#include "cyclometer/server_cyclometer.h"


int enigma_encrypt(EnigmaConfiguration *config, char *output)
{

    Enigma *enigma = create_enigma_from_configuration(config);
    uint8_t *text_as_int = traverse_enigma(enigma);
    if(text_as_int == NULL) return -1;
    char *text = get_string_from_int_array(text_as_int, strlen(config->message));
    strcpy(output, text);

    free(text_as_int);
    free(text);

    return 0;
}
