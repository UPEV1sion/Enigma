#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/random.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "server.h"
#include "helper/helper.h"
#include "picohttpparser.h"
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

#define ENIGMA_PATH "/enigma"
#define CYCLOMETER_PATH "/cyclometer"
#define CYCLOMETER_OPTION_COUNT "count"

static char* get_response_json(const char *input)
{
    char *response = NULL;

    char *json;
    if ((json = strstr(input, "{\"model\":")) == NULL)
    {
        fprintf(stderr, "Invalid JSON!");
        return NULL;
    }

    cJSON *cjson = cJSON_Parse(json);
    if (cjson == NULL)
    {
        fprintf(stderr, "Failed to parse JSON!\n");
        return NULL;
    }

    cJSON *old_output = cJSON_GetObjectItem(cjson, "output");
    if (old_output == NULL)
    {
        fprintf(stderr, "Missing \"output\" item!\n");
        goto CLEANUP_JSON;
    }

    Enigma *enigma = get_enigma_from_json(json);
    if (enigma == NULL)
    {
        fprintf(stderr, "Failed to get enigma from JSON!\n");
        goto CLEANUP_JSON;
    }

    uint8_t *text_as_ints = traverse_enigma(enigma);
    if (text_as_ints == NULL)
    {
        fprintf(stderr, "Failed to traverse enigma!\n");
        goto CLEANUP_ENIGMA;
    }

    char *text = get_string_from_int_array(text_as_ints, strlen(enigma->plaintext));
    if (text == NULL)
    {
        fprintf(stderr, "Failed to get enigma plaintext!\n");
        goto CLEANUP_TEXT_AS_INT;
    }

    cJSON_SetValuestring(old_output, text);
    response = cJSON_PrintUnformatted(cjson);

    free(text);
    CLEANUP_TEXT_AS_INT:
        free(text_as_ints);
    CLEANUP_ENIGMA:
        free_enigma(enigma);
    CLEANUP_JSON:
        cJSON_Delete(cjson);
    return response;
}

static int32_t send_http_response(const char *response_json, struct phr_header headers[MAX_HTTP_HEADER], const int sock)
{
    int32_t ret = 0;



    char return_buffer[BUFFER_SIZE] = "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n";
    const size_t len      = strlen(return_buffer);
    const size_t json_len = strlen(response_json);

    const struct phr_header *header = NULL;
    for (uint8_t origin_i = 0; (header = headers + origin_i)->name != NULL && origin_i < MAX_HTTP_HEADER; ++origin_i)
    {
        if (strncmp(headers[origin_i].name, "Origin", 6) == 0)
        {
            break;
        }
    }

    size_t offset = 0;
    if (header != NULL && header->name != NULL)
    {
        offset += snprintf(return_buffer + len, BUFFER_SIZE - len, "Access-Control-Allow-Origin: %.*s\r\n",
                           (int) header->value_len, header->value);
    }
    offset += snprintf(return_buffer + len + offset, BUFFER_SIZE - len - offset, "Content-Length: %zu\r\n\r\n",
                       json_len);
    offset += snprintf(return_buffer + len + offset, BUFFER_SIZE - len - offset, response_json);

    const size_t response_len   = strlen(return_buffer);
    const ssize_t num_bytes_send = send(sock, return_buffer, response_len, 0);
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
    free(keys);
}

static void free_enigma_config(EnigmaConfiguration *conf)
{
    free(conf->ring_settings);
    free(conf->rotor_positions);
    free(conf->rotors);
    free(conf);
}

static cJSON* create_cycle_json(const Cycle *cycles)
{
    cJSON *json = cJSON_CreateObject();
    char *names[3] = {"first",  "second", "third"};
    for (int32_t i = 0; i < 3; ++i)
    {
        char rotor_name[30];
        sprintf(rotor_name, "%s_rotor", names[i]);
        cJSON *rotor = cJSON_AddArrayToObject(json, rotor_name);
        const Cycle *current_cycle = cycles + i;
        for (int j = 0; j < current_cycle->length; ++j)
        {
            cJSON_AddItemToArray(rotor, cJSON_CreateNumber(current_cycle->cycle_values[j]));
        }
    }

    return json;
}

static void* handle_client(void *arg)
{
    ssize_t len;
    const int sock = *(int *) arg;
    free(arg);

    char buffer[BUFFER_SIZE] = {0};
    if ((len = read(sock, buffer, BUFFER_SIZE)) == -1)
    {
        perror("Couldn't read from socket");
        close(sock);

        return NULL;
    }

    puts(buffer);
    const char *method, *path;
    int minor_version;
    struct phr_header headers[MAX_HTTP_HEADER];
    size_t prevbuflen = 0, method_len, path_len, num_headers = MAX_HTTP_HEADER;
    const int pret          = phr_parse_request(buffer, len, &method, &method_len, &path, &path_len,
                                          &minor_version, headers, &num_headers, prevbuflen);
    if (pret < 0)
    {
        fprintf(stderr, "Failed to parse HTTP request\n");
        close(sock);

        return NULL;
    }
    printf("%.*s\n", (int) path_len, path);

    char *tmp_path     = strdup(path);
    tmp_path[path_len] = 0;

    if (strncmp(tmp_path, ENIGMA_PATH, sizeof ENIGMA_PATH - 1) == 0)
    {
        char *response_json = get_response_json(buffer);
        if (response_json == NULL)
        {
            fprintf(stderr, "Failed to generate response JSON!\n");
            close(sock);
            return NULL;
        }
        send_http_response(response_json, headers, sock);
        cJSON_free(response_json);
    }
    else if (strncmp(tmp_path, CYCLOMETER_PATH, sizeof CYCLOMETER_PATH - 1) == 0)
    {
        ServerCyclometerOptions opt;
        const char *json_start = strstr(buffer, "\r\n\r\n") + 4;
        if (get_server_cyclometer_options_from_json(&opt, json_start) != 0)
        {

        }
        char **keys = generate_n_daily_keys(opt.daily_key_count);
        free(opt.enigma_conf->message);
        char **enc_keys = malloc(sizeof(char *) * opt.daily_key_count);
        assertmsg(enc_keys != NULL, "malloc failed");
        for (int32_t i = 0; i < opt.daily_key_count; ++i)
        {
            char current_key[DAILY_KEY_SIZE * 2 + 1] = {0};
            memcpy(current_key, keys[i], DAILY_KEY_SIZE);
            memcpy(current_key + DAILY_KEY_SIZE, keys[i], DAILY_KEY_SIZE);
            opt.enigma_conf->message = current_key;
            Enigma *enigma = create_enigma_from_configuration(opt.enigma_conf);
            uint8_t *enc_key_as_ints = traverse_enigma(enigma);
            char *enc_key = get_string_from_int_array(enc_key_as_ints, DAILY_KEY_SIZE * 2);
            // printf("%s -> %s\n", current_key, enc_key);
            enc_keys[i] = enc_key;

            free(enc_key_as_ints);
            // free(enc_key);
            free_enigma(enigma);
        }

        Cycle *cycles = server_create_cycles(enc_keys, opt.daily_key_count);
        cJSON *cycle_json = create_cycle_json(cycles);
        char *cycle_json_str = cJSON_PrintUnformatted(cycle_json);
        send_http_response(cycle_json_str, headers, sock);

        cJSON_free(cycle_json);
        cJSON_free(cycle_json_str);
        free(cycles);
        free_keys(enc_keys, opt.daily_key_count);
        free_enigma_config(opt.enigma_conf);
        free_keys(keys, opt.daily_key_count);
    }
    else
    {
    }

    CLEANUP:
        free(tmp_path);
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
