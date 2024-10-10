#pragma once

#include "enigma/reflector/reflector.h"
#include "enigma/rotor/rotor.h"
#include "enigma/enigma.h"

//
// Created by Emanuel on 05.10.2024.
//

#define ENIGMA                 "--enigma"
#define ENIGMA_SHORT           "-e"
#define ENIGMA_CHAR            'e'


typedef struct
{
    enum ENIGMA_TYPE enigma_type;
    enum ROTOR_TYPE rotor_one_type;
    enum ROTOR_TYPE rotor_two_type;
    enum ROTOR_TYPE rotor_three_type;
    enum ROTOR_TYPE rotor_four_type;
    enum REFLECTOR_TYPE reflector_type;
    char *rotor_offsets;
    char *rotor_positions;
    char *plugboard;
    char *plaintext;
} EnigmaCliOptions;

void query_enigma_input(int argc, char *argv[]);
void print_enigma_help(void);
void run_interactive_enigma_input(void);
