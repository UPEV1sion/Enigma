#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"

#include <string.h>

#include "helper/helper.h"
#include "picohttpparser.h"
#include "enigma/enigma.h"
#include "json/cJSON.h"
#include "json/json.h"

//
// Created by Emanuel on 22.12.2024.
//


#define PORT 8081
#define NUM_CLIENTS 10
#define BUFFER_SIZE 4096
#define MAX_HTTP_HEADER 100

static char* get_response_json(const char *input)
{
    char *json;
    if ((json = strstr(input, "{\"model\":")) == NULL) {
        fprintf(stderr, "Invalid JSON!");
        return NULL;
    }
    // char *endptr = strstr(substr, ",\"_value\":");
    //
    // substr[endptr - substr] = 0;
    // puts(substr);

    cJSON *cjson = cJSON_Parse(json);
    cJSON *old_output = cJSON_GetObjectItem(cjson, "output");

    Enigma *enigma = get_enigma_from_json(json);
    assertmsg(enigma != NULL, "Couldn't create enigma");
    uint8_t *text_as_ints = traverse_enigma(enigma);
    assertmsg(text_as_ints != NULL, "Couldn't traverse enigma");
    char *text = get_string_from_int_array(text_as_ints, strlen(enigma->plaintext));
    puts(text);

    cJSON_SetValuestring(old_output, text);

    free(text);
    free(text_as_ints);
    free_enigma(enigma);

    return cJSON_Print(cjson);
}

static int send_http_response(const char *input, struct phr_header headers[MAX_HTTP_HEADER], const int sock)
{
    char *response_json = get_response_json(input);
    puts(response_json);

    char return_buffer[BUFFER_SIZE] = "HTTP/1.1 200 OK\n"
                                       "Content-Type: application/json\n";
    size_t len = strlen(return_buffer);
    size_t json_len = strlen(response_json);
    size_t offset = 0;
    uint8_t origin_i = 0;
    struct phr_header *header = NULL;
    for (;(header = headers + origin_i)->name != NULL && origin_i < 100;++origin_i) {
        if (strncmp(headers[origin_i].name, "Origin", 6) == 0) {
            break;
        }
    }

    if (header->name != NULL) {
        offset += snprintf(return_buffer + len, BUFFER_SIZE - len, "Access-Control-Allow-Origin: %.*s\n", (int)header->value_len, header->value);
    }
    offset += snprintf(return_buffer + len + offset, BUFFER_SIZE - len - offset, "Content-Length: %zu\n\n", json_len);
    offset += snprintf(return_buffer + len + offset, BUFFER_SIZE - len - offset, response_json);
    // puts("\n\n\n\n\n");
    // puts(return_buffer);

    if (send(sock, return_buffer, strlen(return_buffer), 0) == -1) {
        perror("Couldn't send all bytes!");
        return -1;
    }

    cJSON_free(response_json);
    return 0;
}

static void* handle_client(void* arg)
{
    ssize_t len;
    const int sock = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};
    if ((len = read(sock, buffer, BUFFER_SIZE)) == -1)
    {
        perror("Couldn't read from socket");
        close(sock);

        return NULL;
    }

    // puts(buffer);
    const char *method, *path, *msg;
    int pret, minor_version, status;
    struct phr_header headers[MAX_HTTP_HEADER];
    size_t buflen = 0, prevbuflen = 0, method_len, path_len, num_headers = MAX_HTTP_HEADER, msg_len;
    // pret  = phr_parse_response(buffer, len, &minor_version, &status, &msg, &msg_len,
    //                          headers, &num_headers, prevbuflen);
    pret  = phr_parse_request(buffer, len, &method, &method_len, &path, &path_len,
                             &minor_version, headers, &num_headers, prevbuflen);

    printf("request is %d bytes long\n", pret);
    printf("method is %.*s\n", (int)method_len, method);
    printf("path is %.*s\n", (int)path_len, path);
    printf("HTTP version is 1.%d\n", minor_version);
    printf("headers:\n");
    for (size_t i = 0; i != num_headers; ++i) {
        printf("%.*s: %.*s\n", (int)headers[i].name_len, headers[i].name,
               (int)headers[i].value_len, headers[i].value);
    }

    send_http_response(buffer, headers, sock);
}

static int32_t accept_incomming(int sock)
{
    pthread_t thread_ids[NUM_CLIENTS];
    uint8_t thread_count = 0;

    for (;;)
    {
        int new_sock;
        if ((new_sock = accept(sock, NULL, NULL)) == -1)
        {
            perror("Error accepting connection");
            return -1;
        }
        printf("%d\n", new_sock);
        if (pthread_create(thread_ids + thread_count, NULL, handle_client, &new_sock) == -1)
        {
            perror("Error creating thread");
            return -1;
        }
        thread_count++;
        // if (thread_count >= NUM_CLIENTS)
        // {
        //     for (int j = 0; j < NUM_CLIENTS; j++)
        //     {
        //         pthread_join(thread_ids[j], NULL);
        //     }
        //     thread_count = 0;
        // }
    }

    return 0;
}

int32_t server_run(void)
{
    int sock;
    assertmsg((sock = socket(AF_INET, SOCK_STREAM, 0)) != -1, "Couldn't create socket");
    assertmsg(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof (int)) != -1, "Couldn't set socket options");
    struct sockaddr_in server_addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };
    assertmsg(bind(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) != -1, "Couldn't bind");
    assertmsg(listen(sock, 5) != -1, "Couldn't listen");

    return accept_incomming(sock);
}
