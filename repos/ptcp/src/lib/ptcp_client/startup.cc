// Libc includes
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <fstream>

// Ptcp includes
#include <ptcp_client/startup.h>
#include <ptcp_client/socket_supervisor.h>
#include <ptcp_client/serialized/serialized_socket_state.h>
#include <ptcp_client/fd_proxy.h>

using Genode::log;
using Genode::error;
using namespace Ptcp;

void init(Genode::Env &env, Genode::Allocator &alloc) {
    fd_proxy = new(alloc) Fd_proxy(alloc);

    Genode::log("Creating Nic_control connection...");
    Nic_control::Connection *conn = new(alloc) Nic_control::Connection(env);
    Genode::log("Nic_control Connected!");

    socket_supervisor = new(alloc) Socket_supervisor(alloc, *conn);
}

void restore_sockets_state() {
    std::fstream snapshot;
    snapshot.open("/snapshot/sockets");
    if (!snapshot.is_open()) {
        snapshot.close();
        Genode::warning("Can't open snapshot");
        return;
    }

    // Read
    int len = 0;
    snapshot >> len;
    Genode::warning("len ", len);
    if (len == 0) {
        snapshot.close();
        return;
    }
    serialized_socket entries[len];
    for (int i = 0; i < len; ++i) {
        Genode::warning("load ", i);
        entries[i] = serialized_socket::load(snapshot);
        entries[i].save(std::cout); // Load logging
    }

    // Reopen
    for (int i = 0; i < len; ++i) {
        Genode::warning("open ", i);
        int libc_fd = socket(AF_INET, SOCK_STREAM, 0);
        fd_proxy->set(libc_fd, entries[i].pfd);
    }

    // Restore state
    for (int i = 0; i < len; ++i) {
        auto pfd = entries[i].pfd;
        int libc_fd = fd_proxy->map_fd(Fd_proxy::Pfd{pfd});
        if (char *addr = entries[i].boundAddress) {
            struct sockaddr_in in_addr;
            in_addr.sin_family = AF_INET;

            strtok(addr, ":");
            char *port = strtok(nullptr, ":");

            inet_aton((const char *) addr, &in_addr.sin_addr);
            in_addr.sin_port = htons(atoi((const char *) port));
            Genode::warning("port, ", (const char *) port);

            if (0 != bind(libc_fd, (struct sockaddr *) &in_addr, sizeof(in_addr))) {
                error("while calling bind(), errno=", errno);
                return;
            }
            log("Socket bound IN RESTORE");
        }
        if (entries[i].state == LISTEN) {
            if (0 != listen(libc_fd, 1)) {
                error("while calling listen(), errno=", errno);
            }
            log("Socket listens IN RESTORE");
        }
    }

    snapshot.close();
}

[[noreturn]] void dump_loop() {
    while (true) {
        sleep(5);
        std::ofstream snapshot;
        snapshot.open("/snapshot/sockets");
        socket_supervisor->dump(snapshot);
        snapshot.close();
    }
}

void startup_callback(Genode::Env &env, Genode::Allocator &alloc) {
    // init controller classes
    init(env, alloc);
    // load state
    restore_sockets_state();
    // Start dump loop
    pthread_t dumper;
    pthread_create(&dumper, nullptr, (void *(*)(void *)) (dump_loop), nullptr);
}