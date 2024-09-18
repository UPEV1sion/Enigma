#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cyclometer/cyclometer.h"
#include "cli.h"
#include "helper/helper.h"
#include "enigma/enigma.h"
#include "turing_bomb/brute_force.h"

#define INPUT_BUFFER_SIZE  64

/*----------GENERAL----------*/
#define HELP        "--help"
#define HELP_SHORT  "-h"

/*----------ENIGMA----------*/
#define INTERACTIVE            "--interactive"
#define INTERACTIVE_SHORT      "-i"
#define ENIGMA                 "--enigma"
#define ENIGMA_SHORT           "-e"
#define CYCLOMETER             "--cyclometer"
#define CYCLOMETER_SHORT       "-c"
#define ROTOR_ONE              "--rotor-one"
#define ROTOR_ONE_SHORT        "-r1"
#define ROTOR_TWO              "--rotor-two"
#define ROTOR_TWO_SHORT        "-r2"
#define ROTOR_THREE            "--rotor-three"
#define ROTOR_THREE_SHORT      "-r3"
#define ROTOR_FOUR             "--rotor-four"
#define ROTOR_FOUR_SHORT       "-r4"
#define ROTOR_OFFSETS          "--ring-positions"
#define ROTOR_OFFSETS_SHORT    "-ro"
#define ROTOR_POSITIONS        "--rotor-positions"
#define ROTOR_POSITIONS_SHORT  "-rp"
#define REFLECTOR              "--reflector"
#define REFLECTOR_SHORT        "-rf"
#define PLUGBOARD              "--plugboard"
#define PLUGBOARD_SHORT        "-pb"
#define PLAINTEXT              "--plaintext"
#define PLAINTEXT_SHORT        "-pt"

/*----------BOMB----------*/
#define BOMB              "--bomb"
#define BOMB_SHORT        "-b"
#define CIPHERTEXT        "--ciphertext"
#define CIPHERTEXT_SHORT  "-ct"
#define KNOWN_TEXT        "--known-plaintext"
#define KNOWN_TEXT_SHORT  "-kp"

typedef struct
{
    unsigned char enigma: 1;
    unsigned char bomb: 1;
    unsigned char interactive: 1;
    unsigned char help: 1;
    char *enigma_type;
    char *rotor_one_type;
    char *rotor_two_type;
    char *rotor_three_type;
    char *rotor_four_type;
    char *rotor_offsets;
    char *rotor_positions;
    char *reflector_type;
    char *plugboard;
    char *plaintext;
    char *ciphertext;
    char *known_text;
} CliOptions;

