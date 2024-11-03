#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "cli_enigma.h"
#include "json/json.h"

//
// Created by Emanuel on 05.10.2024.
//

// TODO Error codes
// TODO make the CLI for the Bomb interactive & offsets standard AAA

#define INPUT_BUFFER_SIZE       (1024)
#define SEPARATOR_LENGTH        (128)

#define ERR_BAD_ENIGMA_TYPE     (1)
#define ERR_NO_PLAINTEXT        (2)
#define ERR_BAD_ROTOR_OFFSET    (3)
#define ERR_BAD_ROTOR_POSITIONS (4)
#define ERR_BAD_ROTOR_TYPE      (5)
#define ERR_BAD_REFLECTOR_TYPE  (6)
#define ERR_DUPLICATE_ROTORS    (7)
#define ERR_BAD_PLUGBOARD       (8)

/*----------ENIGMA----------*/
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

static void print_separator(void)
{
    for (uint8_t i = 0; i < SEPARATOR_LENGTH; ++i) printf("%c", '=');
    puts("");
}

void print_enigma_help(void)
{
    puts("\n\n");
    puts("Usage: ./enigma [OPTIONS]...");
    puts("Encrypts/decrypts text using the Enigma machine\n");
    puts("Enigma options:");
    printf("%6s, %-20s | %-40s\n", "-short", "--long", "description");
    print_separator();
    printf("%6s, %-20s | %-40s\n", ENIGMA_SHORT, ENIGMA, "Enigma type (M3, M4)");
    printf("%6s, %-20s | %-40s\n", ROTOR_ONE_SHORT, ROTOR_ONE, "first rotor type (1 - 8)");
    printf("%6s, %-20s | %-40s\n", ROTOR_TWO_SHORT, ROTOR_TWO, "second rotor type (1 - 8)");
    printf("%6s, %-20s | %-40s\n", ROTOR_THREE_SHORT, ROTOR_THREE, "third rotor type (1 - 8)");
    printf("%6s, %-20s | %-40s\n", ROTOR_FOUR_SHORT, ROTOR_FOUR, "fourth rotor type (beta, gamma) - Enigma M4 only");
    printf("%6s, %-20s | %-40s\n", ROTOR_OFFSETS_SHORT, ROTOR_OFFSETS, "rotor offset / ring setting (ABC, AABC, etc)");
    printf("%6s, %-20s | %-40s\n", ROTOR_POSITIONS_SHORT, ROTOR_POSITIONS, "rotor position (ABC, AABC, etc)");
    printf("%6s, %-20s | %-80s\n", REFLECTOR_SHORT, REFLECTOR, "reflector type (A, B, C) - Enigma M3 only,"
           " (B_THIN / b, C_THIN / c) Enigma M4 only");
    printf("%6s, %-20s | %-40s\n", PLUGBOARD_SHORT, PLUGBOARD, "plugboard (e.g. AB CD EF)");
    printf("%6s, %-20s | %-40s\n\n", PLAINTEXT_SHORT, PLAINTEXT, "plaintext (A secret Text)");
    puts("Note: Enigma type, rotor offsets, rotor positions, "
        "and plugboard settings are case-insensitive and will be automatically normalized.\n");
    puts("Example:");
    puts("\tenigma "ENIGMA_SHORT " M3 " ROTOR_ONE_SHORT " 1 " ROTOR_TWO_SHORT " 2 " ROTOR_THREE_SHORT " 3 "
        ROTOR_OFFSETS_SHORT " ABC " ROTOR_POSITIONS_SHORT " ABC " REFLECTOR_SHORT " B " PLUGBOARD_SHORT " AB CD EF "
        PLAINTEXT_SHORT " loremipsum");
}

static int32_t string_equals(const char *str1, const char *str2)
{
    return strcmp(str1, str2) == 0;
}

