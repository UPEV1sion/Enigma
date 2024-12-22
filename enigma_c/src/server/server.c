#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include "server.h"
#include "helper/helper.h"

//
// Created by Emanuel on 22.12.2024.
//


#define PORT 17576
#define NUM_CLIENTS 10
#define BUFFER_SIZE 4096

static void* handle_client(void* arg)
{
    const int sock = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};
    if (read(sock, buffer, BUFFER_SIZE) < 0)
    {
        perror("Couldn't read from socket");
        close(sock);

        return NULL;
    }
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
    assertmsg((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1, "Couldn't create socket");
    assertmsg(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof (int)) == -1, "Couldn't set socket options");
    struct sockaddr_in server_addr = {
        .sin_family      = AF_INET,
        .sin_port        = htons(PORT),
        .sin_addr.s_addr = INADDR_ANY,
    };
    assertmsg(bind(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == -1, "Couldn't bind");
    assertmsg(listen(sock, 5) == -1, "Couldn't listen");

    return accept_incomming(sock);
}
