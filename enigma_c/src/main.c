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
#include "helper/helper.h"
#include "turing_bombe/turing_bombe.h"

int main(int argc, char *argv[])
{
//      start_turing_bombe("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV", 0);
//    start_turing_bombe("KOMMANDODERWEHR", "SSKKEZQRHOTJTDW", 0);
//    start_turing_bombe("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV", 0);
//     start_turing_bombe("BEACHHEAD", "EDBGEAHDB", 0);
//      find_longest_menu("KOMMANDODERWEHR", "SSKKEZQRHOTJTDW");
//     find_longest_menu("BEACHHEAD", "EDBGEAHDB");
//    find_longest_menu("ATTACKATDAWN", "WSNPNLKLSTCS");
     // find_longest_menu("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV");
//    query_input(argc, argv);
    uint16_t rotor1 = 0;
    uint16_t rotor2 = 0;
    uint16_t rotor3 = 0;

    uint16_t counter = 0;
    while(1)
    {
        if(rotor1 % 26 == 0)
        {
            if(rotor2 % 26 == 0)
            {
                rotor3++;
                if(rotor3 % 27 == 0)
                    break;
            }
            rotor2++;

        }
        rotor1++;
        counter++;
    }

    printf("%d\n", counter);

    return EXIT_SUCCESS;
}