static void append_to_string(char **dest, const char *src)
{
    const size_t dest_len = *dest != NULL ? strlen(*dest) : 0;
    const size_t src_len  = strlen(src);

    char *new_str = realloc(*dest, dest_len + src_len + 1);
    assertmsg(new_str != NULL, "reallocate failed");

    *dest = new_str;
    strcpy(*dest + dest_len, src);
}

static bool only_contains_upper_alpha(const char *restrict str)
{
    const size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
    {
        if (!isupper(str[i])) return false;
    }

    return true;
}

static void parse_enigma_input(EnigmaCliOptions *options, int32_t *i, const int argc, char *argv[])
{
    if (*i + 1 >= argc) return;
    char *enigma_type = argv[++*i];
    to_uppercase(enigma_type);

    if (string_equals(enigma_type, "M3"))
    {
        options->enigma_type = ENIGMA_M3;
    }
    else if (string_equals(enigma_type, "M4"))
    {
        options->enigma_type = ENIGMA_M4;
    }
    else
    {
        fprintf(stderr, "\nInvalid enigma type: %s\n", enigma_type);
        fflush(stderr);
        exit(ERR_BAD_ENIGMA_TYPE);
    }
}

static void parse_first_three_rotor_input(EnigmaCliOptions *options, int32_t *i, const int argc, char *argv[],
                                          const enum ROTOR_TYPE rotor_num)
{
    if (*i + 1 >= argc) return;
    int32_t rotor_type = 0;
    assertmsg(get_number_from_string(argv[++*i], &rotor_type) == 0, "Number parsing failed, invalid rotor 1-3 type");

    switch (rotor_num)
    {
        case ROTOR_1:
            options->rotor_one_type = rotor_type;
            break;
        case ROTOR_2:
            options->rotor_two_type = rotor_type;
            break;
        case ROTOR_3:
            options->rotor_three_type = rotor_type;
            break;
        default:
            break;
    }
}

static void parse_fourth_rotor_input(EnigmaCliOptions *options, int32_t *i, char *argv[])
{
    char *rotor_four_type = argv[++*i];

    int32_t err_code = remove_non_alpha(rotor_four_type);
    err_code |= to_uppercase(rotor_four_type);
    assertmsg(err_code == 0, "Input normalization failed");

    if (string_equals(rotor_four_type, "BETA"))
    {
        options->rotor_four_type = ROTOR_BETA;
    }
    else if (string_equals(rotor_four_type, "GAMMA"))
    {
        options->rotor_four_type = ROTOR_GAMMA;
    }
    else
    {
        fprintf(stderr, "Please enter a valid fourth rotor (gamma or beta)");
        exit(ERR_BAD_ROTOR_TYPE);
    }
}

static void parse_reflector_input(EnigmaCliOptions *options, int32_t *i, const int argc, char *argv[])
{
    if (*i + 1 >= argc) return;
    char *reflector_type = argv[++*i];

    if (string_equals(reflector_type, "A"))
    {
        options->reflector_type = UKW_A;
    }
    else if (string_equals(reflector_type, "B"))
    {
        options->reflector_type = UKW_B;
    }
    else if (string_equals(reflector_type, "C"))
    {
        options->reflector_type = UKW_C;
    }
    else if (string_equals(reflector_type, "B_THIN") || string_equals(reflector_type, "b"))
    {
        options->reflector_type = UKW_B_THIN;
    }
    else if (string_equals(reflector_type, "C_THIN") || string_equals(reflector_type, "c"))
    {
        options->reflector_type = UKW_C_THIN;
    }
    else
    {
        fprintf(stderr, "\nInvalid reflector type: %s\n", reflector_type);
        fflush(stderr);
        exit(ERR_BAD_REFLECTOR_TYPE);
    }
}

static void parse_until_new_flag(char **option, int32_t *i, const int argc, char *argv[])
{
    if (option == NULL) return;
    *option = strdup(argv[++*i]);
    assertmsg(option != NULL, "strdup failed");

    while (*i + 1 < argc && argv[*i + 1][0] != '-')
    {
        append_to_string(option, argv[++*i]);
    }
}

