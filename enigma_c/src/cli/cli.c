#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "cli_enigma.h"
#include "helper/helper.h"
#include "cyclometer/cycle_generator.h"
#include "turing_bomb/turing_bomb.h"
#include "gui/start_gui/start_gui.h"


#define INPUT_BUFFER_SIZE  1024
#define NUM_MODES          6

#if defined(_WIN32) || defined(_WIN64)
#define strtok_r strtok_s
#endif

//TODO verbose

/*----------GENERAL----------*/
#define HELP                        "--help"
#define HELP_SHORT                  "-h"
#define HELP_SHORT_CHAR             'h'
#define GUI                         "--gui"
#define GUI_SHORT                   "-g"

/*----------ENIGMA INTERACTIVE CLI----------*/
#define INTERACTIVE_ENIGMA          "--interactive"
#define INTERACTIVE_ENIGMA_SHORT    "-i"
#define INTERACTIVE_ENIGMA_CHAR     'i'

/*----------CYCLOMETER----------*/
#define CYCLOMETER                  "--cyclometer"
#define CYCLOMETER_SHORT            "-c"
#define CYCLOMETER_CHAR             'c'

/*----------BOMB----------*/
#define BOMB                        "--bomb"
#define BOMB_SHORT                  "-b"
#define BOMB_CHAR                   'b'
#define CIPHERTEXT                  "--ciphertext"
#define CIPHERTEXT_SHORT            "-ct"
#define CRIB                        "--crib"
#define CRIB_SHORT                  "-cr"
#define CRIB_OFFSET                 "--crib-offset"
#define CRIB_OFFSET_SHORT           "-co"

enum CliMode
{
    MODE_HELP = (1 << 0),
    MODE_ENIGMA = (1 << 1),
    MODE_INTERACTIVE_ENIGMA = (1 << 2),
    MODE_CYCLOMETER = (1 << 3),
    MODE_BOMB = (1 << 4),
    MODE_GUI = (1 << 5)
};

typedef struct
{
    char *ciphertext;
    char *crib;
    int32_t crib_offset;
    uint8_t active_mode;
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
    printf("%6s, %-15s | %-40s\n\n", CRIB_OFFSET_SHORT, CRIB_OFFSET, "crib offset");
    puts("Examples:");
    puts("\tenigma " BOMB_SHORT " " CIPHERTEXT_SHORT
            " OKQDRXACYEHWQDVHBAOXFPNMCQAILNBOGVODGJSZJSRPOWYSKKDBVJSHHMQBSKMBBLRLQUJFAFRDBFWFMCHUSXPBFJNKAINU "
            CRIB_SHORT " WETTERBERICHT " CRIB_OFFSET_SHORT " 0");
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

static void display_help_dialog(uint8_t cli_options)
{
    for (uint8_t mode = 0; mode < NUM_MODES && cli_options != 0; ++mode)
    {
        switch (cli_options & (1 << mode))
        {
            case MODE_ENIGMA:
                cli_options &= ~MODE_ENIGMA;
                print_enigma_help();
                break;
            case MODE_CYCLOMETER:
                cli_options &= ~MODE_CYCLOMETER;
                NULL;
                //TODO cyclometer
                break;
            case MODE_INTERACTIVE_ENIGMA:
                cli_options &= ~MODE_INTERACTIVE_ENIGMA;
                //TODO interactive mode help?
                break;
            case MODE_BOMB:
                cli_options &= ~MODE_BOMB;
                print_bomb_help();
                break;
        }
    }
}

static void query_help(const uint8_t cli_options)
{
    if(cli_options != 0)
    {
        display_help_dialog(cli_options);
        return;
    }

    //TODO cyclometer
    puts("Usage: enigma [OPTIONS]...");
    puts("-a to show all options");
    puts("-e for additional enigma help");
    puts("-b for additional bomb help");

    char buffer[INPUT_BUFFER_SIZE];
    while (my_getline(buffer, INPUT_BUFFER_SIZE) == 0)
        ;

    char *save_ptr;
    const char *options = strtok_r(buffer, " ", &save_ptr);
    while (options != NULL)
    {
        if (string_equals(options, "-a"))
        {
            print_help();
            print_bomb_help();
            print_enigma_help();
        } else if (string_equals(ENIGMA_SHORT, options))
        {
            print_enigma_help();
        } else if (string_equals(BOMB_SHORT, options))
        {
            print_bomb_help();
        }
        options = strtok_r(NULL, " ", &save_ptr);
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
        } else if (string_equals(CRIB, argv[i]) ||
                   string_equals(CRIB_SHORT, argv[i]))
        {
            options->crib = argv[++i];
        } else if (string_equals(CRIB_OFFSET, argv[i]) ||
                   string_equals(CRIB_OFFSET_SHORT, argv[i]))
        {
            assertmsg(get_number_from_string(argv[++i], &options->crib_offset) == 0, "Bad crib offset");
        } else
        {
            printf("Invalid option: %s\n", argv[i]);
            exit(1);
        }
        i++;
    }
}

