#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include "cli_enigma.h"
#include "helper/helper.h"
#include "enigma/reflector/reflector.h"
#include "enigma/rotor/rotor.h"
#include "enigma/enigma.h"
#include "enigma/plugboard/plugboard.h"

//
// Created by Emanuel on 05.10.2024.
//

#define INPUT_BUFFER_SIZE 1024

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
    uint8_t interactive: 1;
} EnigmaCliOptions;

void print_enigma_help(void)
{
    puts("\n\n");
    puts("Usage: enigma [OPTIONS]...");
    puts("Encrypts/decrypts text using the Enigma machine\n");
    puts("Enigma options:");
    printf("%6s, %-20s | %-40s\n", "-short", "--long", "description");
    for (uint16_t i = 0; i < 50; ++i) printf("%c", '=');
    printf("\n%6s, %-20s | %-40s\n", ENIGMA_SHORT, ENIGMA, "Enigma type (M3, M4)");
    printf("%6s, %-20s | %-40s\n", ROTOR_ONE_SHORT, ROTOR_ONE, "first rotor type (1 - 8)");
    printf("%6s, %-20s | %-40s\n", ROTOR_TWO_SHORT, ROTOR_TWO, "second rotor type (1 - 8)");
    printf("%6s, %-20s | %-40s\n", ROTOR_THREE_SHORT, ROTOR_THREE, "third rotor type (1 - 8)");
    printf("%6s, %-20s | %-40s\n", ROTOR_FOUR_SHORT, ROTOR_FOUR, "fourth rotor type (beta, gamma) - Enigma M4 only");
    printf("%6s, %-20s | %-40s\n", ROTOR_OFFSETS_SHORT, ROTOR_OFFSETS, "rotor offset / ring setting (ABC, AABC, etc)");
    printf("%6s, %-20s | %-40s\n", ROTOR_POSITIONS_SHORT, ROTOR_POSITIONS, "rotor position (ABC, AABC, etc)");
    printf("%6s, %-20s | %-40s\n", REFLECTOR_SHORT, REFLECTOR, "reflector type (A, B, C)");
    printf("%6s, %-20s | %-40s\n", PLUGBOARD_SHORT, PLUGBOARD, "plugboard (e.g. AB CD EF)");
    printf("%6s, %-20s | %-40s\n\n", PLAINTEXT_SHORT, PLAINTEXT, "plaintext (A secret Text)");
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
    assertmsg(new_str != NULL, "realloc failed");

    *dest = new_str;
    strcpy(*dest + dest_len, src);
}

static bool only_contains_upper_alpha(const char *restrict str)
{
    const size_t len = strlen(str);
    for (size_t i = 0; i < len; ++i)
    {
        if(!isupper(str[i])) return false;
    }

    return true;
}

