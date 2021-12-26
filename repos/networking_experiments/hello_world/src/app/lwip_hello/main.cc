#include <libc/component.h> // Libc::COmponent

#include <unistd.h>     // close
#include <sys/socket.h> // socket
#include <netinet/in.h> // in_addr
#include <stdio.h>      // puts
#include <arpa/inet.h>  // inet_pton
#include "shared_constants.h"

char receive_buf[CHUNK_SIZE];

void serve(Libc::Env &env)
{
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (0 == sock)
        env.parent().exit(-1);

    puts("Socket created");

    // Bind & listen
    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;

    if (0 != bind(sock, (struct sockaddr *)&in_addr, sizeof(in_addr)))
        env.parent().exit(-1);

    puts("Socket binded");

    if (0 != listen(sock, 1))
        env.parent().exit(-1);

    puts("Socket listens");

    // Wait for client
    while (true)
    {
        puts("Loop pass");

        // Accept client
        struct sockaddr _client_addr;
        socklen_t len = sizeof(_client_addr);
        int client = accept(sock, &_client_addr, &len);
        char a[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in *client_addr = (sockaddr_in *)&_client_addr;
        inet_ntop(AF_INET, &client_addr->sin_addr, a, INET_ADDRSTRLEN);
        printf("Client accepted: %s\n", a);

        // Negotiate payload length
        int32_t payload_count;
        if (read(client, &receive_buf, sizeof(receive_buf)) == -1)
        {
            puts("Failed read count of expected data chunks");
            close(client);
            continue;
        }
        payload_count = *(int32_t *)(receive_buf);
        payload_count = ntohl(payload_count);
        printf("Negotiated payload count %d\n", payload_count);

        // Receive payload

        for (int i = 0; i < payload_count; i++)
        {
            read(client, receive_buf, CHUNK_SIZE);
            puts("Received:");
            puts(receive_buf);
        }
        close(client);
    }
}

void Libc::Component::construct(Libc::Env &env)
{
    with_libc([&]()
              { serve(env); });
}