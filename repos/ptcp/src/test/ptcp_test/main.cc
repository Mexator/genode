#include <libc/component.h>
#include <stdio.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libc-plugin/fd_alloc.h>
#include <list>
#include <pthread.h>

void _main() {
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

    while(true){
        ;
    }
}

void Libc::Component::construct(Libc::Env &env) {

    with_libc([&]() {
        pthread_t t;
        pthread_create(&t, nullptr, (void *(*)(void *))(_main), nullptr);
    });
}