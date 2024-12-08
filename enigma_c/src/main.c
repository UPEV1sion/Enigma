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

#include <string.h>

#include "cli/cli.h"
#include "enigma/enigma.h"
#include "helper/helper.h"
#include "turing_bombe/turing_bombe.h"

static void foo(const uint8_t *rotor)
{
    uint8_t cycle_len = 0;
    bool visited[ALPHABET_SIZE] = {0};

    for (uint8_t base = 0; base < ALPHABET_SIZE; ++base)
    {
        if (!visited[base])
        {
            uint8_t current = rotor[base];
            while (current != base)
            {
                visited[current] = true;
                current = rotor[current];
                printf("%c", current + 'A');
                cycle_len++;
            }
            printf("\ncycle length: %u\n", cycle_len);
            cycle_len = 0;
        }
    }
}


int main(int argc, char *argv[])
{

//      start_turing_bombe("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV", 0);
//    start_turing_bombe("KOMMANDODERWEHR", "SSKKEZQRHOTJTDW", 0);
    // start_turing_bombe("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV", 0);
//     start_turing_bombe("BEACHHEAD", "EDBGEAHDB", 0);
//      find_longest_menu("KOMMANDODERWEHR", "SSKKEZQRHOTJTDW");
//     find_longest_menu("BEACHHEAD", "EDBGEAHDB");
//    find_longest_menu("ATTACKATDAWN", "WSNPNLKLSTCS");
     // find_longest_menu("WETTERVORHERSAGE", "SNMKGGSTZZUGARLV");
    query_input(argc, argv);

    uint8_t rotor_1[ALPHABET_SIZE] = {0};
    uint8_t rotor_2[ALPHABET_SIZE] = {0};
    uint8_t rotor_3[ALPHABET_SIZE] = {0};


    for (int i = 0; i < 26; ++i)
    {
        char message[] = "AAAAAA";
        memset(message, i + 'A', 6);
        enum ROTOR_TYPE rotors[] = {ROTOR_3, ROTOR_1, ROTOR_2};
        uint8_t ring_settings[] = {0, 0, 0};
        uint8_t positions[] = {5, 24, 6};

        EnigmaConfiguration conf = {
            .message = message,
            .rotors = rotors,
            .type = ENIGMA_M3,
            .reflector = UKW_A,
            .ring_settings = ring_settings,
            .rotor_positions = positions,
        };

        Enigma *enigma     = create_enigma_from_configuration(&conf);
        uint8_t *output    = traverse_enigma(enigma);
        rotor_1[output[0]] = output[3];
        rotor_2[output[1]] = output[4];
        rotor_3[output[2]] = output[5];

        free_enigma(enigma);
    }

    puts("ROTOR 1");
    foo(rotor_1);
    puts("ROTOR 2");
    foo(rotor_2);
    puts("ROTOR 3");
    foo(rotor_3);

    return EXIT_SUCCESS;
}
