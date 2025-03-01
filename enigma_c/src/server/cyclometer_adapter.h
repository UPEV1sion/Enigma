//
// Created by escha on 01.03.25.
//

#pragma once

#include "enigma_adapter.h"
#include "cyclometer/server_cyclometer.h"

int cyclometer_create_cycles(EnigmaConfigAdapter *adapter, int daily_key_count, ComputedCycles *computed_cycles);

