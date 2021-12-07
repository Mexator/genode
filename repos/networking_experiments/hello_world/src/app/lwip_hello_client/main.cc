#include <libc/component.h> // Libc::COmponent

#include <unistd.h>     // close
#include <sys/socket.h> // socket
#include <netinet/in.h> // in_addr
#include <stdio.h>      // puts
#include <arpa/inet.h>  // inet_pton

void spam(Libc::Env &env)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock != 0)
        puts("sock created");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "172.20.0.1", &serv_addr.sin_addr.s_addr);
    serv_addr.sin_port = htons(80);

    int res = connect(sock, (const sockaddr *)&serv_addr, sizeof(serv_addr));

    if (res == 0)
        puts("sock connected");
    else
        env.parent().exit(-1);

    for (int i = 0; i < 10; i++)
    {
        puts("loop pass");

        char buf[1024] = "asdasd";

        if (write(res, buf, 1024) == -1)
        {
            puts("send failed");
        }
        if (read(res, buf, 1024) == -1)
        {
            puts("recv failed");
        }
        puts(buf);
    }
}

void Libc::Component::construct(Libc::Env &env)
{
    with_libc([&]()
              { spam(env); });
}