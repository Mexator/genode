#include <libc/component.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <cstdio>
#include <arpa/inet.h>

using Genode::log;
using Genode::warning;
using Genode::error;

void _main(Libc::Env *env) {
    warning("_main");
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        error("failed opening socket, errno=", errno);
        return;
    }
    log("Socket created");

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;
    if (0 != bind(sock, (struct sockaddr *) &in_addr, sizeof(in_addr))) {
        error("while calling bind(), errno=", errno);
        return;
    }
    log("Socket bound");

    if (0 != listen(sock, 1)) {
        error("while calling listen(), errno=", errno);
    }
    log("Socket listens");

    while (true) {
        const char message[] = "I am alive!";
        char rcvd_msg[1024];
        char addr[24];
        struct sockaddr_in incoming_addr;
        socklen_t sock_len = sizeof(sockaddr_in);

        int i = accept(sock, (sockaddr *) &incoming_addr, &sock_len);

        log("Accepted ", inet_ntop(AF_INET, &incoming_addr.sin_addr, addr, sizeof(in_addr)));

        read(i, rcvd_msg, sizeof(rcvd_msg));
        printf("Read: %s \n", rcvd_msg);
        write(i, message, sizeof(message));
        sleep(4);
        close(i);
    }
}

void Libc::Component::construct(Libc::Env &env) {
    log(__func__);
    with_libc([&]() {
        pthread_t t;
        pthread_create(&t, nullptr, (void *(*)(void *)) (_main), &env);
    });
}