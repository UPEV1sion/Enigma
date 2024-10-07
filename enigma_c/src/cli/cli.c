#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cyclometer/cyclometer.h"
#include "cli.h"

#include "cli_enigma.h"
#include "helper/helper.h"
#include "turing_bomb/turing_bomb.h"
#include "gui/start_gui/start_gui.h"


#define INPUT_BUFFER_SIZE  1024

/*----------GENERAL----------*/
#define HELP        "--help"
#define HELP_SHORT  "-h"
#define GUI         "--gui"
#define GUI_SHORT   "-g"

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

enum CliMode
{
    MODE_HELP,
    MODE_ENIGMA,
    MODE_INTERACTIVE_ENIGMA,
    MODE_CYCLOMETER,
    MODE_BOMB,
    MODE_GUI
};

typedef struct
{
    char *ciphertext;
    char *crib;
    int32_t crib_offset;
    enum CliMode mode;
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
        options->mode = MODE_HELP;
        return;
    }
    if (string_equals(INTERACTIVE_ENIGMA, argv[1]) ||
        string_equals(INTERACTIVE_ENIGMA_SHORT, argv[1]))
    {
        options->mode = MODE_INTERACTIVE_ENIGMA;
        return;
    }
    if (string_equals(ENIGMA, argv[1]) ||
        string_equals(ENIGMA_SHORT, argv[1]))
    {
        options->mode = MODE_ENIGMA;
        return;
    }
    if (string_equals(CYCLOMETER, argv[1]) ||
        string_equals(CYCLOMETER_SHORT, argv[1]))
    {
        options->mode = MODE_CYCLOMETER;
        return;
    }
    if (string_equals(BOMB, argv[1]) ||
        string_equals(BOMB_SHORT, argv[1]))
    {
        options->mode = MODE_BOMB;
        save_bomb_input(options, argc, ++argv);
    }
    if(string_equals(GUI, argv[1]) ||
       string_equals(GUI_SHORT, argv[1]))
    {
        options->mode = MODE_GUI;
        return;
    }
    else
    {
        printf("Invalid option: %s\n", argv[1]);
        exit(1);
    }
}

static void validate_bomb_input(const CliOptions *restrict options) {
    assertmsg(options->crib != NULL && options->ciphertext != NULL, "Input a valid known and cipher text");
    assertmsg(options->crib_offset < 0 ||
              (strlen(options->crib) + options->crib_offset > strlen(options->ciphertext)), "Bad crib offset");
}

static void normalize_bomb_options(CliOptions *restrict options)
{
    int err_code = 0;
    err_code |= to_uppercase(options->ciphertext);
    err_code |= remove_non_alnum(options->ciphertext);
    err_code |= to_uppercase(options->crib);
    err_code |= remove_non_alnum(options->crib);
    assertmsg(err_code == 0, "Bomb normalization failed");
}


static void select_run_mode(CliOptions *options, const int argc, char *argv[])
{
    switch(options->mode)
    {
        case MODE_HELP:
            query_help();
            break;
        case MODE_GUI:
            run_start_gui(argv);
            break;
        case MODE_ENIGMA:
            query_enigma_input(argc, argv);
            break;
        case MODE_INTERACTIVE_ENIGMA:
            run_interactive_enigma_input();
            break;
        case MODE_CYCLOMETER:
            create_cycles();
            break;
        case MODE_BOMB:
            validate_bomb_input(options);
            normalize_bomb_options(options);
            start_turing_bomb(options->crib, options->ciphertext, options->crib_offset);
            break;
        default:
            fprintf(stderr, "Unknown mode");
            exit(1);
    }
}

void query_input(const int argc, char *argv[])
{
    if (argc < 2)
    {
        puts("Usage: ./enigma [OPTIONS]");
        puts("Options:");
        puts(GUI " / " GUI_SHORT " for GUI");
        puts("CLI use is implicit");
        puts(HELP " / " HELP_SHORT " for help");
        exit(1);
    }

    CliOptions options = {0};

    save_input(&options, argc, argv);
    select_run_mode(&options, argc, argv);
}
