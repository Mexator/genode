#include <libc/component.h>
#include <base/log.h>
#include <base/heap.h>

#include <pthread.h>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h> // in_addr
#include <string>

#include <ptcp_client/startup.h>
#include <ptcp_client/fd_proxy.h>

using Genode::log;
using Genode::warning;
using Genode::error;
using Ptcp::Pfd;

void stage_one() {
    Pfd sock;
    {
        int libc_socket = socket(AF_INET, SOCK_STREAM, 0);
        sock = fd_proxy->register_fd(libc_socket);
    }
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

    char rcvd_msg[1024];
    char addr[24];
    struct sockaddr_in incoming_addr;
    socklen_t sock_len = sizeof(sockaddr_in);

    Pfd accept_fd;
    {
        int libc_accept_fd = accept(fd_proxy->map_fd(sock), (sockaddr *) &incoming_addr, &sock_len);
        accept_fd = fd_proxy->register_fd(libc_accept_fd);
    }
    std::ofstream appSnap;
    appSnap.open("/snapshot/restore_accept");
    appSnap << accept_fd.value;
    appSnap.close();

    int i = fd_proxy->map_fd(accept_fd);

    log("Accepted ", inet_ntop(AF_INET, &incoming_addr.sin_addr, addr, sizeof(in_addr)));

    while (true) {
        int len = read(i, rcvd_msg, 1024);
        warning("Read returned ", len, "errno", errno);
        log("Read: ", (const char *) rcvd_msg, "\n");
        write(i, rcvd_msg, len);
    }
}

void stage_two(Pfd fd) {
    int i = fd_proxy->map_fd(fd);
    warning(fd, " ", i);
    char rcvd_msg[1024];

    while (true) {
        log("Reading");
        int len = read(i, rcvd_msg, 1024);
        if (!len || len < 0) {
            warning("Read returned 0");
            return;
        }
        rcvd_msg[len] = '\0';

        std::string s(rcvd_msg);
        log("Reversed: ", (const char *) rcvd_msg);
        std::string rev(s.rbegin(), s.rend());
        rev.append("\n");
        log("Reversed: ", (const char *) rev.c_str(), "\n");
        write(i, rev.c_str(), len);
        sleep(1);
    }
}

void _main() {
    std::ifstream appSnap;
    appSnap.open("/snapshot/restore_accept");
    if (!appSnap.is_open()) {
        warning("Failed to open snapshot. Going to stage one!");
        Genode::log("---------------------------");
        Genode::log("Starting simple echo server");
        Genode::log("---------------------------");
        stage_one();
    } else {
        unsigned long acceptedPfd;
        appSnap >> acceptedPfd;
        appSnap.close();
        Pfd fd{acceptedPfd};
        Genode::log("----------------------------");
        Genode::log("Starting REVERSE echo server");
        Genode::log("----------------------------");
        stage_two(fd);
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