static void save_enigma_input(EnigmaCliOptions *options, const int argc, char *argv[])
{
    int32_t i = 1;
    while (i < argc)
    {
        if (string_equals(ENIGMA, argv[i]) ||
            string_equals(ENIGMA_SHORT, argv[i]))
        {
            parse_enigma_input(options, &i, argc, argv);
        }
        else if (string_equals(ROTOR_ONE, argv[i]) ||
                 string_equals(ROTOR_ONE_SHORT, argv[i]))
        {
            parse_first_three_rotor_input(options, &i, argc, argv, ROTOR_1);
        }
        else if (string_equals(ROTOR_TWO, argv[i]) ||
                 string_equals(ROTOR_TWO_SHORT, argv[i]))
        {
            parse_first_three_rotor_input(options, &i, argc, argv, ROTOR_2);
        }
        else if (string_equals(ROTOR_THREE, argv[i]) ||
                 string_equals(ROTOR_THREE_SHORT, argv[i]))
        {
            parse_first_three_rotor_input(options, &i, argc, argv, ROTOR_3);
        }
        else if (string_equals(ROTOR_FOUR, argv[i]) ||
                 string_equals(ROTOR_FOUR_SHORT, argv[i]))
        {
            parse_fourth_rotor_input(options, &i, argv);
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
            parse_reflector_input(options, &i, argc, argv);
        }
        else if (string_equals(PLUGBOARD, argv[i]) ||
                 string_equals(PLUGBOARD_SHORT, argv[i]))
        {
            parse_until_new_flag(&options->plugboard, &i, argc, argv);
        }
        else if (string_equals(PLAINTEXT, argv[i]) ||
                 string_equals(PLAINTEXT_SHORT, argv[i]))
        {
            parse_until_new_flag(&options->plaintext, &i, argc, argv);
        }
        else
        {
            printf("Invalid option: %s\n", argv[i]);
            exit(1);
        }

        i++;
    }
}

static int32_t validate_cli_enigma_options(const EnigmaCliOptions *options)
{
    if (options->enigma_type != ENIGMA_M3 && options->enigma_type != ENIGMA_M4) return ERR_BAD_ENIGMA_TYPE;
    if (options->plaintext == NULL) return ERR_NO_PLAINTEXT;
    if (options->rotor_offsets == NULL) return ERR_BAD_ROTOR_OFFSET;
    if (!only_contains_upper_alpha(options->rotor_offsets)) return ERR_BAD_ROTOR_OFFSET;
    if (options->rotor_positions == NULL) return ERR_BAD_ROTOR_POSITIONS;
    if (!only_contains_upper_alpha(options->rotor_positions)) return ERR_BAD_ROTOR_POSITIONS;

    // This will fail in the create_reflector_by_type switch case and give a clearer error
     if (options->reflector_type < UKW_A || options->reflector_type > UKW_C) return ERR_BAD_REFLECTOR_TYPE;

    if (strlen(options->rotor_offsets) != options->enigma_type) return ERR_BAD_ROTOR_OFFSET;
    if (strlen(options->rotor_positions) != options->enigma_type) return ERR_BAD_ROTOR_POSITIONS;
    if (options->rotor_one_type < ROTOR_1 || options->rotor_one_type > ROTOR_8) return ERR_BAD_ROTOR_TYPE;
    if (options->rotor_two_type < ROTOR_1 || options->rotor_two_type > ROTOR_8) return ERR_BAD_ROTOR_TYPE;
    if (options->rotor_three_type < ROTOR_1 || options->rotor_three_type > ROTOR_8) return ERR_BAD_ROTOR_TYPE;

    if (options->enigma_type == ENIGMA_M3 && options->rotor_four_type != 0)
        return ERR_BAD_ROTOR_TYPE;
    if (options->enigma_type == ENIGMA_M4)
    {
        if (options->rotor_four_type != ROTOR_BETA && options->rotor_four_type != ROTOR_GAMMA)
            return ERR_BAD_ROTOR_TYPE;
        if (options->reflector_type != UKW_B_THIN && options->reflector_type != UKW_C_THIN)
            return ERR_BAD_REFLECTOR_TYPE;
    }

    int32_t rotor_num[4];
    rotor_num[0] = options->rotor_one_type;
    rotor_num[1] = options->rotor_two_type;
    rotor_num[2] = options->rotor_three_type;
    if (options->enigma_type == ENIGMA_M4) rotor_num[3] = options->rotor_four_type;


    bool rotor_seen[12] = {false};
    for (uint8_t i = 0; i < (uint8_t) options->enigma_type; ++i)
    {
        const uint8_t rotor_index = rotor_num[i] - 1;
        if (rotor_seen[rotor_index]) return ERR_DUPLICATE_ROTORS;
        rotor_seen[rotor_index] = true;
    }
    if (options->plugboard != NULL && strlen(options->plugboard) > 0 && has_duplicates(options->plugboard))
        return ERR_BAD_PLUGBOARD;
    return 0;
}

