#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cyclometer/cyclometer.h"
#include "cli.h"

#include "cli_enigma.h"
#include "helper/helper.h"
#include "enigma/enigma.h"
#include "turing_bomb/turing_bomb.h"

// TODO make the CLI for the Bomb interactive & offsets standard AAA

#define INPUT_BUFFER_SIZE  1024

/*----------ENIGMA INTERACTIVE CLI----------*/
#define INTERACTIVE_ENIGMA          "--interactive"
#define INTERACTIVE_ENIGMA_SHORT    "-i"

/*----------CYCLOMETER----------*/
#define CYCLOMETER                  "--cyclometer"
#define CYCLOMETER_SHORT            "-c"

/*----------BOMB----------*/
#define BOMB                        "--bomb"
#define BOMB_SHORT                  "-b"
#define CIPHERTEXT                  "--ciphertext"
#define CIPHERTEXT_SHORT            "-ct"
#define CRIB                        "--crib"
#define CRIB_SHORT                  "-cr"
#define CRIB_OFFSET                 "--crib-offset"
#define CRIB_OFFSET_SHORT           "-co"

typedef struct
{
    char *ciphertext;
    char *crib;
    int32_t crib_offset;
    uint8_t enigma: 1;
    uint8_t bomb: 1;
    uint8_t interactive_enigma: 1;
    uint8_t cyclometer: 1;
    uint8_t help: 1;
} CliOptions;

