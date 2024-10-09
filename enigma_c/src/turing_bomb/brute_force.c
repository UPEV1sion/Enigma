#include <stdint.h>
#include <string.h>

#include "brute_force.h"
#include "helper/helper.h"
#include "gui/bomb_out_gui//bomb_out.h"
#include "enigma/enigma.h"

//
// Created by Emanuel on 29.07.2024.
//

/**
* @brief Platform independent implementation of an omp parallel for loop,
* as linux requires stderr as a shared resource. TODO MAC?
*/
#ifdef _OPENMP
#if defined(_WIN32) || defined(_WIN64)
#define OMP_PARALLEL_PRAGMA \
    _Pragma("omp parallel for collapse(6) default (none) shared(known_plaintext, file, conf)")
#elif defined(__unix__) || defined(__unix) || defined(__linux__)
#define OMP_PARALLEL_PRAGMA \
    _Pragma("omp parallel for collapse(6) default (none) shared(known_plaintext, file, conf, stderr)")
#endif
#endif


#define OUTPUT_STRING_SIZE 64
#define NUM_ROTATORS 5
#define NUM_ROTORS_PER_ENIGMA 3

typedef struct Node Node;

static Node *top          = NULL;
static int64_t stack_size = 0;

//Priority Queue for best rotor order
struct Node
{
    double ic;
    Node *next;
    EnigmaConfiguration *conf;
};

static Node* new_node(EnigmaConfiguration *config, const double ic)
{
    Node *temp = malloc(sizeof(Node));
    assertmsg(temp != NULL, "temp == NULL");
    temp->conf = config;
    temp->next = NULL;
    temp->ic   = ic;
    return temp;
}

static void place_node_in_pq(Node *node)
{
    if (top == NULL || node->ic > top->ic)
    {
        node->next = top;
        top        = node;
        return;
    }
    Node *cur;
    for (cur = top; cur->next != NULL && cur->next->ic >= node->ic; cur = cur->next)
        ;
    node->next = cur->next;
    cur->next  = node;

    stack_size++;
}

static void push(EnigmaConfiguration *conf, const double ic)
{
    Node *temp = new_node(conf, ic);

    #ifdef _OPENMP
    #pragma omp critical
    #endif
    {
        place_node_in_pq(temp);
    }
}

static EnigmaConfiguration* pop(void)
{
    EnigmaConfiguration *conf;
    if (stack_size == 0) return NULL;

    #ifdef _OPENMP
    #pragma omp critical
    #endif
    {
        Node *temp = top;
        stack_size--;
        printf("IC %f\n", temp->ic);
        conf = temp->conf;
        top  = top->next;
        free(temp);
    }

    return conf;
}

static void free_config(EnigmaConfiguration *conf)
{
    if (conf == NULL) return;
    free(conf->rotors);
    free(conf->rotor_positions);
    free(conf->ring_settings);
    free(conf);
}

// static void pop_and_print_whole_stack(void)
// {
//     for (EnigmaConfiguration *conf_temp = pop(); conf_temp != NULL; conf_temp = pop())
//     {
//         for (uint16_t i = 0; i < NUM_ROTORS_PER_ENIGMA; i++)
//         {
//             printf("%d :", conf_temp->rotors[i]);
//             printf(" %d :", conf_temp->rotor_positions[i] + 1);
//             printf(" %d\n", conf_temp->ring_settings[i] + 1);
//         }
//         puts(conf_temp->message);
//         puts("");
//         free_config(conf_temp);
//     }
// }

static void advance_enigma_back(const EnigmaConfiguration *conf, const Enigma *enigma)
{
    const size_t len_text = strlen(conf->message);
    Rotor *rotorOne       = enigma->rotors[0];
    Rotor *rotorTwo       = enigma->rotors[1];
    Rotor *rotorThree     = enigma->rotors[2];

    printf("%d\n", conf->rotor_positions[0]);
    printf("%d\n", conf->rotor_positions[1]);
    printf("%d\n", conf->rotor_positions[2]);

    for (size_t i = 0; i < len_text; ++i)
    {
        rotorOne->position = (26 + rotorOne->position - 1) % 26;

        if (should_rotate(rotorOne))
        {
            rotorTwo->position = (26 + rotorTwo->position - 1) % 26;

            if (should_rotate(rotorTwo))
            {
                rotorThree->position = (26 + rotorThree->position - 1) % 26;
            }
        }
    }
    conf->rotor_positions[0] = enigma->rotors[0]->position;
    conf->rotor_positions[1] = enigma->rotors[1]->position;
    conf->rotor_positions[2] = enigma->rotors[2]->position;
}


