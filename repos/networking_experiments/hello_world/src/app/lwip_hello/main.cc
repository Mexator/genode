#include <libc/component.h> // Libc::COmponent

#include <unistd.h>     // close
#include <sys/socket.h> // socket
#include <netinet/in.h> // in_addr
#include <stdio.h>      // puts

void serve()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock != 0)
        puts("sock created");

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;
    int res = bind(sock, (struct sockaddr *)&in_addr, sizeof(in_addr));

    if (res == 0)
        puts("sock binded");

    res = listen(sock, 1);

    if (res == 0)
        puts("sock listens");

    while (true)
    {
        puts("loop pass");
        struct sockaddr addr;
        socklen_t len = sizeof(addr);
        int client = accept(sock, &addr, &len);
        puts("client accepted");

        char buf[1024];

        read(client, buf, 1024);
        send(client, buf, 1024, 0);
    }
}

void Libc::Component::construct(Libc::Env &env)
{
    with_libc([&]()
              { serve(); });
}