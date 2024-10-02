#pragma once

//
// Created by Emanuel on 29.09.2024.
//

#include <stdint.h>

#include "helper/helper.h"
#include "cycle_finder.h"


// typedef struct
// {
//     Node *cycle_start;
//     uint8_t pos_cycle_wo_stubs[ALPHABET_SIZE];
//     uint8_t pos_cycle_w_stubs[ALPHABET_SIZE]; // with / without
//     uint8_t len_wo_stubs;
//     uint8_t len_w_stubs;
// } CycleGraph;

CycleCribCipher* find_best_cycle_graph(const char *restrict crib, const char *restrict ciphertext);