static uint8_t* traverse_enigma_w_checks(register const char *known_plaintext,
                                         register Enigma *enigma,
                                         register char *start_enc)
{
    uint8_t *text           = get_int_array_from_string(enigma->plaintext);
    assertmsg(text != NULL, "string to int[] conversion failed");

    const size_t array_size = strlen(known_plaintext);
    uint8_t *output         = malloc(array_size * sizeof(uint8_t));
    assertmsg(output != NULL, "malloc failed");

    Rotor *rotorOne         = enigma->rotors[0];
    Rotor *rotorTwo         = enigma->rotors[1];
    Rotor *rotorThree       = enigma->rotors[2];

    const Plugboard *plugboard = enigma->plugboard;
    assertmsg(plugboard != NULL, "plugboard == NULL");
    const Reflector *reflector = enigma->reflector;
    for (size_t i = 0; i < array_size; ++i)
    {
        rotorOne->position = (rotorOne->position + 1) % 26;

        if (should_rotate(rotorOne))
        {
            rotorTwo->position = (rotorTwo->position + 1) % 26;

            if (should_rotate(rotorTwo))
            {
                rotorThree->position = (rotorThree->position + 1) % 26;
            }
        }

        uint8_t character      = traverse_rotor(rotorOne, text[i]);
        character              = traverse_rotor(rotorTwo, character);
        character              = traverse_rotor(rotorThree, character);
        character              = reflector->wiring[character];
        character              = traverse_rotor_inverse(rotorThree, character);
        character              = traverse_rotor_inverse(rotorTwo, character);
        character              = traverse_rotor_inverse(rotorOne, character);
        character              = plugboard->plugboard_data[character];
        const uint8_t testchar = *known_plaintext++ - 'A';
        if ((output[i] = character) != testchar)
        {
            free(text);
            free(output);
            enigma->plaintext = start_enc;
            free_enigma(enigma);
            return NULL;
        }
    }
    free(text);
    return output;
}

static double check_enigma_rotors(register const EnigmaConfiguration *conf)
{
    const size_t array_size = strlen(conf->message);
    uint8_t *text           = get_int_array_from_string(conf->message);
    assertmsg(text != NULL, "string to int[] conversion failed");

    uint8_t *output         = malloc(array_size * sizeof(uint8_t));
    assertmsg(output != NULL, "malloc failed");

    Enigma *enigma = create_enigma_from_configuration(conf);
    assertmsg(enigma != NULL, "enigma == NULL");

    Rotor *rotorOne            = enigma->rotors[0];
    Rotor *rotorTwo            = enigma->rotors[1];
    Rotor *rotorThree          = enigma->rotors[2];
    const Reflector *reflector = enigma->reflector;
    const Plugboard *plugboard = enigma->plugboard;

    for (size_t i = 0; i < array_size; ++i)
    {
        rotorOne->position = (rotorOne->position + 1) % 26;
        uint8_t character  = traverse_rotor(rotorOne, text[i]);
        character          = traverse_rotor(rotorTwo, character);
        character          = traverse_rotor(rotorThree, character);
        character          = reflector->wiring[character];
        character          = traverse_rotor_inverse(rotorThree, character);
        character          = traverse_rotor_inverse(rotorTwo, character);
        character          = traverse_rotor_inverse(rotorOne, character);
        output[i]          = plugboard->plugboard_data[character];
    }
    free(text);
    free_enigma(enigma);
    const double ic = calc_index_of_coincidence(output, array_size);
    assertmsg(ic != NaN, "calc_index_of_coincidence failed");
    free(output);
    return ic;
}

static char* create_rotor_string(const EnigmaConfiguration *conf)
{
    char *rotor_string = malloc(OUTPUT_STRING_SIZE);
    assertmsg(rotor_string != NULL, "rotor_string == NULL");
    const int ret = snprintf(rotor_string, OUTPUT_STRING_SIZE, "%d : %d : %d", conf->rotors[0], conf->rotors[1], conf->rotors[2]);
    assertmsg(ret >= 0 && ret < OUTPUT_STRING_SIZE, "snprintf failed");
    return rotor_string;
}

static char* create_positions_string(const EnigmaConfiguration *conf)
{
    char *position_string = malloc(OUTPUT_STRING_SIZE);
    assertmsg(position_string != NULL, "position_string == NULL");
    const int ret = snprintf(position_string, OUTPUT_STRING_SIZE, "%d : %d : %d",
        conf->rotor_positions[0] + 1,
        conf->rotor_positions[1] + 1,
        conf->rotor_positions[2] + 1);
    assertmsg(ret >= 0 && ret < OUTPUT_STRING_SIZE, "snprintf failed");
    return position_string;
}

