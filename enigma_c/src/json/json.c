/*
* Diese Datei verwendet die cJSON-Bibliothek, die unter der MIT-Lizenz lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/DaveGamble/cJSON/blob/master/LICENSE.
*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#include <cjson/cJSON.h>

#include "json.h"
#include "gui/enigma_gui.h"
#include "helper/helper.h"

//
// Created by Emanuel on 26.07.2024.
//

static cJSON *root;

static char* get_json_string(void)
{
    char *json_string = cJSON_Print(root);
    return json_string;
}

static void delete_root(void)
{
    cJSON_Delete(root);
    root = NULL;
}

static void add_enigma_model_to_json(void)
{
    cJSON_AddItemToObject(root, "model",
                          cJSON_CreateNumber(get_enigma_type_from_gui()));
}

static void add_reflector_to_json()
{
    // somewhat scuffed but works for now
    char reflector[2] = {0};
    reflector[0] = get_reflector_type_from_gui() + 'B';
    cJSON_AddStringToObject(root, "reflector", reflector);
}

static void add_rotors_to_json(void)
{
    uint8_t *rotors = get_rotors_from_gui();
    cJSON *rotors_array      = cJSON_AddArrayToObject(root, "rotors");
    for (uint16_t i = 0; i < get_enigma_type_from_gui(); ++i)
    {
        cJSON_AddItemToArray(rotors_array, cJSON_CreateNumber(rotors[i]));
    }
    free(rotors);
}

static void add_positions_to_json(void)
{
    uint8_t *positions = get_rotor_positions_from_gui();
    cJSON *positions_array      = cJSON_AddArrayToObject(root, "positions");
    for (uint16_t i = 0; i < get_enigma_type_from_gui(); ++i)
    {
        cJSON_AddItemToArray(positions_array, cJSON_CreateNumber(positions[i]));
    }
    free(positions);
}

static void add_rings_to_json(void)
{
    uint8_t *rings = get_rotor_ring_positions_from_gui();
    cJSON *rings_array      = cJSON_AddArrayToObject(root, "rings");
    for (uint16_t i = 0; i < get_enigma_type_from_gui(); ++i)
    {
        cJSON_AddItemToArray(rings_array, cJSON_CreateNumber(rings[i]));
    }
    free(rings);
}

static void add_plugboard_to_json(void)
{
    char *text = get_plugboard_from_gui();
    cJSON_AddStringToObject(root, "plugboard", text);
    free(text);
}

static void add_input_to_json(void)
{
    char *input_text = get_input_text_from_gui();
    cJSON_AddStringToObject(root, "input", input_text);
    free(input_text);
}

static void add_output_to_json(const char *text)
{
    cJSON_AddStringToObject(root, "output", text);
}

static void write_json_to_file(void)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_JSON, "w")) != NULL, "file == NULL");

    char *json_string = get_json_string();
    fputs(json_string, file);
    fclose(file);
    cJSON_free(json_string);
}

void enigma_to_json(const char *out)
{
    root                   = cJSON_CreateObject();
    GtkTreeModel *modelT   = NULL;
    const GtkTreeIter iter = {0, NULL, NULL, NULL};

    add_enigma_model_to_json();
    add_reflector_to_json(modelT, iter);
    add_rotors_to_json();
    add_positions_to_json();
    add_rings_to_json();
    add_plugboard_to_json();
    add_input_to_json();
    if (strlen(out) > 0) add_output_to_json(out);
    write_json_to_file();
    delete_root();
}

char* read_json(void)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_JSON, "r")) != NULL, "file == NULL");

    fseek(file, 0, SEEK_END);
    const long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    //malloc
    char *buffer = malloc(file_size + 1);
    assertmsg(buffer != NULL, "buffer == NULL");
    fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = 0;

    fclose(file);

    return buffer;
}

static void save_model_to_conf(EnigmaConfiguration *configuration)
{
    const cJSON *model_item = cJSON_GetObjectItem(root, "model");
    if (cJSON_IsNumber(model_item))
    {
        configuration->type = model_item->valueint;
    }
}

static void save_reflector_to_conf(EnigmaConfiguration *configuration)
{
    const cJSON *refl_item = cJSON_GetObjectItem(root, "reflector");
    if (cJSON_IsString(refl_item))
    {
        configuration->reflector = refl_item->valuestring[0];
    }
}

static void save_rotor_to_conf(EnigmaConfiguration *configuration)
{
    configuration->rotors          = malloc(configuration->type * sizeof(uint8_t));
    configuration->rotor_positions = malloc(configuration->type * sizeof(uint8_t));
    configuration->ring_settings   = malloc(configuration->type * sizeof(uint8_t));
    assertmsg(configuration->rotors != NULL, "configuration->rotors == NULL");
    assertmsg(configuration->rotor_positions != NULL, "configuration->rotor_positions == NULL");
    assertmsg(configuration->ring_settings != NULL, "configuration->ring_setting == NULL");

    const cJSON *rotor_item = cJSON_GetObjectItem(root, "rotors");
    const cJSON *pos_item   = cJSON_GetObjectItem(root, "positions");
    const cJSON *ring_item  = cJSON_GetObjectItem(root, "rings");
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

static void save_plugboard_to_conf(EnigmaConfiguration *configuration)
{
    const cJSON *plugboard_item = cJSON_GetObjectItem(root, "plugboard");
    if (cJSON_IsString(plugboard_item))
    {
        strcpy(configuration->plugboard, plugboard_item->valuestring);
    }
}

static void save_input_to_conf(EnigmaConfiguration *configuration)
{
    const cJSON *input_item = cJSON_GetObjectItem(root, "input");
    if (cJSON_IsString(input_item))
    {
        configuration->message = malloc(strlen(input_item->valuestring) + 1);
        strcpy(configuration->message, input_item->valuestring);
    }
}

Enigma* get_enigma_from_json(void)
{
    EnigmaConfiguration configuration;

    char *json_string = read_json();
    root              = cJSON_Parse(json_string);
    assertmsg(root != NULL, "parsing failed");

    puts(json_string);
    free(json_string);

    save_model_to_conf(&configuration);
    save_reflector_to_conf(&configuration);
    save_rotor_to_conf(&configuration);
    save_plugboard_to_conf(&configuration);
    save_input_to_conf(&configuration);
    Enigma *temp = create_enigma_from_configuration(&configuration);

    free(configuration.ring_settings);
    free(configuration.rotor_positions);
    free(configuration.rotors);
    free(configuration.message);
    delete_root();

    return temp;
}
