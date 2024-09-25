#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cyclometer/cyclometer.h"
#include "cli.h"
#include "helper/helper.h"
#include "enigma/enigma.h"
#include "turing_bomb/turing_bomb.h"

#define INPUT_BUFFER_SIZE  1024

/*----------ENIGMA----------*/
#define INTERACTIVE            "--interactive"
#define INTERACTIVE_SHORT      "-i"
#define ENIGMA                 "--enigma"
#define ENIGMA_SHORT           "-e"
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

/*----------CYCLOMETER----------*/
#define CYCLOMETER             "--cyclometer"
#define CYCLOMETER_SHORT       "-c"

/*----------BOMB----------*/
#define BOMB              "--bomb"
#define BOMB_SHORT        "-b"
#define CIPHERTEXT        "--ciphertext"
#define CIPHERTEXT_SHORT  "-ct"
#define KNOWN_TEXT        "--known-plaintext"
#define KNOWN_TEXT_SHORT  "-kp"

typedef struct
{
    enum ENIGMA_TYPE enigma_type;
    enum ROTOR_TYPE rotor_one_type;
    enum ROTOR_TYPE rotor_two_type;
    enum ROTOR_TYPE rotor_three_type;
    enum ROTOR_TYPE rotor_four_type;
    enum REFLECTOR_TYPE reflector_type;
    char *rotor_offsets;
    char *rotor_positions;
    char *plugboard;
    char *plaintext;
    char *ciphertext;
    char *known_text;
    uint8_t enigma: 1;
    uint8_t bomb: 1;
    uint8_t interactive: 1;
    uint8_t help: 1;
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
        buffer[len - 1] = 0;
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

static void parse_rotor_input(CliOptions *options, char *argv[], int32_t *i, const enum ROTOR_TYPE rotor_num)
{
    int32_t rotor_type = 0;
    assertmsg(get_number_from_string(argv[++(*i)], &rotor_type) == 0, "Number parsing failed");

    switch (rotor_num) {
        case ROTOR_1: options->rotor_one_type = rotor_type; break;
        case ROTOR_2: options->rotor_two_type = rotor_type; break;
        case ROTOR_3: options->rotor_three_type = rotor_type; break;
        case ROTOR_4: options->rotor_four_type = rotor_type; break;
        default: break;
    }
}

static void save_enigma_input(CliOptions *options, const int32_t argc, char *argv[])
{
    int32_t i = 1;
    while (i < argc)
    {
        if (string_equals(ENIGMA, argv[i]) ||
            string_equals(ENIGMA_SHORT, argv[i]))
        {
            options->enigma_type = string_equals(argv[++i], "M3") ? ENIGMA_M3 : ENIGMA_M4;
        }
        else if (string_equals(ROTOR_ONE, argv[i]) ||
                 string_equals(ROTOR_ONE_SHORT, argv[i]))
        {
            parse_rotor_input(options, argv, &i, ROTOR_1);
        }
        else if (string_equals(ROTOR_TWO, argv[i]) ||
                 string_equals(ROTOR_TWO_SHORT, argv[i]))
        {
            parse_rotor_input(options, argv, &i, ROTOR_2);
        }
        else if (string_equals(ROTOR_THREE, argv[i]) ||
                 string_equals(ROTOR_THREE_SHORT, argv[i]))
        {
            parse_rotor_input(options, argv, &i, ROTOR_3);
        }
        else if (string_equals(ROTOR_FOUR, argv[i]) ||
                 string_equals(ROTOR_FOUR_SHORT, argv[i]))
        {
            parse_rotor_input(options, argv, &i, ROTOR_4);
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
            options->reflector_type = string_equals(argv[++i], "B") ? UKW_B : UKW_C;
        }
        else if (string_equals(PLUGBOARD, argv[i]) ||
                 string_equals(PLUGBOARD_SHORT, argv[i]))
        {
            options->plugboard = strdup(argv[++i]);
            assertmsg(options->plugboard != NULL, "strdup failed");

            size_t total_len = strlen(options->plugboard);

            while (i + 1 < argc && argv[i + 1][0] != '-')
            {
                const size_t next_len = strlen(argv[++i]);
                options->plugboard = realloc(options->plugboard, total_len + next_len + 1);
                assertmsg(options->plugboard != NULL, "realloc failed");

                strcpy(options->plugboard + total_len, argv[i]);
                total_len += next_len;
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
    if (options->enigma_type != ENIGMA_M3 && options->enigma_type != ENIGMA_M4) return 1;
    if (options->plaintext == NULL) return 1;
    if (options->rotor_offsets == NULL) return 1;
    if (options->rotor_positions == NULL) return 1;
    if (options->reflector_type != UKW_B && options->reflector_type != UKW_C) return 1;
    if (options->rotor_one_type < ROTOR_1 || options->rotor_one_type > ROTOR_8) return 1;
    if (options->rotor_two_type < ROTOR_1 || options->rotor_two_type > ROTOR_8) return 1;
    if (options->rotor_three_type < ROTOR_1 || options->rotor_three_type > ROTOR_8) return 1;

    if (options->enigma_type == ENIGMA_M4
        && (options->rotor_four_type < ROTOR_1 || options->rotor_four_type > ROTOR_8))
        return 1;
    int32_t rotor_num[4];
    rotor_num[0] = options->rotor_one_type;
    rotor_num[1] = options->rotor_two_type;
    rotor_num[2] = options->rotor_three_type;
    if (options->enigma_type == ENIGMA_M4) rotor_num[3] = options->rotor_four_type;


    bool rotor_seen[8] = {false};
    for (uint16_t i = 0; i < options->enigma_type; ++i)
    {
        if (rotor_seen[rotor_num[i] - 1]) return 1;
        rotor_seen[rotor_num[i] - 1] = true;
    }
    if (options->plugboard != NULL &&
        strlen(options->plugboard) > 0 &&
        has_duplicates(options->plugboard))
        return 1;
    return 0;
}

static void normalize_cli_options(const CliOptions *options)
{
    int err_code = 0;
    if (options->bomb)
    {
        err_code |= to_uppercase(options->ciphertext);
        err_code |= remove_non_alnum(options->ciphertext);
        err_code |= to_uppercase(options->known_text);
        err_code |= remove_non_alnum(options->known_text);
        assertmsg(err_code == 0, "normalization failed");
        return;
    }
    err_code |= to_uppercase(options->rotor_offsets);
    err_code |= remove_non_alnum(options->rotor_offsets);
    err_code |= to_uppercase(options->rotor_positions);
    err_code |= remove_non_alnum(options->rotor_positions);
    err_code |= to_uppercase(options->plugboard);
    err_code |= remove_non_alnum(options->plugboard);
    err_code |= to_uppercase(options->plaintext);
    err_code |= remove_non_alnum(options->plaintext);
    assertmsg(err_code == 0, "normalization failed");
}

static int32_t enigma_type_char_to_int(const char *enigma_type)
{
    if (string_equals("M3", enigma_type))
    {
        return ENIGMA_M3;
    }
    if (string_equals("M4", enigma_type))
    {
        return ENIGMA_M4;
    }
    return -1;
}


static Enigma* create_enigma_from_cli_configuration(const CliOptions *options)
{
    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "enigma == NULL");
    assertmsg(validate_cli_options(options) == 0, "Input validation failed");
    normalize_cli_options(options);

    enigma->type   = options->enigma_type;
    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors, "enigma->rotors == NULL");
    enigma->rotors[0] = create_rotor(options->rotor_one_type,
                                     options->rotor_positions[0] - 'A',
                                     options->rotor_offsets[0] - 'A');
    enigma->rotors[1] = create_rotor(options->rotor_two_type,
                                     options->rotor_positions[1] - 'A',
                                     options->rotor_offsets[1] - 'A');
    enigma->rotors[2] = create_rotor(options->rotor_three_type,
                                     options->rotor_positions[2] - 'A',
                                     options->rotor_offsets[2] - 'A');
    if (enigma->type == ENIGMA_M4)
    {
        enigma->rotors[3] = create_rotor(options->rotor_four_type,
                                         options->rotor_positions[3] - 'A',
                                         options->rotor_offsets[3] - 'A');
    }

    enigma->reflector = create_reflector_by_type(options->reflector_type);

    if (options->plugboard == NULL)
    {
        enigma->plugboard = create_plugboard("");
    }
    else
    {
        enigma->plugboard = create_plugboard(options->plugboard);
    }

    enigma->plaintext = strdup(options->plaintext);
    assertmsg(enigma->plaintext != NULL, "enigma->plaintext == NULL");
    free(options->plugboard);
    return enigma;
}

Enigma* query_input_interactive(void)
{
    char primary_input[INPUT_BUFFER_SIZE];
    int32_t err_code = 0;

    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "enigma == NULL");

    printf("Enigma type (M3, M4): ");
    while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    err_code |= to_uppercase(primary_input);
    err_code |= remove_non_alnum(primary_input);
    assertmsg(err_code == 0, "normalization failed");
    enigma->type   = enigma_type_char_to_int(primary_input);
    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors != NULL, "enigma->rotors == NULL");
    puts("");

    char *numeration[4] = {"First", "Second", "Third", "Fourth"};
    for (uint8_t rotor_num = 0; rotor_num < enigma->type; ++rotor_num)
    {
        char secondary_input[INPUT_BUFFER_SIZE];
        char tertiary_input[INPUT_BUFFER_SIZE];

        printf("%s rotor type (1, 2, 3, 4, 5, 6, 7, 8): ", numeration[rotor_num]);
        while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code |= to_uppercase(primary_input);
        err_code |=remove_non_alnum(primary_input);
        assertmsg(err_code == 0, "normalization failed");
        printf("%s rotor position (A, B, C, etc): ", numeration[rotor_num]);
        while (my_getline(secondary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code |= to_uppercase(secondary_input);
        err_code |= remove_non_alnum(secondary_input);
        assertmsg(err_code == 0, "normalization failed");
        printf("%s rotor offset (A, B, C, etc): ", numeration[rotor_num]);
        while (my_getline(tertiary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code |= to_uppercase(tertiary_input);
        err_code |= remove_non_alnum(tertiary_input);
        assertmsg(err_code == 0, "normalization failed");
        enigma->rotors[rotor_num] = create_rotor(primary_input[0] - '0', secondary_input[0] - 'A',
                                                 tertiary_input[0] - 'A');

        puts("");
    }

    printf("Reflector type (B, C): ");
    while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    err_code |= to_uppercase(primary_input);
    err_code |= remove_non_alnum(primary_input);
    assertmsg(err_code == 0, "normalization failed");
    enigma->reflector = create_reflector_by_type(*primary_input);
    puts("");

    printf("Plugboard (e.g. AB CD EF, EMPTY for standard): ");
    if (my_getline(primary_input, INPUT_BUFFER_SIZE) > 0)
    {
        err_code |= to_uppercase(primary_input);
        err_code |= remove_non_alnum(primary_input);
        assertmsg(err_code == 0, "normalization failed");
        enigma->plugboard = create_plugboard(primary_input);
    }
    enigma->plugboard = create_plugboard(NULL);
    puts("");

    printf("Plaintext: ");
    while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    enigma->plaintext = strdup(primary_input);
    assertmsg(enigma->plaintext != NULL, "strdup failed");
    err_code |= to_uppercase(enigma->plaintext);
    err_code |= remove_non_alnum(enigma->plaintext);
    assertmsg(err_code == 0, "normalization failed");

    return enigma;
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
        exit(0);
    }

    if (options.bomb)
    {
        assertmsg(options.known_text != NULL && options.ciphertext != NULL, "Input a valid known and cipher text");
        //TODO 0 for now
        start_turing_bomb(options.known_text, options.ciphertext, 0);
        exit(0);
    }

    Enigma *enigma;

    if (options.interactive)
    {
        enigma = query_input_interactive();
    }
    else
    {
        enigma = create_enigma_from_cli_configuration(&options);
    }

    uint8_t *text              = traverse_enigma(enigma);
    const size_t plaintext_len = strlen(enigma->plaintext);
    for (size_t i = 0; i < plaintext_len; i++)
    {
        printf("%c", text[i] + 'A');
    }
    printf("\n");

    free_enigma(enigma);
    free(text);
}