static int32_t string_equals(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

static void print_enigma_help(void)
{
    puts("\n\n");
    puts("Usage: enigma [OPTIONS]...");
    puts("Encrypts/decrypts text using the Enigma machine\n");
    puts("Enigma options:");
    printf("%6s, %-15s | %-40s\n", "-short", "--long", "description");
    for (uint16_t i = 0; i < 41; ++i) printf("%c", '=');
    printf("\n%6s, %-15s | %-40s\n", "-e", "--enigma", "Enigma type (M3, M4)");
    printf("%6s, %-15s | %-40s\n", "-r1", "--rotor-one", "first rotor type (1 - 8)");
    printf("%6s, %-15s | %-40s\n", "-r2", "--rotor-two", "second rotor type (1 - 8)");
    printf("%6s, %-15s | %-40s\n", "-r3", "--rotor-three", "third rotor type (1 - 8)");
    printf("%6s, %-15s | %-40s\n", "-r4", "--rotor-four", "fourth rotor type (1 - 8)");
    printf("%6s, %-15s | %-40s\n", "-ro", "--offsets", "rotor offset (ABC, AABC, etc)");
    printf("%6s, %-15s | %-40s\n", "-rp", "--positions", "rotor position (ABC, AABC, etc)");
    printf("%6s, %-15s | %-40s\n", "-rf", "--reflector", "reflector type (B, C)");
    printf("%6s, %-15s | %-40s\n", "-pb", "--plugboard", "plugboard (e.g. AB CD EF)");
    printf("%6s, %-15s | %-40s\n\n", "-pt", "--plaintext", "plaintext (A secret Text)");
    puts("Example:");
    puts("\tenigma -e M3 -r1 1 -r2 2 -r3 3 -ro ABC -rp ABC -rf B -pb AB CD EF -pt loremipsum");
}

static void print_bomb_help(void)
{
    puts("\n\n");
    puts("Usage: enigma [OPTIONS]...");
    puts("Decrypts message text using the a turing bomb implementation\n");
    puts("Bomb options:");
    printf("%6s, %-20s | %-40s\n", "-short", "--long", "description");
    for (uint16_t i = 0; i < 41; ++i) printf("%c", '=');
    printf("\n%6s, %-20s | %-40s\n", "-b", "--bomb", "bomb mode");
    printf("%6s, %-20s | %-40s\n", "-ct", "--ciphertext", "ciphertext");
    printf("%6s, %-20s | %-40s\n\n", "-kp", "--known-plaintext", "known plaintext");
    puts("Examples:");
    puts(
        "\tenigma -b -ct OKQDRXACYEHWQDVHBAOXFPNMCQAILNBOGVODGJSZJSRPOWYSKKDBVJSHHMQBSKMBBLRLQUJFAFRDBFWFMCHUSXPBFJNKAINU -kp WETTERBERICHT");
}

static void print_help(void)
{
    puts("\n\n");
    puts("Usage: enigma [OPTIONS]...");
    printf("%6s, %-15s | %-40s\n", "-short", "--long", "description");
    for (uint16_t i = 0; i < 41; ++i) printf("%c", '=');
    printf("\n%6s, %-15s | %-40s\n", "-h", "--help", "display this help and exit");
    printf("%6s, %-15s | %-40s\n", "-e", "--help", "enigma mode");
    printf("%6s, %-15s | %-40s\n", "-i", "--interactive", "configure the enigma interactively");
    printf("%6s, %-15s | %-80s\n", "-c", "--cyclometer", "generate all possible cycles of an enigma "
           "M3 with Rotors 1 - 3");
    printf("%6s, %-15s | %-40s\n", "-b", "--bomb", "bomb mode");
}

void query_help(void)
{
    puts("Usage: enigma [OPTIONS]...");
    puts("-a to show all options");
    puts("-e for additional enigma help");
    puts("-b for additional bomb help");
    char buffer[64];
    fgets(buffer, sizeof buffer, stdin);
    const size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    //Don't use strtok elsewhere util parsing is finished!
    //strtok stores string internally, and string gets overwritten
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

static void init_cli_options(CliOptions *options)
{
    options->enigma_type      = NULL;
    options->rotor_one_type   = NULL;
    options->rotor_two_type   = NULL;
    options->rotor_three_type = NULL;
    options->rotor_four_type  = NULL;
    options->rotor_offsets    = NULL;
    options->rotor_positions  = NULL;
    options->reflector_type   = NULL;
    options->plugboard        = NULL;
    options->plaintext        = NULL;
    options->interactive      = 0;
    options->help             = 0;
    options->bomb             = 0;
    options->enigma           = 0;
}

static void save_enigma_input(CliOptions *options, const int32_t argc, char *argv[])
{
    int32_t i = 1;
    while (i < argc)
    {
        if (string_equals(ENIGMA, argv[i]) ||
            string_equals(ENIGMA_SHORT, argv[i]))
        {
            options->enigma_type = argv[++i];
        }
        else if (string_equals(ROTOR_ONE, argv[i]) ||
                 string_equals(ROTOR_ONE_SHORT, argv[i]))
        {
            options->rotor_one_type = argv[++i];
        }
        else if (string_equals(ROTOR_TWO, argv[i]) ||
                 string_equals(ROTOR_TWO_SHORT, argv[i]))
        {
            options->rotor_two_type = argv[++i];
        }
        else if (string_equals(ROTOR_THREE, argv[i]) ||
                 string_equals(ROTOR_THREE_SHORT, argv[i]))
        {
            options->rotor_three_type = argv[++i];
        }
        else if (string_equals(ROTOR_FOUR, argv[i]) ||
                 string_equals(ROTOR_FOUR_SHORT, argv[i]))
        {
            options->rotor_four_type = argv[++i];
        }
        else if (string_equals(ROTOR_OFFSETS, argv[i]) ||
                 string_equals(ROTOR_OFFSETS_SHORT, argv[i]))
        {
            options->rotor_offsets = argv[++i];
        }
        else if (string_equals(ROTOR_POSITIONS, argv[i]) ||
                 string_equals(ROTOR_POSITIONS_SHORT, argv[i]))
        {
            options->rotor_positions = argv[++i];
        }
        else if (string_equals(REFLECTOR, argv[i]) ||
                 string_equals(REFLECTOR_SHORT, argv[i]))
        {
            options->reflector_type = argv[++i];
        }
        else if (string_equals(PLUGBOARD, argv[i]) ||
                 string_equals(PLUGBOARD_SHORT, argv[i]))
        {
            size_t bufferSize = 1;
            for (uint16_t j = i + 1; j < argc && argv[j][0] != '-'; j++)
            {
                bufferSize += strlen(argv[j]);
            }
            options->plugboard = calloc(bufferSize, sizeof(uint8_t));
            assertmsg(options->plugboard != NULL, "options->plugboard == NULL");
            for (uint16_t j = i + 1; j < argc && argv[j][0] != '-'; ++j)
            {
                strcat(options->plugboard, argv[i + 1]);
                i++;
            }
        }
        else if (string_equals(PLAINTEXT, argv[i]) ||
                 string_equals(PLAINTEXT_SHORT, argv[i]))
        {
            options->plaintext = argv[i + 1];
            i++;
        }
        else
        {
            printf("Invalid option: %s\n", argv[i]);
            exit(1);
        }

        i++;
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
        else if (string_equals(KNOWN_TEXT, argv[i]) ||
                 string_equals(KNOWN_TEXT_SHORT, argv[i]))
        {
            options->known_text = argv[++i];
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
        puts("Help");
        options->help = 1;

        return;
    }
    if (string_equals(INTERACTIVE, argv[1]) ||
        string_equals(INTERACTIVE_SHORT, argv[1]))
    {
        options->interactive = 1;
        return;
    }
    if (string_equals(ENIGMA, argv[1]) ||
        string_equals(ENIGMA_SHORT, argv[1]))
    {
        options->enigma = 1;
        save_enigma_input(options, argc, argv);
        return;
    }
    if (string_equals(CYCLOMETER, argv[1]) ||
        string_equals(CYCLOMETER_SHORT, argv[1]))
    {
        create_cycles();
        exit(0);
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

static int32_t validate_cli_options(const CliOptions *options)
{
    if (options->enigma_type == NULL) return 1;
    if (!string_equals(options->enigma_type, "M3") &&
        !string_equals(options->enigma_type, "M4"))
        return 1;
    if (options->plaintext == NULL) return 1;
    if (options->rotor_offsets == NULL) return 1;
    if (options->rotor_positions == NULL) return 1;
    if (options->reflector_type == NULL) return 1;
    if (options->rotor_one_type == NULL) return 1;
    if (options->rotor_two_type == NULL) return 1;
    if (options->rotor_three_type == NULL) return 1;
    if (string_equals(options->enigma_type, "M4")
        && options->rotor_four_type == NULL)
        return 1;
    int32_t enigma_type = options->enigma_type[1] - '0';
    int32_t rotor_num[4];
    rotor_num[0] = options->rotor_one_type[0] - '0';
    rotor_num[1] = options->rotor_two_type[0] - '0';
    rotor_num[2] = options->rotor_three_type[0] - '0';
    if (enigma_type == 4) rotor_num[3] = options->rotor_four_type[0] - '0';
    bool rotor_seen[8] = {false};
    for (uint16_t i = 0; i < enigma_type; ++i)
    {
        if (rotor_seen[rotor_num[i]]) return 1;
        rotor_seen[rotor_num[i]] = true;
    }
    if (options->plugboard != NULL &&
        strlen(options->plugboard) > 0 &&
        has_duplicates(options->plugboard))
        return 1;
    return 0;
}

static void normalize_cli_options(const CliOptions *options)
{
    if (options->bomb)
    {
        to_upper_case(options->ciphertext);
        remove_none_alnum(options->ciphertext);
        to_upper_case(options->known_text);
        remove_none_alnum(options->known_text);
        return;
    }
    to_upper_case(options->enigma_type);
    remove_none_alnum(options->enigma_type);
    to_upper_case(options->rotor_one_type);
    remove_none_alnum(options->rotor_one_type);
    to_upper_case(options->rotor_two_type);
    remove_none_alnum(options->rotor_two_type);
    to_upper_case(options->rotor_three_type);
    remove_none_alnum(options->rotor_three_type);
    if (string_equals(options->enigma_type, "M4"))
    {
        to_upper_case(options->rotor_four_type);
        remove_none_alnum(options->rotor_four_type);
    }
    to_upper_case(options->rotor_offsets);
    remove_none_alnum(options->rotor_offsets);
    to_upper_case(options->rotor_positions);
    remove_none_alnum(options->rotor_positions);
    to_upper_case(options->reflector_type);
    remove_none_alnum(options->reflector_type);
    to_upper_case(options->plugboard);
    remove_none_alnum(options->plugboard);
    to_upper_case(options->plaintext);
    remove_none_alnum(options->plaintext);
}


static int32_t enigma_type_char_to_int(const char *enigma_type)
{
    if (string_equals("M3", enigma_type))
    {
        return M3;
    }
    if (string_equals("M4", enigma_type))
    {
        return M4;
    }
    return -1;
}

static Enigma* query_input_none_interactive(const CliOptions *options)
{
    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "enigma == NULL");
    assertmsg(validate_cli_options(options) == 0, "Input validation failed");
    normalize_cli_options(options);

    enigma->type   = enigma_type_char_to_int(options->enigma_type);
    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors, "enigma->rotors == NULL");
    enigma->rotors[0] = create_rotor(options->rotor_one_type[0] - '0',
                                     options->rotor_positions[0] - 'A',
                                     options->rotor_offsets[0] - 'A');
    enigma->rotors[1] = create_rotor(options->rotor_two_type[0] - '0',
                                     options->rotor_positions[1] - 'A',
                                     options->rotor_offsets[1] - 'A');
    enigma->rotors[2] = create_rotor(options->rotor_three_type[0] - '0',
                                     options->rotor_positions[2] - 'A',
                                     options->rotor_offsets[2] - 'A');
    if (enigma->type == M4)
    {
        enigma->rotors[3] = create_rotor(options->rotor_four_type[0] - '0',
                                         options->rotor_positions[3] - 'A',
                                         options->rotor_offsets[3] - 'A');
    }

    enigma->reflector = create_reflector_by_type(options->reflector_type[0]);

    if (options->plugboard == NULL)
    {
        enigma->plugboard = create_plugboard("");
    }
    else
    {
        enigma->plugboard = create_plugboard(options->plugboard);
    }
    const size_t plaintext_len = strlen(options->plaintext);
    enigma->plaintext          =
            malloc(plaintext_len + 1);
    assertmsg(enigma->plaintext != NULL, "enigma->plaintext == NULL");
    memcpy(enigma->plaintext, options->plaintext, plaintext_len + 1);
    free(options->plugboard);
    return enigma;
}

Enigma* query_input_interactive(void)
{
    //TODO validate input
    char input[INPUT_BUFFER_SIZE];
    char secondary_input[INPUT_BUFFER_SIZE];
    char ternary_input[INPUT_BUFFER_SIZE];

    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "enigma == NULL");


    printf("Enigma type (M3, M4): ");
    while (mygetline(input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(input);
    remove_none_alnum(input);
    enigma->type   = enigma_type_char_to_int(input);
    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors != NULL, "enigma->rotors == NULL");
    puts("");

    printf("First rotor type (1, 2, 3, 4, 5, 6, 7, 8): ");
    while (mygetline(input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(input);
    remove_none_alnum(input);
    printf("First rotor position (A, B, C, etc): ");
    while (mygetline(secondary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(secondary_input);
    remove_none_alnum(secondary_input);
    printf("First rotor offset (A, B, C, etc): ");
    while (mygetline(ternary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(ternary_input);
    remove_none_alnum(ternary_input);
    enigma->rotors[0] = create_rotor(input[0] - '0', secondary_input[0] - 'A',
                                     ternary_input[0] - 'A');

    puts("");

    printf("Second rotor type (1, 2, 3, 4, 5, 6, 7, 8): ");
    while (mygetline(input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(input);
    remove_none_alnum(input);
    printf("Second rotor position (A, B, C, etc): ");
    while (mygetline(secondary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(secondary_input);
    remove_none_alnum(secondary_input);
    printf("Second rotor offset (A, B, C, etc): ");
    while (mygetline(ternary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(ternary_input);
    remove_none_alnum(ternary_input);
    enigma->rotors[1] = create_rotor(input[0] - '0', secondary_input[0] - 'A',
                                     ternary_input[0] - 'A');
    puts("");

    printf("Third rotor type (1, 2, 3, 4, 5, 6, 7, 8): ");
    while (mygetline(input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(input);
    remove_none_alnum(input);
    printf("Third rotor position (A, B, C, etc): ");
    while (mygetline(secondary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(secondary_input);
    remove_none_alnum(secondary_input);
    printf("Third rotor offset (A, B, C, etc): ");
    while (mygetline(ternary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(ternary_input);
    remove_none_alnum(ternary_input);
    enigma->rotors[2] = create_rotor(input[0] - '0', secondary_input[0] - 'A',
                                     ternary_input[0] - 'A');
    puts("");

    if (enigma->type == M4)
    {
        printf("Fourth rotor type (1, 2, 3, 4, 5, 6, 7, 8): ");
        while (mygetline(input, INPUT_BUFFER_SIZE) == 0)
            ;
        to_upper_case(input);
        remove_none_alnum(input);
        printf("Fourth rotor position (A, B, C, etc): ");
        while (mygetline(secondary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        to_upper_case(secondary_input);
        remove_none_alnum(secondary_input);
        printf("Fourth rotor offset (A, B, C, etc): ");
        while (mygetline(ternary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        to_upper_case(ternary_input);
        remove_none_alnum(ternary_input);
        enigma->rotors[3] = create_rotor(
            input[0] - '0', secondary_input[0] - 'A', ternary_input[0] - 'A');
        puts("");
    }

    printf("Reflector type (B, C): ");
    while (mygetline(input, INPUT_BUFFER_SIZE) == 0)
        ;
    to_upper_case(input);
    remove_none_alnum(input);
    enigma->reflector = create_reflector_by_type(input[0]);
    puts("");

    printf("Plugboard (e.g. AB CD EF, EMPTY for standard): ");
    if (mygetline(input, INPUT_BUFFER_SIZE) > 0)
    {
        to_upper_case(input);
        remove_none_alnum(input);
        enigma->plugboard = create_plugboard(input);
    }
    enigma->plugboard = create_plugboard(NULL);
    puts("");

    printf("Plaintext: ");

    const size_t plain_len = mygetline(input, INPUT_BUFFER_SIZE);
    enigma->plaintext      = malloc(plain_len);
    to_upper_case(input);
    remove_none_alnum(input);
    strcpy(enigma->plaintext, input);
    puts("");

    return enigma;
}

static void free_stack_enigma(const Enigma *enigma)
{
    free(enigma->rotors[0]->notch);
    free(enigma->rotors[0]);
    free(enigma->rotors[1]->notch);
    free(enigma->rotors[1]);
    free(enigma->rotors[2]->notch);
    free(enigma->rotors[2]);
    free(enigma->rotors);
    free(enigma->plugboard->plugboard_data);
    free(enigma->plugboard);
    free(enigma->reflector->wiring);
    free(enigma->reflector);
    free(enigma->plaintext);
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

    CliOptions options;

    init_cli_options(&options);
    save_input(&options, argc, argv);

    if (options.help)
    {
        query_help();
        exit(0);
    }

    if (options.bomb)
    {
        crack_enigma(options.known_text, options.ciphertext);
        exit(0);
    }

    const Enigma *enigma;

    if (options.interactive)
    {
        enigma = query_input_interactive();
    }
    else
    {
        enigma = query_input_none_interactive(&options);
    }

    uint8_t *text              = traverse_enigma(enigma);
    const size_t plaintext_len = strlen(enigma->plaintext);
    for (size_t i = 0; i < plaintext_len; i++)
    {
        printf("%c", text[i] + 'A');
    }
    printf("\n");

    free_stack_enigma(enigma);
    free(text);
}
