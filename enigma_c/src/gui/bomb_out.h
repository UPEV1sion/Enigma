/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
* Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
*/

#pragma once

//
// Created by Emanuel on 07.08.2024.
//

#include <gtk/gtk.h>

void add_output_row(const gchar *rotor, const gchar *position, const gchar *ring, const gchar *plugboard);
void run_gui_bomb_out(void);