static int32_t string_equals(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

static void print_bomb_help(void)
{
    puts("\n\n");
    puts("Usage: enigma [OPTIONS]...");
    puts("Decrypts message text using the a turing bomb implementation\n");
    puts("Bomb options:");
    printf("%6s, %-15s | %-40s\n", "-short", "--long", "description");
    for (uint16_t i = 0; i < 41; ++i) printf("%c", '=');
    printf("\n%6s, %-15s | %-40s\n", BOMB_SHORT, BOMB_SHORT, "bomb mode");
    printf("%6s, %-15s | %-40s\n", CIPHERTEXT_SHORT, CIPHERTEXT, "ciphertext");
    printf("%6s, %-15s | %-40s\n", CRIB_SHORT, CRIB, "crib");
    printf("%6s, %-15s | %-40s\n\n", CRIB_OFFSET_SHORT, CRIB_OFFSET, "crib");
    puts("Examples:");
    puts(
        "\tenigma " BOMB_SHORT " " CIPHERTEXT_SHORT
        " OKQDRXACYEHWQDVHBAOXFPNMCQAILNBOGVODGJSZJSRPOWYSKKDBVJSHHMQBSKMBBLRLQUJFAFRDBFWFMCHUSXPBFJNKAINU "
        CRIB_SHORT " WETTERBERICHT" CRIB_OFFSET_SHORT " 0");
}

static void print_help(void)
{
    puts("\n\n");
    puts("Usage: enigma [OPTIONS]...");
    printf("%6s, %-15s | %-40s\n", "-short", "--long", "description");
    for (uint16_t i = 0; i < 41; ++i) printf("%c", '=');
    printf("\n%6s, %-15s | %-40s\n", HELP_SHORT, HELP, "display this help and exit");
    printf("%6s, %-15s | %-40s\n", ENIGMA_SHORT, ENIGMA, "enigma mode");
    printf("%6s, %-15s | %-40s\n", INTERACTIVE_ENIGMA_SHORT, INTERACTIVE_ENIGMA, "configure the enigma interactively");
    printf("%6s, %-15s | %-80s\n", CYCLOMETER_SHORT, CYCLOMETER, "generate all possible cycles of an enigma "
           "M3 with Rotors 1 - 3");
    printf("%6s, %-15s | %-40s\n", BOMB_SHORT, BOMB, "bomb mode");
}

void query_help(void)
{
    puts("Usage: enigma [OPTIONS]...");
    puts("-a to show all options");
    puts("-e for additional enigma help");
    puts("-b for additional bomb help");
    char buffer[INPUT_BUFFER_SIZE];
    while (my_getline(buffer, INPUT_BUFFER_SIZE) == 0)
        ;

    //Don't use strtok elsewhere util parsing is finished!
    //strtok stores the string internally, and the string will get overwritten when used elsewhere
    const char *options = strtok(buffer, " ");
    while (options != NULL)
    {
        if (string_equals(options, "-a"))
        {
            print_help();
            print_bomb_help();
            print_enigma_help();
        }
        else if (string_equals(ENIGMA_SHORT, options))
        {
            print_enigma_help();
        }
        else if (string_equals(BOMB_SHORT, options))
        {
            print_bomb_help();
        }
        options = strtok(NULL, " ");
    }
}

static void save_bomb_input(CliOptions *options, const int32_t argc, char *argv[])
{
    int32_t i = 1;
    while (i < argc - 1)
    {
        if (string_equals(CIPHERTEXT, argv[i]) ||
            string_equals(CIPHERTEXT_SHORT, argv[i]))
        {
            options->ciphertext = argv[++i];
        }
        else if (string_equals(CRIB, argv[i]) ||
                 string_equals(CRIB_SHORT, argv[i]))
        {
            options->crib = argv[++i];
        }
        else if (string_equals(CRIB_OFFSET, argv[i]) ||
                 string_equals(CRIB_OFFSET_SHORT, argv[i]))
        {
            assertmsg(get_number_from_string(argv[++i], &options->crib_offset) == 0, "Bad crib offset");
        }
        else
        {
            printf("Invalid option: %s\n", argv[i]);
            exit(1);
        }
        i++;
    }
}

static void save_input(CliOptions *options, const int32_t argc, char *argv[])
{
    if (string_equals(HELP, argv[1]) ||
        string_equals(HELP_SHORT, argv[1]))
    {
        options->help = 1;
        return;
    }
    if (string_equals(INTERACTIVE_ENIGMA, argv[1]) ||
        string_equals(INTERACTIVE_ENIGMA_SHORT, argv[1]))
    {
        options->interactive_enigma = 1;
        return;
    }
    if (string_equals(ENIGMA, argv[1]) ||
        string_equals(ENIGMA_SHORT, argv[1]))
    {
        options->enigma = 1;
        return;
    }
    if (string_equals(CYCLOMETER, argv[1]) ||
        string_equals(CYCLOMETER_SHORT, argv[1]))
    {
        options->cyclometer = 1;
        return;
    }
    if (string_equals(BOMB, argv[1]) ||
        string_equals(BOMB_SHORT, argv[1]))
    {
        options->bomb = 1;
        save_bomb_input(options, argc, ++argv);
    }
    else
    {
        printf("Invalid option: %s\n", argv[1]);
        exit(1);
    }
}

static void normalize_cli_options(const CliOptions *options)
{
    int err_code = 0;
    if (options->bomb)
    {
        err_code |= to_uppercase(options->ciphertext);
        err_code |= remove_non_alnum(options->ciphertext);
        err_code |= to_uppercase(options->crib);
        err_code |= remove_non_alnum(options->crib);
        err_code |= options->crib_offset < 0;
        assertmsg(err_code == 0, "normalization failed");
    }
}

// TODO: refactor into more functions
void query_input(const int32_t argc, char *argv[])
{
    if (argc == 1)
    {
        puts("Usage: enigma [OPTIONS]...");
        puts("Use enigma -h for addition help");
        exit(0);
    }

    CliOptions options = {0};

    save_input(&options, argc, argv);

    if (options.help)
    {
        query_help();
        return;
    }
    if (options.enigma)
    {
        query_enigma_input(argc, argv);
        return;
    }
    if(options.interactive_enigma)
    {
        run_interactive_enigma_input();
        return;
    }
    if(options.cyclometer)
    {
        create_cycles();
        return;
    }
    if (options.bomb)
    {
        assertmsg(options.crib != NULL && options.ciphertext != NULL, "Input a valid known and cipher text");
        assertmsg(options.crib_offset < 0 ||
            (strlen(options.crib) + options.crib_offset > strlen(options.ciphertext)), "Bad crib offset");
        start_turing_bomb(options.crib, options.ciphertext, options.crib_offset);
    }
}