static void parse_chained_flags(CliOptions *options, char *arg)
{
    if(arg[0] == '-' && arg[1] != '-' && strlen(arg) > 1)
    {
        for (uint8_t j = 1; arg[j] != 0; ++j)
        {
            switch(arg[j])
            {
                case HELP_SHORT_CHAR:
                    options->active_mode |= MODE_HELP;
                    break;
                case ENIGMA_CHAR:
                    options->active_mode |= MODE_ENIGMA;
                    break;
                case INTERACTIVE_ENIGMA_CHAR:
                    options->active_mode |= MODE_INTERACTIVE_ENIGMA;
                    break;
                case CYCLOMETER_CHAR:
                    options->active_mode |= MODE_CYCLOMETER;
                    break;
                case BOMB_CHAR:
                    options->active_mode |= MODE_BOMB;
                    break;
                default:
                    fprintf(stderr, "Invalid flag %c, try running -h for help\n", arg[j]);
                    exit(1);
            }
        }
    }
    else
    {
        fprintf(stderr, "Invalid flag %s\n", arg);
        exit(1);
    }
}

static void save_input(CliOptions *options, const int argc, char *argv[])
{
    int32_t i = 1;
    while (i < argc)
    {
        char *arg = argv[i];
        if (string_equals(HELP, arg) ||
            string_equals(HELP_SHORT, arg))
        {
            options->active_mode |= MODE_HELP;
        }
        else if (string_equals(INTERACTIVE_ENIGMA, arg) ||
                 string_equals(INTERACTIVE_ENIGMA_SHORT, arg))
        {
            options->active_mode |= MODE_INTERACTIVE_ENIGMA;
        }
        else if (string_equals(ENIGMA, arg) ||
                 string_equals(ENIGMA_SHORT, arg))
        {
            options->active_mode |= MODE_ENIGMA;
        }
        else if (string_equals(CYCLOMETER, arg) ||
                 string_equals(CYCLOMETER_SHORT, arg))
        {
            options->active_mode |= MODE_CYCLOMETER;
        }
        else if (string_equals(BOMB, arg) ||
                 string_equals(BOMB_SHORT, arg))
        {
            options->active_mode |= MODE_BOMB;
        }
        else if (string_equals(GUI, arg) ||
                 string_equals(GUI_SHORT, arg))
        {
            options->active_mode |= MODE_GUI;
        }
        else
        {
            parse_chained_flags(options, arg);
        }
        i++;
    }
}

static void validate_bomb_input(const CliOptions *options)
{
    assertmsg(options->crib != NULL && options->ciphertext != NULL, "Input a valid known and cipher text");
    assertmsg(options->crib_offset >= 0 ||
              (strlen(options->crib) + options->crib_offset <= strlen(options->ciphertext)), "Bad crib offset");
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
    for (int mode = 0; mode < NUM_MODES; ++mode)
    {
        switch (options->active_mode & (1 << mode))
        {
            case MODE_HELP:
                query_help(options->active_mode & ~MODE_HELP);
                // Chained flags only for help
                exit(0);
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
                save_bomb_input(options, argc, ++argv);
                validate_bomb_input(options);
                normalize_bomb_options(options);
                start_turing_bomb(options->crib, options->ciphertext, options->crib_offset);
                break;
        }
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
        puts("Chained help flags like -he for Enigma help are allowed");
        exit(1);
    }

    CliOptions options = {0};

    save_input(&options, argc, argv);
    select_run_mode(&options, argc, argv);
}
