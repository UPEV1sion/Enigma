//
// Created by escha on 10.02.25.
//

#include "enigma_adapter.h"
#include "string.h"
#include "cyclometer/server_cyclometer.h"
#include "server.h"
#include "db_cyclometer.h"

#define MAX_DAILY_KEYS 1024

int config_adapter_to_enigma(EnigmaConfigAdapter *adapter, EnigmaConfiguration *config)
{

    strncpy(config->plugboard, adapter->plugboard, ALPHABET_SIZE);
    config->message = strdup(adapter->message);

    for (int i = 0; i < (int)adapter->type; i++)
    {
        config->rotor_positions[i] = (uint8_t)adapter->rotor_positions[i];
        config->ring_settings[i] = (uint8_t)adapter->ring_settings[i];
        config->rotors[i] = adapter->rotors[i];
    }
    config->type = adapter->type;
    config->reflector = adapter->reflector;
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

    config_adapter_to_enigma(adapter, &config);
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

static char** generate_n_daily_keys(const int32_t n)
{
    char **keys = malloc(sizeof(char *) * n);
    assertmsg(keys != NULL, "malloc failed");

    for (int32_t i = 0; i < n; ++i)
    {
        keys[i] = malloc(DAILY_KEY_SIZE + 1);
        assertmsg(keys[i] != NULL, "malloc failed");
        for (int j = 0; j < DAILY_KEY_SIZE; ++j)
        {
            keys[i][j] = (char) ((random() % 26) + 'A');
        }
        keys[i][DAILY_KEY_SIZE] = 0;
    }

    return keys;
}

static void free_keys(char **keys, const int32_t n)
{
    for (int32_t i = 0; i < n; ++i)
    {
        free(keys[i]);
    }
}

static void free_enigma_config(EnigmaConfiguration *conf)
{
    free(conf->ring_settings);
    free(conf->rotor_positions);
    free(conf->rotors);
    free(conf);
}



int cyclometer_create_cycles (EnigmaConfigAdapter *adapter, int daily_key_count) {
    printf("Cylometer Adapter \n");
    fflush(stdout);

    EnigmaConfiguration config;
    uint8_t rotor_positions[4];
    uint8_t ring_settings[4];
    enum ROTOR_TYPE rotors[4];
    config.rotor_positions = rotor_positions;
    config.ring_settings = ring_settings;
    config.rotors = rotors;
    config_adapter_to_enigma(adapter, &config);


    if (daily_key_count > MAX_DAILY_KEYS) daily_key_count = MAX_DAILY_KEYS;
    char **keys = generate_n_daily_keys(daily_key_count);
    // free(opt.enigma_conf->message);
    char *enc_keys[MAX_DAILY_KEYS];


    for (int32_t i = 0; i < daily_key_count; ++i)
    {
        char current_key[DAILY_KEY_SIZE * 2 + 1] = {0};
        memcpy(current_key, keys[i], DAILY_KEY_SIZE);
        memcpy(current_key + DAILY_KEY_SIZE, keys[i], DAILY_KEY_SIZE);
        config.message = current_key;
        Enigma *enigma           = create_enigma_from_configuration(&config);
        uint8_t *enc_key_as_ints = traverse_enigma(enigma);
        char *enc_key            = get_string_from_int_array(enc_key_as_ints, DAILY_KEY_SIZE * 2);
        printf("%s -> %s\n", current_key, enc_key);
        enc_keys[i] = enc_key;

        free(enc_key_as_ints);
        free_enigma(enigma);
        
    }


    S_Cycle *cycles        = server_create_cycles(enc_keys, daily_key_count);

    //query_db(cycles);


    free(cycles);
    free_keys(enc_keys, daily_key_count);
    free_enigma_config(&config);
    free_keys(keys, daily_key_count);
    free(keys);


    //free(config.message);
    return 0;
}
