#pragma once

//
// Created by Emanuel on 25.07.2024.
//

#include "enigma/enigma.h"
#include "cli/cli_enigma.h"
#include "server/server.h"
#include "cJSON.h"

void enigma_config_to_json(const EnigmaConfiguration *enigma_config, const char *output);
void enigma_cli_options_to_json(const EnigmaCliOptions *cli_options, const char *output);
Enigma* get_enigma_from_json_string(const char *json_string);
Enigma* get_enigma_from_json(const cJSON *json);
int32_t get_server_cyclometer_options_from_json(ServerCyclometerOptions *options, const char *json_string);
char* read_json(void);
