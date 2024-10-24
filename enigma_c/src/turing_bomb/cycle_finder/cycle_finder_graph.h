#pragma once

//
// Created by Emanuel on 29.09.2024.
//

#include <stdint.h>

#include "helper/helper.h"
#include "cycle_finder.h"

CycleCribCipher* find_longest_cycle_graph(const char *restrict crib, const char *restrict ciphertext);
