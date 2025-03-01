//
// Created by escha on 10.02.25.
//

#include "enigma_adapter.h"
#include "string.h"
#include "cyclometer/server_cyclometer.h"

int config_adapter_to_enigma(EnigmaConfigAdapter *adapter, EnigmaConfiguration *config)
{
    strncpy(config->plugboard, adapter->plugboard, ALPHABET_SIZE);
    config->message = strdup(adapter->message);
    if(config->message == NULL) return -1;

    for (int i = 0; i < (int)adapter->type; i++)
    {
        config->rotor_positions[i] = (uint8_t)adapter->rotor_positions[i];
        config->ring_settings[i] = (uint8_t)adapter->ring_settings[i];
        config->rotors[i] = adapter->rotors[i];
    }
    config->type = adapter->type;
    config->reflector = adapter->reflector;

    return 0;
}

int enigma_encrypt(EnigmaConfigAdapter *adapter, char *output)
{
    EnigmaConfiguration config;
    uint8_t rotor_positions[4];
    uint8_t ring_settings[4];
    enum ROTOR_TYPE rotors[4];
    config.rotor_positions = rotor_positions;
    config.ring_settings = ring_settings;
    config.rotors = rotors;

    if(config_adapter_to_enigma(adapter, &config) < 0) return -1;

    Enigma *enigma = create_enigma_from_configuration(&config);
    uint8_t *text_as_int = traverse_enigma(enigma);
    char *text = get_string_from_int_array(text_as_int, strlen(adapter->message));
    strcpy(output, text);

    free(config.message);
    free_enigma(enigma);
    free(text_as_int);
    free(text);

    return 0;
}
