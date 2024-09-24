/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#pragma once

//
// Created by Emanuel on 23.07.2024.
//

#include <gtk/gtk.h>

enum ENIGMA_TYPE get_enigma_type_from_gui(void);
enum ROTOR_TYPE* get_rotors_from_gui(void);
enum REFLECTOR_TYPE get_reflector_type_from_gui(void);
uint8_t* get_rotor_positions_from_gui(void);
uint8_t* get_rotor_ring_positions_from_gui(void);
char* get_plugboard_from_gui(void);
char* get_input_text_from_gui(void);

void run_gui();
