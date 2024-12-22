#pragma once

//
// Created by Emanuel on 25.07.2024.
//

#include "enigma/enigma.h"
#include "cli/cli_enigma.h"

void enigma_config_to_json(const EnigmaConfiguration *enigma_config, const char *output);
void enigma_cli_options_to_json(const EnigmaCliOptions *cli_options, const char *output);
Enigma* get_enigma_from_json(char *json_string);
char* read_json(void);
