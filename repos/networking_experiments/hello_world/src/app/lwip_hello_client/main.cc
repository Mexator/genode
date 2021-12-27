#include <libc/component.h> // Libc::COmponent

#include <unistd.h>     // close
#include <sys/socket.h> // socket
#include <netinet/in.h> // in_addr
#include <stdio.h>      // puts
#include <arpa/inet.h>  // inet_pton
#include <errno.h>
#include <netinet/tcp.h>
#include "shared_constants.h"

const char SERVER_IP[] = "172.20.0.25";
const uint32_t TRANS_COUNT = 16;
char buf[CHUNK_SIZE];

void spam(Libc::Env &env)
{
    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0 == sock)
        env.parent().exit(-1);

    // Try to set sockopts to avoid concatetanion of
    // payload chunks into one segment
    // (I use it wrong seemingly)
    int flag = 1;
    int res = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(int));
    printf("res: %d\n", res);
    printf("errno: %d\n", errno);

    puts("Socket created");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(SERVER_PORT);

    // Connect
    if (0 == connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)))
        puts("Socket connected");
    else
        env.parent().exit(-1);

    // Negotiate payload length
    int32_t conv = htonl(TRANS_COUNT);
    *(int32_t *)buf = conv;
    printf("Sending count of expected data chunks %x \n", conv);
    if (send(sock, &buf, CHUNK_SIZE, 0) == -1)
    {
        printf("Failed sending count of expected data chunks, error: %d \n", errno);
    }

    // Sending payload
    for (int i = 1; i <= TRANS_COUNT; i++)
    {
        printf("Transmitting item %d out of %d \n", i, TRANS_COUNT);

        sprintf(buf, "Item %d", i);

        if (send(sock, buf, CHUNK_SIZE, 0) == -1)
        {
            puts("Send failed. Exitting...");
            env.parent().exit(-1);
        }

        // TCP_NODELAY seem to be not doing what I want it to do
        // (segments are glued together), so I added pause
        // between transmissions
        usleep(500000);
    }
    close(sock);
}

void Libc::Component::construct(Libc::Env &env)
{
    with_libc([&]()
              { spam(env); });
    env.parent().exit(0);
}