static int32_t normalize_input(char *input)
{
    return to_uppercase(input) | remove_non_alnum(input);
}

static void normalize_cli_options(const EnigmaCliOptions *options)
{
    assertmsg(normalize_input(options->rotor_offsets) == 0, "Rotor offsets normalization failed");
    assertmsg(normalize_input(options->rotor_positions) == 0, "Rotor positions normalization failed");
    normalize_input(options->plugboard);
    assertmsg(normalize_input(options->plaintext) == 0, "Plaintext normalization failed");
}

static enum ENIGMA_TYPE parse_enigma_type(const char *enigma_type)
{
    // The Enigma M1 is just like the M3 but without the plugboard.
    // TODO flag for m1 flagging plugboard
    if (string_equals("M3", enigma_type) || string_equals("M1", enigma_type))
    {
        return ENIGMA_M3;
    }
    if (string_equals("M4", enigma_type))
    {
        return ENIGMA_M4;
    }
    return -1;
}


static Enigma* create_enigma_from_cli_configuration(const EnigmaCliOptions *options)
{
    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "malloc failed");
    assertmsg(validate_cli_enigma_options(options) == 0, "Input validation failed. Please check your Enigma settings.");
    normalize_cli_options(options);

    enigma->type   = options->enigma_type;
    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors, "malloc failed");

    const enum ROTOR_TYPE rotor_types[4] = {
        options->rotor_one_type,
        options->rotor_two_type,
        options->rotor_three_type,
        options->rotor_four_type
    };

    for (uint8_t i = 0; i < (uint8_t) enigma->type; ++i)
    {
        enigma->rotors[i] = create_rotor_by_type(rotor_types[i],
                                                 options->rotor_positions[i] - 'A',
                                                 options->rotor_offsets[i] - 'A');
    }

    enigma->reflector = create_reflector_by_type(options->reflector_type);
    enigma->plugboard = create_plugboard(options->plugboard);

    enigma->plaintext = strdup(options->plaintext);
    assertmsg(enigma->plaintext != NULL, "strdup failed");

    return enigma;
}

static int32_t validate_enigma_type(const char *type)
{
    return (string_equals(type, "M3") || string_equals(type, "M4")) ? 0 : 1;
}

static int32_t validate_rotor_type(const char *rotor, const enum ROTOR_TYPE rotor_type)
{
    if (rotor_type >= ROTOR_1 && rotor_type <= ROTOR_3)
    {
        if (isdigit(*rotor))
        {
            return 0;
        }
        return 1;
    }
    if (rotor_type == ROTOR_4)
    {
        return (string_equals(rotor, "BETA") || string_equals(rotor, "GAMMA")) ? 0 : 1;
    }

    return 1;
}

static int32_t validate_rotor_positions_and_offsets(const char *input)
{
    return !isalpha(*input);
}

