/*
* Dieses Projekt verwendet GTK-3.0, das unter der GNU General Public License (GPL) lizenziert ist.
 * Die vollständige Lizenz finden Sie hier: https://github.com/GNOME/gtk/blob/main/COPYING.
 *
 * Die cJSON-Bibliothek, die unter der MIT-Lizenz lizenziert ist, wird ebenfalls verwendet.
 * Die vollständige Lizenz finden Sie hier: https://github.com/DaveGamble/cJSON/blob/master/LICENSE.
 */

#include "cli/cli.h"
#include "enigma/rotor/rotor.h"
#include "gui/bomb_out.h"
#include "gui/start_gui.h"
#include "helper/helper.h"
#include "turing_bomb/brute_force.h"
#include "turing_bomb/cycle_finder.h"
#include "turing_bomb/turing_bomb.h"

int main(int argc, char *argv[])
{
    // Rotor *rotor = create_one_notch_rotor("EKMFLGDQVZNTOWYHXUSPAIBRCJ", "UWYGADFPVZBECKMTHXSLRINQOJ", 'Q', 0, 0);
    // printf("%d", traverse_rotor(rotor, 0));
    // printf("%d", traverse_rotor(rotor, 0));
    // crack_enigma("KOMMANDODERWEHR", "CVGTLLBDXKNCZQX");
    // find_cycles("KOMMANDODERWEHR", "SSKKEZQRHOTJTDW");
    // find_cycles("KOMMANDODERWEHR", "NLGAGPQLQUYASLS");
    // puts("");
    // find_cycles("KOMMANDODERWEHR", "NLGBNPTLKOYBXLS");
    // printf("%s\n", is_valid_crip_position("OBERKOMMANDODERWEHRMACHT", "BHNCXSEQKOBIIODWFBTZGCYEHQQJEWOYNBDXHQBALHTSSDPWGW", 4) ? "true" : "false");
    // return 0;
    if (argc < 2)
    {
        puts("Usage: ./main [OPTIONS]");
        puts("Options:");
        puts(GUI " / " GUI_SHORT " for GUI");
        puts("CLI use is implicit");
        puts(HELP " / " HELP_SHORT " for help");
        return EXIT_FAILURE;
    }

    if (strcmp(GUI, argv[1]) == 0 || strcmp(GUI_SHORT, argv[1]) == 0)
        run_start_gui(argv);
    else if (strcmp(HELP, argv[1]) == 0 || strcmp(HELP_SHORT, argv[1]) == 0)
        query_help();
    else
        query_input(argc, argv);
    return EXIT_SUCCESS;
}