static char* create_ring_string(const EnigmaConfiguration *conf)
{
    char *ring_string = malloc(OUTPUT_STRING_SIZE);
    assertmsg(ring_string != NULL, "ring_string == NULL");
    const int ret = snprintf(ring_string, OUTPUT_STRING_SIZE, "%d : %d : %d",
        conf->ring_settings[0] + 1,
        conf->ring_settings[1] + 1,
        conf->ring_settings[2] + 1);
    assertmsg(ret >= 0 && ret < OUTPUT_STRING_SIZE, "snprintf failed");
    return ring_string;
}

static char* create_plugboard_string(const EnigmaConfiguration *conf)
{
    char *plugboard_string = malloc(ALPHABET_SIZE + 1);
    assertmsg(plugboard_string != NULL, "plugboard_string == NULL");
    strncpy(plugboard_string, conf->plugboard, ALPHABET_SIZE);
    plugboard_string[ALPHABET_SIZE] = 0;
    return plugboard_string;
}

static void print_conf(const char *rotor_string, const char *position_string, const char *ring_string,
                       const char *plugboard_string, FILE *file)
{
    //fwrite is preferable to fprintf or fputs because it yields faster results
    fwrite(rotor_string, sizeof(char), strlen(rotor_string), file);
    fwrite(" / ", sizeof(char), 3, file);
    fwrite(position_string, sizeof(char), strlen(position_string), file);
    fwrite(" / ", sizeof(char), 3, file);
    fwrite(ring_string, sizeof(char), strlen(ring_string), file);
    fwrite(" / ", sizeof(char), 3, file);
    fwrite(plugboard_string, sizeof(char), strlen(plugboard_string), file);
    fwrite("\n", sizeof(char), 1, file);
}

static void test_config(register const char *known_plaintext,
                        register const EnigmaConfiguration *conf,
                        FILE *file)
{
    Enigma *temp = create_enigma_from_configuration(conf);
    assertmsg(temp != NULL, "enigma == NULL");
    //Testing only the last characters as there were always known
    char *start_enc        = temp->plaintext;
    const size_t len_enc   = strlen(temp->plaintext);
    const size_t len_known = strlen(known_plaintext);

    temp->plaintext = temp->plaintext + (len_enc - len_known);

    uint8_t *candidate = traverse_enigma_w_checks(known_plaintext, temp, start_enc);
    if (candidate == NULL)
    {
        free(candidate);
        return;
    }

    char *message = get_string_from_int_array(candidate, strlen(temp->plaintext));
    assertmsg(message != NULL, "int[] to string conversion failed");
    free(candidate);
    assertmsg(message != NULL, "failed to get string from array");
    //Branching hint for pipelining
    if (expected_false(strcmp(known_plaintext, message) == 0))
    {
        static bool bomb_out_launched = false;
        puts("Valid");
        #ifdef _OPENMP
        #pragma omp critical
        #endif
        {
            advance_enigma_back(conf, temp);
            char *rotor_string     = create_rotor_string(conf);
            char *position_string  = create_positions_string(conf);
            char *ring_string      = create_ring_string(conf);
            char *plugboard_string = create_plugboard_string(conf);
            print_conf(rotor_string, position_string, ring_string, plugboard_string, file);
            free(rotor_string);
            free(position_string);
            free(ring_string);
            free(plugboard_string);
            if (!bomb_out_launched)
            {
                bomb_out_launched = true;
            }
        }
    }
    temp->plaintext = start_enc;
    free(message);
    free_enigma(temp);
}

