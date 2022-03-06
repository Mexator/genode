#include <libc/component.h>
#include <stdio.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libc-plugin/fd_alloc.h>
#include <pthread.h>

void _main() {
    int snapshot = open("/socket/fd_mapping", O_RDONLY);

    char buf[1024];
    int res = read(snapshot, buf, 1024);
    Genode::warning("snapshot: read returned ", res);
    Genode::log("Fd_mapping:");
    puts(buf);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;

    if (0 != bind(sock, (struct sockaddr *) &in_addr, sizeof(in_addr)))
        Genode::error("while calling bind()");

    in_addr.sin_port = htons(85);
    int sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (0 != bind(sock2, (struct sockaddr *) &in_addr, sizeof(in_addr)))
        Genode::error("while calling bind()");

    listen(sock, 1);

    while (true) {
        struct sockaddr_in in_addr2;
        socklen_t sock_len = sizeof(sockaddr_in);
        accept(sock, (sockaddr *) &in_addr2, &sock_len);
        Genode::log("accepted", in_addr2.sin_addr.s_addr);
        sleep(1);
    }
}

void Libc::Component::construct(Libc::Env &env) {

    with_libc([&]() {
        pthread_t t;
        pthread_create(&t, nullptr, (void *(*)(void *)) (_main), nullptr);
    });
}