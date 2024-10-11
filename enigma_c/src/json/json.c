/*
* Diese Datei verwendet die cJSON-Bibliothek, die unter der MIT-Lizenz lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/DaveGamble/cJSON/blob/master/LICENSE.
*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#include <gtk/gtk.h>

#include "cJSON.h"

#include "json.h"
#include "cli/cli_enigma.h"

//
// Created by Emanuel on 26.07.2024.
//

static char* get_json_string(const cJSON *json)
{
    return cJSON_Print(json);
}

static void delete_json(cJSON *json)
{
    cJSON_Delete(json);
//    root = NULL;
}

static void add_enigma_model_to_json(cJSON *json, const enum ENIGMA_TYPE enigma_type)
{
    cJSON_AddItemToObject(json, "model",
                          cJSON_CreateNumber(enigma_type));
}

static void add_reflector_to_json(cJSON *json, const enum REFLECTOR_TYPE reflector_type)
{
    // I really don't like this, but works for now
    const char reflector[2] = {reflector_type, 0};
    cJSON_AddStringToObject(json, "reflector", reflector);
}

static void add_rotors_to_json(cJSON *json, const enum ROTOR_TYPE *rotors, const uint8_t num_rotors)
{
    cJSON *rotors_array     = cJSON_AddArrayToObject(json, "rotors");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        cJSON_AddItemToArray(rotors_array, cJSON_CreateNumber(rotors[i]));
    }
}

static void add_positions_to_json(cJSON *json, uint8_t *positions, const uint8_t num_rotors)
{
    cJSON *positions_array = cJSON_AddArrayToObject(json, "positions");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        cJSON_AddItemToArray(positions_array, cJSON_CreateNumber(positions[i]));
    }
}

static void add_rings_to_json(cJSON *json, const uint8_t *rings, const uint8_t num_rotors)
{
    cJSON *rings_array = cJSON_AddArrayToObject(json, "rings");
    for (uint8_t i = 0; i < num_rotors; ++i)
    {
        cJSON_AddItemToArray(rings_array, cJSON_CreateNumber(rings[i]));
    }
}

static void add_plugboard_to_json(cJSON *json, const char *plugboard)
{
    cJSON_AddStringToObject(json, "plugboard", plugboard);
}

static void add_input_to_json(cJSON *json, const char *input_text)
{
    cJSON_AddStringToObject(json, "input", input_text);
}

static void add_output_to_json(cJSON *json, const char *text)
{
    cJSON_AddStringToObject(json, "output", text);
}

static void write_json_to_file(cJSON *json)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_JSON, "w")) != NULL, "file == NULL");

    char *json_string = get_json_string(json);
    fputs(json_string, file);
    fclose(file);
    cJSON_free(json_string);
}

void enigma_config_to_json(const EnigmaConfiguration *const enigma_config, const char *output)
{
    cJSON *json = cJSON_CreateObject();

     add_enigma_model_to_json(json, enigma_config->type);
     add_reflector_to_json(json, enigma_config->reflector);
     add_rotors_to_json(json, enigma_config->rotors, enigma_config->type);
     add_positions_to_json(json, enigma_config->rotor_positions, enigma_config->type);
     add_rings_to_json(json, enigma_config->ring_settings, enigma_config->type);
     add_plugboard_to_json(json, enigma_config->plugboard);
     add_input_to_json(json, enigma_config->message);
     add_output_to_json(json, output);

    write_json_to_file(json);
    delete_json(json);
}

void enigma_cli_options_to_json(const EnigmaCliOptions *const cli_options, const char *output)
{
    cJSON *json = cJSON_CreateObject();

    add_enigma_model_to_json(json, cli_options->enigma_type);
    add_reflector_to_json(json, cli_options->reflector_type);
    enum ROTOR_TYPE rotor_types[4] = {cli_options->rotor_one_type,
                                      cli_options->rotor_two_type,
                                      cli_options->rotor_three_type,
                                      cli_options->rotor_four_type};
    add_rotors_to_json(json, rotor_types, cli_options->enigma_type);
    uint8_t rotor_positions[4];
    for (uint8_t i = 0; i < (uint8_t) cli_options->enigma_type; ++i)
    {
        rotor_positions[i] = cli_options->rotor_positions[i] - 'A';
    }
    add_positions_to_json(json, rotor_positions, cli_options->enigma_type);
    uint8_t ring_settings[4];
    for (uint8_t i = 0; i < (uint8_t) cli_options->enigma_type; ++i)
    {
        ring_settings[i] = cli_options->rotor_offsets[i] - 'A';
    }
    add_rings_to_json(json, ring_settings, cli_options->enigma_type);
    add_plugboard_to_json(json, cli_options->plugboard);
    add_input_to_json(json, cli_options->plaintext);
    add_output_to_json(json, output);

    write_json_to_file(json);
    delete_json(json);
}

char* read_json(void)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_JSON, "rb")) != NULL, "file == NULL");

    fseek(file, 0, SEEK_END);
    const size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *buffer = malloc(file_size + 1);
    assertmsg(buffer != NULL, "buffer == NULL");
    (void) fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = 0;

    fclose(file);

    return buffer;
}

static void save_model_to_conf(cJSON *json, EnigmaConfiguration *configuration)
{
    const cJSON *model_item = cJSON_GetObjectItem(json, "model");
    if (cJSON_IsNumber(model_item))
    {
        configuration->type = model_item->valueint;
    }
}

static void save_reflector_to_conf(cJSON *json, EnigmaConfiguration *configuration)
{
    const cJSON *reflector_item = cJSON_GetObjectItem(json, "reflector");
    if (cJSON_IsString(reflector_item))
    {
        if(strcmp(reflector_item->valuestring, "A") == 0)
            configuration->reflector = UKW_A;
        else if(strcmp(reflector_item->valuestring, "B") == 0)
            configuration->reflector = UKW_B;
        else if(strcmp(reflector_item->valuestring, "C") == 0)
            configuration->reflector = UKW_C;
    }
}

static void save_rotor_to_conf(cJSON *json,EnigmaConfiguration *configuration)
{
    configuration->rotors          = malloc(configuration->type * sizeof(uint8_t));
    configuration->rotor_positions = malloc(configuration->type * sizeof(uint8_t));
    configuration->ring_settings   = malloc(configuration->type * sizeof(uint8_t));
    assertmsg(configuration->rotors != NULL, "configuration->rotors == NULL");
    assertmsg(configuration->rotor_positions != NULL, "configuration->rotor_positions == NULL");
    assertmsg(configuration->ring_settings != NULL, "configuration->ring_setting == NULL");

    const cJSON *rotor_item = cJSON_GetObjectItem(json, "rotors");
    const cJSON *pos_item   = cJSON_GetObjectItem(json, "positions");
    const cJSON *ring_item  = cJSON_GetObjectItem(json, "rings");
    if (cJSON_IsArray(rotor_item) && cJSON_IsArray(pos_item) &&
        cJSON_IsArray(ring_item))
    {
        for (uint16_t i = 0; i < (uint16_t) configuration->type; ++i)
        {
            const cJSON *rotor    = cJSON_GetArrayItem(rotor_item, i);
            const cJSON *position = cJSON_GetArrayItem(pos_item, i);
            const cJSON *ring     = cJSON_GetArrayItem(ring_item, i);

            if (cJSON_IsNumber(rotor) && cJSON_IsNumber(position) &&
                cJSON_IsNumber(ring))
            {
                configuration->rotors[i]          = rotor->valueint;
                configuration->rotor_positions[i] = position->valueint;
                configuration->ring_settings[i]   = ring->valueint;
            }
        }
    }
}

static void save_plugboard_to_conf(cJSON *json, EnigmaConfiguration *configuration)
{
    const cJSON *plugboard_item = cJSON_GetObjectItem(json, "plugboard");
    if (cJSON_IsString(plugboard_item))
    {
        strcpy(configuration->plugboard, plugboard_item->valuestring);
    }
}

static void save_input_to_conf(cJSON *json, EnigmaConfiguration *configuration)
{
    const cJSON *input_item = cJSON_GetObjectItem(json, "input");
    if (cJSON_IsString(input_item))
    {
        configuration->message = strdup(input_item->valuestring);
        assertmsg(configuration->message != NULL, "strdup failed");
    }
}

Enigma* get_enigma_from_json(void)
{
    EnigmaConfiguration configuration;

    char *json_string = read_json();
    cJSON *json              = cJSON_Parse(json_string);
    assertmsg(json != NULL, "parsing failed");

    puts(json_string);
    free(json_string);

    save_model_to_conf(json, &configuration);
    save_reflector_to_conf(json, &configuration);
    save_rotor_to_conf(json, &configuration);
    save_plugboard_to_conf(json, &configuration);
    save_input_to_conf(json, &configuration);
    Enigma *temp = create_enigma_from_configuration(&configuration);

    free(configuration.ring_settings);
    free(configuration.rotor_positions);
    free(configuration.rotors);
    free(configuration.message);
    delete_json(json);

    return temp;
}