static void test_configs_sorted(const char *known_plaintext)
{
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_CONFIGURATIONS, "w")) != NULL, "file == NULL");
    EnigmaConfiguration *conf = pop();

    for (uint16_t i = 0; i < 1 && conf != NULL; ++i)
    {
        printf("%d %d %d\n", conf->rotors[0], conf->rotors[1], conf->rotors[2]);
        #ifdef _OPENMP
        OMP_PARALLEL_PRAGMA
        #endif
        for (uint8_t rotor1_pos = 0; rotor1_pos < ALPHABET_SIZE; ++rotor1_pos)
        {
            for (uint8_t rotor2_pos = 0; rotor2_pos < ALPHABET_SIZE; ++rotor2_pos)
            {
                for (uint8_t rotor3_pos = 0; rotor3_pos < ALPHABET_SIZE; ++rotor3_pos)
                {
                    for (uint8_t ring1_pos = 0; ring1_pos < ALPHABET_SIZE; ++ring1_pos)
                    {
                        for (uint8_t ring2_pos = 0; ring2_pos < ALPHABET_SIZE; ++ring2_pos)
                        {
                            for (uint8_t ring3_pos = 0; ring3_pos < ALPHABET_SIZE; ++ring3_pos)
                            {
                                EnigmaConfiguration conf_copy;
                                // = malloc(sizeof(EnigmaConfiguration));
                                // assertmsg(conf_copy != NULL, "conf_copy == NULL");

                                conf_copy                 = *conf;
                                conf_copy.rotors          = malloc(NUM_ROTORS_PER_ENIGMA * sizeof(enum ROTOR_TYPE));
                                conf_copy.rotor_positions = malloc(NUM_ROTORS_PER_ENIGMA * sizeof(uint8_t));
                                conf_copy.ring_settings   = malloc(NUM_ROTORS_PER_ENIGMA * sizeof(uint8_t));
                                assertmsg(conf_copy.rotors != NULL
                                          && conf_copy.rotor_positions != NULL
                                          && conf_copy.ring_settings != NULL,
                                          "malloc failed");

                                memcpy(conf_copy.rotors, conf->rotors, NUM_ROTORS_PER_ENIGMA * sizeof(uint8_t));
                                memcpy(conf_copy.rotor_positions, conf->rotor_positions,
                                       NUM_ROTORS_PER_ENIGMA * sizeof(uint8_t));
                                memcpy(conf_copy.ring_settings, conf->ring_settings,
                                       NUM_ROTORS_PER_ENIGMA * sizeof(uint8_t));
                                conf_copy.message = conf->message;

                                conf_copy.rotor_positions[0] = rotor1_pos;
                                conf_copy.rotor_positions[1] = rotor2_pos;
                                conf_copy.rotor_positions[2] = rotor3_pos;
                                conf_copy.ring_settings[0]   = ring1_pos;
                                conf_copy.ring_settings[1]   = ring2_pos;
                                conf_copy.ring_settings[2]   = ring3_pos;

                                test_config(known_plaintext, &conf_copy, file);

                                // free_config(conf_copy);
                            }
                        }
                    }
                }
            }
        }
        free_config(conf);
        conf = pop();
    }
    free_config(conf);
    fclose(file);
}

void crack_enigma(const char *known_plaintext, char *encrypted_text)
{
    int32_t combinations = 0;
    FILE *file;
    assertmsg((file = fopen(FILE_PATH_IC, "w")) != NULL, "file == NULL");

    for (uint8_t rotor_one = 1; rotor_one <= NUM_ROTATORS; rotor_one++)
    {
        for (uint8_t rotor_two = 1; rotor_two <= NUM_ROTATORS; rotor_two++)
        {
            if (rotor_one == rotor_two)
            {
                continue;
            }
            for (uint8_t rotor_three = 1; rotor_three <= NUM_ROTATORS; rotor_three++)
            {
                if (rotor_one == rotor_three || rotor_two == rotor_three)
                {
                    continue;
                }

                combinations++;
                EnigmaConfiguration *conf =
                        malloc(sizeof(EnigmaConfiguration));
                assertmsg(conf != NULL, "conf == NULL");

                conf->type            = ENIGMA_M3;
                conf->reflector       = 'B';
                conf->rotors          = malloc(NUM_ROTORS_PER_ENIGMA * sizeof(enum ROTOR_TYPE));
                conf->rotor_positions = calloc(NUM_ROTORS_PER_ENIGMA, sizeof(uint8_t));
                conf->ring_settings   = calloc(NUM_ROTORS_PER_ENIGMA, sizeof(uint8_t));
                assertmsg(conf->rotors != NULL
                          && conf->rotor_positions != NULL
                          && conf->ring_settings != NULL,
                          "malloc failed");

                conf->rotors[0] = rotor_one;
                conf->rotors[1] = rotor_two;
                conf->rotors[2] = rotor_three;
                conf->message   = encrypted_text;
                for (uint8_t i = 0; i < ALPHABET_SIZE; ++i)
                {
                    conf->plugboard[i] = (char) i;
                }
                const double ic = check_enigma_rotors(conf);
                push(conf, ic);
            }
        }
    }
    fclose(file);
    printf("%d\n", combinations);
    puts(encrypted_text);
    test_configs_sorted(known_plaintext);
    //    pop_and_print_whole_stack();
}
