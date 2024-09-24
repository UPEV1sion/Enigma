#pragma once

//
// Created by Emanuel on 30.08.2024.
//

#include <stdbool.h>

#include "cycle_finder.h"

bool passes_welchman_test(const Cycles *candidate_cycles, const Cycles *current_cycles);
