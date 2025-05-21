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

int main(int argc, char *argv[])
{
    query_input(argc, argv);

    return EXIT_SUCCESS;
}
