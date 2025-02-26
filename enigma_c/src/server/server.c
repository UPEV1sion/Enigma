#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/random.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "server.h"

#include "db_cyclometer.h"
#include "helper/helper.h"
#include "enigma/enigma.h"
#include "json/cJSON.h"
#include "json/json.h"
#include "cyclometer/server_cyclometer.h"

//
// Created by Emanuel on 22.12.2024.
//


#define PORT 8081
#define NUM_CLIENTS 10
#define BUFFER_SIZE 4096
#define MAX_HTTP_HEADER 100
#define MAX_DAILY_KEYS 1024
#define MAX_TOKENS 100

#define ENIGMA_PATH "/enigma"
#define CYCLOMETER_PATH "/cyclometer"
#define CYCLOMETER_OPTION_COUNT "count"

typedef struct
{
    int sock;
    char *URL;
    char *body;
    char *cors;
    char *connection_type;
} HttpPost;

// static char* get_response_json(const char *input)
// {
//     char *response = NULL;
//
//     char *json;
//     if ((json = strstr(input, "{\"model\":")) == NULL)
//     {
//         fprintf(stderr, "Invalid JSON!");
//         return NULL;
//     }
//
//     cJSON *cjson = cJSON_Parse(json);
//     if (cjson == NULL)
//     {
//         fprintf(stderr, "Failed to parse JSON!\n");
//         return NULL;
//     }
//
//     cJSON *old_output = cJSON_GetObjectItem(cjson, "output");
//     if (old_output == NULL)
//     {
//         fprintf(stderr, "Missing \"output\" item!\n");
//         goto CLEANUP_JSON;
//     }
//
//     Enigma *enigma = get_enigma_from_json_string(json);
//     if (enigma == NULL)
//     {
//         fprintf(stderr, "Failed to get enigma from JSON!\n");
//         goto CLEANUP_JSON;
//     }
//
//     uint8_t *text_as_ints = traverse_enigma(enigma);
//     if (text_as_ints == NULL)
//     {
//         fprintf(stderr, "Failed to traverse enigma!\n");
//         goto CLEANUP_ENIGMA;
//     }
//
//     char *text = get_string_from_int_array(text_as_ints, strlen(enigma->plaintext));
//     if (text == NULL)
//     {
//         fprintf(stderr, "Failed to get enigma plaintext!\n");
//         goto CLEANUP_TEXT_AS_INT;
//     }
//
//     cJSON_SetValuestring(old_output, text);
//     response = cJSON_PrintUnformatted(cjson);
//
//     free(text);
// CLEANUP_TEXT_AS_INT:
//     free(text_as_ints);
// CLEANUP_ENIGMA:
//     free_enigma(enigma);
// CLEANUP_JSON:
//     cJSON_Delete(cjson);
//     return response;
// }


static void free_http_post(const HttpPost *post)
{
    if (post->cors != NULL) free(post->cors);
    if (post->body != NULL) free(post->body);
    if (post->URL != NULL) free(post->URL);
    if (post->connection_type != NULL) free(post->connection_type);
}

