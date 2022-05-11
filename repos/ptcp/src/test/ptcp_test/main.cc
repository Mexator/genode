#include <base/heap.h>
#include <libc/component.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

#include <ptcp_client/startup.h>
#include <ptcp_client/fd_proxy.h>
#include <nic/packet_allocator.h>

using Genode::log;
using Genode::warning;
using Genode::error;
using Ptcp::Fd_proxy;

void _main(Libc::Env *env) {
    warning("_main");
    Fd_proxy::Pfd sock = fd_proxy->supervised_socket(AF_INET, SOCK_STREAM, 0);
    if (fd_proxy->map_fd(sock) == -1) {
        error("failed opening socket, errno=", errno);
        return;
    }
    log("Socket created");

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;
    if (0 != bind(fd_proxy->map_fd(sock), (struct sockaddr *) &in_addr, sizeof(in_addr))) {
        error("while calling bind(), errno=", errno);
        return;
    }
    log("Socket bound");

    if (0 != listen(fd_proxy->map_fd(sock), 1)) {
        error("while calling listen(), errno=", errno);
    }
    log("Socket listens");

    while (true) {
        const char message[] = "I am alive!";
        char rcvd_msg[1024];
        char addr[24];
        struct sockaddr_in incoming_addr;
        socklen_t sock_len = sizeof(sockaddr_in);

        Fd_proxy::Pfd accept_fd = fd_proxy->accept(sock, (sockaddr *) &incoming_addr, &sock_len);
        int i = fd_proxy->map_fd(accept_fd);

        log("Accepted ", inet_ntop(AF_INET, &incoming_addr.sin_addr, addr, sizeof(in_addr)));

        read(i, rcvd_msg, sizeof(rcvd_msg));
        log("Read: ", (const char *) rcvd_msg, "\n");
        write(i, message, sizeof(message));
        sleep(1);
        fd_proxy->close(accept_fd);
    }
}

void Libc::Component::construct(Libc::Env &env) {
    log(__func__);
    with_libc([&]() {
        static Genode::Heap heap(env.ram(), env.rm());
        startup_callback(env, heap);
        pthread_t t;
        pthread_create(&t, nullptr, (void *(*)(void *)) (_main), &env);
    });
}