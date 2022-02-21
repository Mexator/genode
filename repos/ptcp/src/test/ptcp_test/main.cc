#include <libc/component.h>
#include <stdio.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket

// lwip private api (subject to test)
#define LWIP_NO_STDINT_H 0
#define LWIP_NO_UNISTD_H 0
#define SSIZE_MAX
#include <lwip/priv/tcp_priv.h>

void test() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;

    if (0 != bind(sock, (struct sockaddr *) &in_addr, sizeof(in_addr)))
        Genode::error("while calling bind()");

    Genode::log("test: tcp_bound_pcbs->local_port. If it is 80, test is OK\n", tcp_bound_pcbs->local_port);
}

void Libc::Component::construct(Libc::Env &env) {
    with_libc([&]() {
        puts("hello world");
        test();
    });
}