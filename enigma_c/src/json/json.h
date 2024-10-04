#pragma once

//
// Created by Emanuel on 25.07.2024.
//

#include "enigma/enigma.h"

void enigma_to_json(const Enigma *restrict enigma);
Enigma* get_enigma_from_json(void);
char* read_json(void);