static int32_t validate_reflector(const enum ENIGMA_TYPE type, const char *reflector)
{
    if(type == ENIGMA_M3)
    {
        return (*reflector == UKW_A || *reflector == UKW_B || *reflector == UKW_C) ? 0 : 1;
    }
    if(type == ENIGMA_M4)
    {
        return (string_equals(reflector, "B_THIN") || string_equals(reflector, "C_THIN")) ? 0 : 1;
    }
    return 1;
}

static int32_t validate_plugboard(const char *plugboard)
{
    bool visited[ALPHABET_SIZE] = {false};
    const size_t plugboard_len = strlen(plugboard);
    if(plugboard_len % 2 != 0) return 1;
    for(size_t i = 0; i < plugboard_len; ++i)
    {
        const int32_t visited_index = plugboard[i] - 'A';
        if(visited[visited_index]) return 1;
        visited[visited_index] = true;
    }

    return 0;
}

static void parse_enigma_rotors_interactive(Enigma *enigma)
{
    enigma->rotors = malloc(enigma->type * sizeof(Rotor *));
    assertmsg(enigma->rotors != NULL, "malloc failed");

    int32_t err_code = 0;

    char *numeration[4] = {"First", "Second", "Third", "Fourth"};
    for (uint8_t rotor_num = 0; rotor_num < (uint8_t) enigma->type; ++rotor_num)
    {
        char primary_input[INPUT_BUFFER_SIZE];
        char secondary_input[INPUT_BUFFER_SIZE];
        char tertiary_input[INPUT_BUFFER_SIZE];

        do
        {
            if (rotor_num < 3)
            {
                printf("%s rotor type (1, 2, 3, 4, 5, 6, 7, 8): ", numeration[rotor_num]);
            }
            else
            {
                printf("Fourth rotor type (BETA, GAMMA): ");
            }
            while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
                ;
            err_code = normalize_input(primary_input);
            err_code |= validate_rotor_type(primary_input, rotor_num + 1);
            if (err_code != 0)
            {
                fprintf(stderr, "\nInvalid rotor type: %s\n", primary_input);
                fflush(stderr);
                print_enigma_help();
            }
        } while (err_code != 0);

        do
        {
            printf("%s rotor position (A, B, C, etc): ", numeration[rotor_num]);
            while (my_getline(secondary_input, INPUT_BUFFER_SIZE) == 0)
                ;
            err_code = normalize_input(primary_input);
            err_code |= validate_rotor_positions_and_offsets(secondary_input);
            if (err_code != 0)
            {
                fprintf(stderr, "\nInvalid rotor positions: %s\n", secondary_input);
                fflush(stderr);
                print_enigma_help();
            }
        } while (err_code != 0);

        do
        {
            printf("%s rotor offset / ring setting (A, B, C, etc): ", numeration[rotor_num]);
            while (my_getline(tertiary_input, INPUT_BUFFER_SIZE) == 0)
                ;
            err_code = normalize_input(tertiary_input);
            err_code |= validate_rotor_positions_and_offsets(tertiary_input);
            if (err_code != 0)
            {
                fprintf(stderr, "\nInvalid rotor offset: %s\n", secondary_input);
                fflush(stderr);
                print_enigma_help();
            }
        } while (err_code != 0);

        enigma->rotors[rotor_num] = create_rotor_by_type(primary_input[0] - '0', secondary_input[0] - 'A',
                                                         tertiary_input[0] - 'A');

        puts("");
    }
}

static void parse_enigma_type_interactive(Enigma *enigma)
{
    int32_t err_code = 0;

    do
    {
        char primary_input[INPUT_BUFFER_SIZE];
        printf("Enigma type (M3, M4): ");
        while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code = normalize_input(primary_input);
        err_code |= validate_enigma_type(primary_input);
        enigma->type = parse_enigma_type(primary_input);
        if (err_code != 0)
        {
            fprintf(stderr, "\nInvalid enigma type: %s\n", primary_input);
            fflush(stderr);
            print_enigma_help();
        }
    } while (err_code != 0);
}

