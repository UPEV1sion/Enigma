/*
 * Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
 * Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
 *
 * Die cJSON-Bibliothek, die unter der MIT-Lizenz lizenziert ist, wird ebenfalls verwendet.
 * Die vollständige Lizenz finden Sie hier: https://github.com/DaveGamble/cJSON/blob/master/LICENSE.
 */

/**
 * @name Enigma Simulator
 * @authors Arif Hasanic, Emanuel Schäffer
 * @version 1.1
 */

/*
 * Files that Emanuel created include a header marking them.
 * Files that Arif created include no header.
 * Arif's files are highly likely to also be modified/expanded by Emanuel.
 * Modifications and expansions are not documented, because that would clutter everything.
 * For reference here's Arif's GitHub repository: https://github.com/murderbaer/enigma
 */

#include "cli/cli.h"
#include "gui/bomb_out_gui/bomb_out.h"
#include "helper/helper.h"
#include "turing_bomb/turing_bomb.h"
#include "turing_bomb/cycle_finder/graph_builder.h"
#include "turing_bomb/cycle_finder/cycle_finder_graph.h"
#include "turing_bomb/cycle_finder/cycle_finder.h"

int main(int argc, char *argv[])
{
    // uint8_t *arr_crib = get_int_array_from_string("WETTERVORHERSAGE");
    // uint8_t *arr_cipher = get_int_array_from_string("SNMKGGSTZZUGARLV");
//      start_turing_bomb("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV", 0);
//      GraphOfLongestCycle *graph = build_best_scrambler_graph("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV");
//    start_turing_bomb("KOMMANDODERWEHR", "SSKKEZQRHOTJTDW", 0);
    // start_turing_bomb("BEACHHEAD", "EDBGEAHDB", 0);
//      find_longest_cycle_graph("KOMMANDODERWEHR", "SSKKEZQRHOTJTDW");
//     find_longest_cycle_graph("BEACHHEAD", "EDBGEAHDB");
//    find_longest_cycle_graph("ATTACKATDAWN", "WSNPNLKLSTCS");
     find_longest_cycle_graph("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV");
    // CyclesCribCipher *cycles = find_cycles("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV");
    // free(cycles);
    // return 0;

    query_input(argc, argv);

    return EXIT_SUCCESS;
}