static int32_t send_http_200_response(const char *body, const HttpPost *post)
{
    int32_t ret = 0;

    char return_buffer[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n";
    const size_t len      = strlen(return_buffer);
    const size_t json_len = strlen(body);

    size_t offset = 0;
    if (post->cors != NULL)
    {
        offset += snprintf(return_buffer + len, BUFFER_SIZE - len, "Access-Control-Allow-Origin: %s\r\n", post->cors);
    }
    offset += snprintf(return_buffer + len + offset, BUFFER_SIZE - len - offset, "Content-Length: %zu\r\n\r\n",
                       json_len);
    strncat(return_buffer + len + offset, body, BUFFER_SIZE - len - offset);

    const size_t response_len    = strlen(return_buffer);
    const ssize_t num_bytes_send = send(post->sock, return_buffer, response_len, 0);
    if (num_bytes_send != (ssize_t) response_len || num_bytes_send == -1) //TODO
    {
        perror("Couldn't send all bytes!");
        ret = -1;
    }

    return ret;
}

char** generate_n_daily_keys(const int32_t n)
{
    char **keys = malloc(sizeof(char *) * n);
    assertmsg(keys != NULL, "malloc failed");

    for (int32_t i = 0; i < n; ++i)
    {
        keys[i] = malloc(DAILY_KEY_SIZE + 1);
        assertmsg(keys[i] != NULL, "malloc failed");
        for (int j = 0; j < DAILY_KEY_SIZE; ++j)
        {
            keys[i][j] = (char) ((random() % 26) + 'A');
        }
        keys[i][DAILY_KEY_SIZE] = 0;
    }

    return keys;
}

void free_keys(char **keys, const int32_t n)
{
    for (int32_t i = 0; i < n; ++i)
    {
        free(keys[i]);
    }
}

static void free_enigma_config(EnigmaConfiguration *conf)
{
    free(conf->ring_settings);
    free(conf->rotor_positions);
    free(conf->rotors);
    free(conf);
}

static cJSON* create_cycle_json(const S_Cycle *cycles)
{
    cJSON *json    = cJSON_CreateObject();
    char *names[3] = {"first", "second", "third"};
    for (int32_t i = 0; i < 3; ++i)
    {
        char rotor_name[30];
        sprintf(rotor_name, "%s_cycle", names[i]);
        cJSON *rotor               = cJSON_AddArrayToObject(json, rotor_name);
        const S_Cycle *current_cycle = cycles + i;
        for (int j = 0; j < current_cycle->length; ++j)
        {
            cJSON_AddItemToArray(rotor, cJSON_CreateNumber(current_cycle->cycle_values[j]));
        }
    }

    return json;
}

static void tokenize_string(char *tokens[MAX_TOKENS], char *str)
{
    char *save_ptr;
    char *tok = strtok_r(str, " ", &save_ptr);
    for (int i = 0; i < MAX_TOKENS && tok; ++i)
    {
        tokens[i] = tok;
        tok       = strtok_r(NULL, " ", &save_ptr);
    }
}

static int save_token(char **dest, const char *origin)
{
    if (origin == NULL) return -1;
    if (dest == NULL) return -2;
    *dest = strdup(origin);
    assertmsg(dest != NULL, "strdup failed");

    return 0;
}

static int parse_post_headers(HttpPost *headers, char *header_str)
{
    char *save_ptr;
    const char *line = strtok_r(header_str, "\r\n", &save_ptr);
    if (!line) return -1;

    char *first_tokens[MAX_TOKENS] = {NULL};
    char *line_copy                = strdup(line);
    tokenize_string(first_tokens, line_copy);

    if (strncmp(first_tokens[0], "POST", 4) != 0) return -2;
    if (save_token(&headers->URL, first_tokens[1]) != 0) return -3;

    free(line_copy);

    while ((line = strtok_r(NULL, "\r\n", &save_ptr)))
    {
        char *tokens[MAX_TOKENS] = {NULL};
        line_copy                = strdup(line);
        tokenize_string(tokens, line_copy);
        if (strncmp(tokens[0], "Origin", 6) == 0)
        {
            save_token(&headers->cors, tokens[1]);
        }
        else if (strncmp(tokens[0], "Connection", 10) == 0)
        {
            save_token(&headers->connection_type, tokens[1]);
        }
        //TODO more headers?

        free(line_copy);
    }

    return 0;
}

static int send_enigma_response(const HttpPost *post)
{
    int retval = 0;

    cJSON *json = cJSON_Parse(post->body); //TODO maybe more sophisticated
    if (json == NULL)
    {
        retval =  -1;
        goto FAIL1;
    }

    const cJSON *enigma_json = cJSON_GetObjectItem(json, "enigma");
    if (enigma_json == NULL)
    {
        retval =  -2;
        goto FAIL2;
    }

    Enigma *enigma = get_enigma_from_json(enigma_json);
    if (enigma == NULL)
    {
        retval =  -3; //TODO error message
        goto FAIL2;
    }

    uint8_t *text_as_int = traverse_enigma(enigma); //TODO remove assert
    char *output         = get_string_from_int_array(text_as_int, strlen(enigma->plaintext));
    if (output == NULL)
    {
        retval = -4;
        goto FAIL3;
    }

    cJSON *old_output = cJSON_GetObjectItem(enigma_json, "output");
    if (old_output == NULL)
    {
        retval = -5;
        goto FAIL4;
    }

    cJSON_SetValuestring(old_output, output);

    char *json_str = cJSON_PrintUnformatted(json);

    send_http_200_response(json_str, post);

    cJSON_free(json_str);
    FAIL4:
        free(output);
    FAIL3:
        free_enigma(enigma);
        free(text_as_int);
    FAIL2:
        cJSON_Delete(json);
    FAIL1:
        return retval;
}

static int send_cyclometer_response(const HttpPost *post)
{
    ServerCyclometerOptions opt = { 0 };
    if (get_server_cyclometer_options_from_json(&opt, post->body) != 0)
    {
        //TODO
        return -1;
    }
    if (opt.daily_key_count > MAX_DAILY_KEYS) opt.daily_key_count = MAX_DAILY_KEYS;
    char **keys = generate_n_daily_keys(opt.daily_key_count);
    free(opt.enigma_conf->message);
    char *enc_keys[MAX_DAILY_KEYS];

    for (int32_t i = 0; i < opt.daily_key_count; ++i)
    {
        char current_key[DAILY_KEY_SIZE * 2 + 1] = {0};
        memcpy(current_key, keys[i], DAILY_KEY_SIZE);
        memcpy(current_key + DAILY_KEY_SIZE, keys[i], DAILY_KEY_SIZE);
        opt.enigma_conf->message = current_key;
        Enigma *enigma           = create_enigma_from_configuration(opt.enigma_conf);
        uint8_t *enc_key_as_ints = traverse_enigma(enigma);
        char *enc_key            = get_string_from_int_array(enc_key_as_ints, DAILY_KEY_SIZE * 2);
        // printf("%s -> %s\n", current_key, enc_key);
        enc_keys[i] = enc_key;

        free(enc_key_as_ints);
        free_enigma(enigma);
    }

    S_Cycle *cycles        = server_create_cycles(enc_keys, opt.daily_key_count);
    cJSON *cycle_json    = create_cycle_json(cycles);
    char *cycle_json_str = cJSON_PrintUnformatted(cycle_json);

    send_http_200_response(cycle_json_str, post);
    query_db(cycles);

    cJSON_Delete(cycle_json);
    cJSON_free(cycle_json_str);
    free(cycles);
    free_keys(enc_keys, opt.daily_key_count);
    free_enigma_config(opt.enigma_conf);
    free_keys(keys, opt.daily_key_count);
    free(keys);

    return 0;
}

static int send_response(const HttpPost *post)
{
    if (strncasecmp(post->URL, ENIGMA_PATH, sizeof (ENIGMA_PATH) - 1) == 0)
    {
        return send_enigma_response(post);
    }
    if (strncasecmp(post->URL, CYCLOMETER_PATH, sizeof(CYCLOMETER_PATH) - 1) == 0)
    {
        return send_cyclometer_response(post);
    }

    return 1;
}

static void parse_http_post(HttpPost *post, const char *post_str)
{
    const char *header_end = strstr(post_str, "\r\n\r\n");
    char *header_str       = strndup(post_str, header_end - post_str);
    parse_post_headers(post, header_str);
    free(header_str);
    post->body = strdup(header_end + 4);
}

static int handle_response(const char *buffer, const int sock)
{
    HttpPost post = {.sock = sock};
    parse_http_post(&post, buffer);

    const int retval =  send_response(&post);
    free_http_post(&post);

    return retval;
}

static void* handle_client(void *arg)
{
    const int sock = *(int *) arg;
    free(arg);

    char buffer[BUFFER_SIZE] = {0};
    if (recv(sock, buffer, BUFFER_SIZE, 0) == -1)
    {
        perror("Couldn't read from socket");
        close(sock);

        return NULL;
    }
    handle_response(buffer, sock);

    close(sock);

    return NULL;
}

static int32_t accept_incoming(const int sock)
{
    for (;;)
    {
        int *new_sock = malloc(sizeof(int));
        if (new_sock == NULL)
        {
            perror("Couldn't allocate sock");
            continue;
        };

        if ((*new_sock = accept(sock, NULL, NULL)) == -1)
        {
            perror("Error accepting connection");
            free(new_sock);
            continue;
        }
        printf("%d\n", *new_sock);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_sock) != 0)
        {
            perror("Error creating thread");
            close(*new_sock);
            free(new_sock);
            continue;
        }

        pthread_detach(thread_id);
    }

    return 0;
}

int32_t server_run(void)
{
    init_db();
    int sock;
    assertmsg((sock = socket(AF_INET, SOCK_STREAM, 0)) != -1, "Couldn't create socket");
    assertmsg(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof (int)) != -1, "Couldn't set socket options");
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };
    assertmsg(bind(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) != -1, "Couldn't bind");
    assertmsg(listen(sock, 5) != -1, "Couldn't listen");

    return accept_incoming(sock);
}