static void parse_reflector_type_interactive(Enigma *enigma)
{
    int32_t err_code = 0;
    char primary_input[INPUT_BUFFER_SIZE];

    do
    {
        printf("Reflector type (A, B, C): ");
        while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code = normalize_input(primary_input);
        err_code |= validate_reflector(enigma->type, primary_input);
        if (err_code != 0)
        {
            fprintf(stderr, "\nInvalid reflector: %s\n", primary_input);
            fflush(stderr);
            print_enigma_help();
        }
    } while (err_code != 0);
    enigma->reflector = create_reflector_by_type(*primary_input);
}

static void parse_plugboard_interactive(Enigma *enigma)
{
    int32_t err_code = 0;

    do
    {
        char primary_input[INPUT_BUFFER_SIZE];
        printf("\nPlugboard (e.g. AB CD EF, EMPTY for standard): ");
        if (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
        {
            enigma->plugboard = create_plugboard(NULL);
            continue;
        }
        err_code = normalize_input(primary_input);
        err_code |= validate_plugboard(primary_input);
        enigma->plugboard = create_plugboard(primary_input);
        if (err_code != 0)
        {
            fprintf(stderr, "\nInvalid plugboard: %s\n", primary_input);
            fflush(stderr);
            print_enigma_help();
        }
    } while (err_code != 0);
}

static void parse_plaintext_interactive(Enigma *enigma)
{
    int32_t err_code = 0;

    do
    {
        char primary_input[INPUT_BUFFER_SIZE];
        printf("\nPlaintext: ");
        while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code = normalize_input(primary_input);
        enigma->plaintext = strdup(primary_input);
        assertmsg(enigma->plaintext != NULL, "strdup failed");
        if(err_code != 0)
        {
            fprintf(stderr, "\nCan't normalize plaintext: %s\n", primary_input);
            fflush(stderr);
            print_enigma_help();
        }
    } while (err_code != 0);
}

static Enigma* query_enigma_input_interactive(void)
{
    Enigma *enigma = malloc(sizeof(Enigma));
    assertmsg(enigma != NULL, "malloc failed");

    parse_enigma_type_interactive(enigma);
    parse_enigma_rotors_interactive(enigma);
    parse_reflector_type_interactive(enigma);
    parse_plugboard_interactive(enigma);
    parse_plaintext_interactive(enigma);

    return enigma;
}

static void pretty_print_enigma_output(const uint8_t *text, const size_t plaintext_len)
{
    for (size_t i = 0; i < plaintext_len; i++)
    {
        if (i % 5 == 0 && i != 0) printf(" ");
        printf("%c", text[i] + 'A');
    }
    printf("\n");
}

void run_interactive_enigma_input(void)
{
    Enigma *enigma = query_enigma_input_interactive();

    uint8_t *enigma_output_as_ints = traverse_enigma(enigma);
    pretty_print_enigma_output(enigma_output_as_ints, strlen(enigma->plaintext));

    free_enigma(enigma);
    free(enigma_output_as_ints);
}

static void free_enigma_cli_options(EnigmaCliOptions *options)
{
    free(options->plaintext);
    free(options->plugboard);
}

void query_enigma_input(const int argc, char *argv[])
{
    EnigmaCliOptions enigma_options = {0};

    save_enigma_input(&enigma_options, argc, argv);

    Enigma *enigma = create_enigma_from_cli_configuration(&enigma_options);

    const size_t plaintext_len = strlen(enigma->plaintext);
    uint8_t *text              = traverse_enigma(enigma);

    char *output_str = get_string_from_int_array(text, plaintext_len);

    enigma_cli_options_to_json(&enigma_options, output_str);

    pretty_print_enigma_output(text, plaintext_len);

    free_enigma(enigma);
    free(text);
    free(output_str);
    free_enigma_cli_options(&enigma_options);
}