static void parse_rotor_input(EnigmaCliOptions *options, char *argv[], int32_t *i, const enum ROTOR_TYPE rotor_num)
{
    int32_t rotor_type = 0;
    assertmsg(get_number_from_string(argv[++*i], &rotor_type) == 0, "Number parsing failed");

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

static void save_enigma_input(EnigmaCliOptions *options, const int argc, char *argv[])
{
    int32_t i = 1;
    while (i < argc)
    {
        if (string_equals(ENIGMA, argv[i]) ||
            string_equals(ENIGMA_SHORT, argv[i]))
        {
            options->enigma_type = string_equals(argv[++i], "M3") ? ENIGMA_M3 : ENIGMA_M4;
        }
        else if(string_equals(INTERACTIVE, argv[i]) ||
            string_equals(INTERACTIVE_SHORT, argv[i]))
        {
            options->interactive = 1;
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
            char *rotor_four_type = argv[++i];

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
                exit(1);
            }
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
            char *reflector_type = argv[++i];

            assertmsg(to_uppercase(reflector_type) == 0, "Input normalization failed");

            if (string_equals(reflector_type, "B_THIN"))
            {
                options->reflector_type = UKW_B_THIN;
            }
            else if (string_equals(reflector_type, "C_THIN"))
            {
                options->reflector_type = UKW_C_THIN;
            }
            else
            {
                options->reflector_type = *reflector_type;
            }
        }
        else if (string_equals(PLUGBOARD, argv[i]) ||
                 string_equals(PLUGBOARD_SHORT, argv[i]))
        {
            options->plugboard = strdup(argv[++i]);
            assertmsg(options->plugboard != NULL, "strdup failed");

            while (i + 1 < argc && argv[i + 1][0] != '-')
            {
                append_to_string(&options->plugboard, argv[++i]);
            }
        }
        else if (string_equals(PLAINTEXT, argv[i]) ||
                 string_equals(PLAINTEXT_SHORT, argv[i]))
        {
            options->plaintext = strdup(argv[++i]);
            assertmsg(options->plaintext != NULL, "strdup failed");

            while (i + 1 < argc && argv[i + 1][0] != '-')
            {
                append_to_string(&options->plaintext, argv[++i]);
            }
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
    if (options->enigma_type != ENIGMA_M3 && options->enigma_type != ENIGMA_M4) return 1;
    if (options->plaintext == NULL) return 1;
    if (options->rotor_offsets == NULL) return 1;
	if (!only_contains_upper_alpha(options->rotor_offsets)) return 1;
    if (options->rotor_positions == NULL) return 1;
    if (!only_contains_upper_alpha(options->rotor_positions)) return 1;

    // This will fail in the create_reflector_by_type switch case and give a clearer error
    // if (options->reflector_type < UKW_A || options->reflector_type > UKW_C) return 1;

    if (strlen(options->rotor_offsets) != options->enigma_type) return 1;
    if (strlen(options->rotor_positions) != options->enigma_type) return 1;
    if (options->rotor_one_type < ROTOR_1 || options->rotor_one_type > ROTOR_8) return 1;
    if (options->rotor_two_type < ROTOR_1 || options->rotor_two_type > ROTOR_8) return 1;
    if (options->rotor_three_type < ROTOR_1 || options->rotor_three_type > ROTOR_8) return 1;

    if (options->enigma_type == ENIGMA_M3 && options->rotor_four_type != 0)
        return 1;
    if (options->enigma_type == ENIGMA_M4)
    {
        if (options->rotor_four_type != ROTOR_BETA && options->rotor_four_type != ROTOR_GAMMA)
            return 1;
        if (options->reflector_type != UKW_B_THIN && options->reflector_type != UKW_C_THIN)
            return 1;
    }

    int32_t rotor_num[4];
    rotor_num[0] = options->rotor_one_type;
    rotor_num[1] = options->rotor_two_type;
    rotor_num[2] = options->rotor_three_type;
    if (options->enigma_type == ENIGMA_M4) rotor_num[3] = options->rotor_four_type;


    bool rotor_seen[12] = {false};
    for (uint16_t i = 0; i < options->enigma_type; ++i)
    {
        const uint8_t rotor_index = rotor_num[i] - 1;
        if (rotor_seen[rotor_index]) return 1;
        rotor_seen[rotor_index] = true;
    }
    if (options->plugboard != NULL && strlen(options->plugboard) > 0 &&  has_duplicates(options->plugboard))
        return 1;
    return 0;
}


static void normalize_cli_options(const EnigmaCliOptions *options)
{
    int32_t err_code = 0;
    err_code |= to_uppercase(options->rotor_offsets);
    err_code |=remove_non_alnum(options->rotor_offsets);
    assertmsg(err_code == 0, "Rotor offsets normalization failed");

    err_code |= to_uppercase(options->rotor_positions);
    err_code |= remove_non_alnum(options->rotor_positions);
    assertmsg(err_code == 0, "Rotor positions normalization failed");

    to_uppercase(options->plugboard);
    remove_non_alnum(options->plugboard);

    err_code |= to_uppercase(options->plaintext);
    err_code |= remove_non_alnum(options->plaintext);
    assertmsg(err_code == 0, "Plaintext normalization failed");
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
    fprintf(stderr, "Invalid enigma type: %s\n", enigma_type);
    exit(1);
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

    for (uint8_t i = 0; i < enigma->type; ++i)
    {
        enigma->rotors[i] = create_rotor_by_type(rotor_types[i],
                                                 options->rotor_positions[i] - 'A',
                                                 options->rotor_offsets[i] - 'A');
    }

    enigma->reflector = create_reflector_by_type(options->reflector_type);
    enigma->plugboard = create_plugboard(options->plugboard);

    enigma->plaintext = strdup(options->plaintext);
    assertmsg(enigma->plaintext != NULL, "strdup failed");
    free(options->plaintext);
    free(options->plugboard);
    return enigma;
}

Enigma* query_enigma_input_interactive(void)
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
    enigma->type   = parse_enigma_type(primary_input);
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
        err_code |= remove_non_alnum(primary_input);
        assertmsg(err_code == 0, "normalization failed");
        printf("%s rotor position (A, B, C, etc): ", numeration[rotor_num]);
        while (my_getline(secondary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code |= to_uppercase(secondary_input);
        err_code |= remove_non_alnum(secondary_input);
        assertmsg(err_code == 0, "normalization failed");
        printf("%s rotor offset / ring setting (A, B, C, etc): ", numeration[rotor_num]);
        while (my_getline(tertiary_input, INPUT_BUFFER_SIZE) == 0)
            ;
        err_code |= to_uppercase(tertiary_input);
        err_code |= remove_non_alnum(tertiary_input);
        assertmsg(err_code == 0, "normalization failed");
        enigma->rotors[rotor_num] = create_rotor_by_type(primary_input[0] - '0', secondary_input[0] - 'A',
                                                         tertiary_input[0] - 'A');

        puts("");
    }

    printf("Reflector type (A, B, C): ");
    while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    err_code |= to_uppercase(primary_input);
    err_code |= remove_non_alnum(primary_input);
    assertmsg(err_code == 0, "normalization failed");
    enigma->reflector = create_reflector_by_type(*primary_input);

    printf("\nPlugboard (e.g. AB CD EF, EMPTY for standard): ");
    if (my_getline(primary_input, INPUT_BUFFER_SIZE) > 0)
    {
        err_code |= to_uppercase(primary_input);
        err_code |= remove_non_alnum(primary_input);
        assertmsg(err_code == 0, "normalization failed");
        enigma->plugboard = create_plugboard(primary_input);
    }
    enigma->plugboard = create_plugboard(NULL);

    printf("\nPlaintext: ");
    while (my_getline(primary_input, INPUT_BUFFER_SIZE) == 0)
        ;
    enigma->plaintext = strdup(primary_input);
    assertmsg(enigma->plaintext != NULL, "strdup failed");
    err_code |= to_uppercase(enigma->plaintext);
    err_code |= remove_non_alnum(enigma->plaintext);
    assertmsg(err_code == 0, "normalization failed");

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

void query_enigma_input(const int argc, char *argv[])
{

    EnigmaCliOptions enigma_options = {0};

    save_enigma_input(&enigma_options, argc, argv);

    Enigma *enigma;

    if (enigma_options.interactive)
    {
        enigma = query_enigma_input_interactive();
    }
    else
    {
        enigma = create_enigma_from_cli_configuration(&enigma_options);
    }

    const size_t plaintext_len = strlen(enigma->plaintext);
    uint8_t *text              = traverse_enigma(enigma);

    // char *output_str = get_string_from_int_array(text, plaintext_len);

    // enigma_to_json(output_str);

    pretty_print_enigma_output(text, plaintext_len);

    free_enigma(enigma);
    free(text);

}