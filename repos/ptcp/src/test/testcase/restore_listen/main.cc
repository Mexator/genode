#include <base/heap.h>
#include <libc/component.h>
#include <vector>
#include <fstream>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

#include <ptcp_client/startup.h>
#include <ptcp_client/fd_proxy.h>

using Genode::log;
using Genode::warning;
using Genode::error;
using Ptcp::Fd_proxy;

std::vector<Fd_proxy::Pfd> create_sockets(int count) {
    std::vector<Fd_proxy::Pfd> vector;

    for (int i = 0; i < count; ++i) {
        Fd_proxy::Pfd pfd = fd_proxy->supervised_socket(AF_INET, SOCK_STREAM, 0);
        vector.push_back(pfd);
    }
    return vector;
}

struct testcase_sockets {
    // ports 4000 4001
    Fd_proxy::Pfd bound1, bound2;
    // ports 4002 4003
    Fd_proxy::Pfd listen1, listen2;
    // ports 4004 4005
    Fd_proxy::Pfd closed1, closed2;
    std::vector<Fd_proxy::Pfd> all;

    static testcase_sockets from_vector(std::vector<Fd_proxy::Pfd> fds) {
        return testcase_sockets{
                fds[0], fds[1],
                fds[2], fds[3],
                fds[4], fds[5],
                fds
        };
    }

    static const int base_port = 4000;

    int count() { return all.size(); }
};

testcase_sockets initialize(std::vector<Fd_proxy::Pfd> vector) {
    testcase_sockets sockets = testcase_sockets::from_vector(vector);
    for (int i = 0; i < 4; ++i) {
        struct sockaddr_in in_addr;
        in_addr.sin_family = AF_INET;
        in_addr.sin_port = htons(testcase_sockets::base_port + i);
        in_addr.sin_addr.s_addr = INADDR_ANY;
        if (0 != bind(fd_proxy->map_fd(vector[i]), (struct sockaddr *) &in_addr, sizeof(in_addr))) {
            error("while calling bind(), errno=", errno, "i = ", i);
        }
    }

    if (0 != listen(fd_proxy->map_fd(sockets.listen1), 1)) {
        error("while calling listen(), errno=", errno);
    }
    log("Socket 1 listens");

    if (0 != listen(fd_proxy->map_fd(sockets.listen2), 1)) {
        error("while calling listen(), errno=", errno);
    }
    log("Socket 2 listens");

    return sockets;
}

void verify(testcase_sockets sockets) {
    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = INADDR_ANY;
    int res = 0;

    in_addr.sin_port = htons(4004);
    res += bind(fd_proxy->map_fd(sockets.closed1), (struct sockaddr *) &in_addr, sizeof(in_addr));
    in_addr.sin_port = htons(4005);
    res += bind(fd_proxy->map_fd(sockets.closed2), (struct sockaddr *) &in_addr, sizeof(in_addr));
    if (res != 0) error("while binding closed sockets");

    res = listen(fd_proxy->map_fd(sockets.closed1), 1);
    res = listen(fd_proxy->map_fd(sockets.closed2), 1);
    res = listen(fd_proxy->map_fd(sockets.bound1), 1);
    res = listen(fd_proxy->map_fd(sockets.bound2), 1);
    if (res != 0) error("while calling listen");

    for (int i = 0; i < sockets.count(); ++i) {
        Fd_proxy::Pfd socket = sockets.all[i];

        struct sockaddr_in incoming_addr;
        char addr_str[INET6_ADDRSTRLEN];
        socklen_t sock_len = sizeof(sockaddr_in);

        Genode::log("Waiting for accept for socket at port", testcase_sockets::base_port + i);
        fd_proxy->accept(socket, (sockaddr *) &incoming_addr, &sock_len);
        log("Accepted ", inet_ntop(AF_INET, &incoming_addr.sin_addr, addr_str, sizeof(in_addr)));
    }

    Genode::warning("Test case succeeded");
}

void _main(Libc::Env *env) {
    std::fstream snapshot;
    snapshot.open("/snapshot/testcase1", std::fstream::in);
    if (!snapshot.is_open()) {
        snapshot.close();
        Genode::warning("Snapshot file not exists -- initialization;");
        auto sockets = initialize(create_sockets(6));
        snapshot.open("/snapshot/testcase1", std::fstream::out);
        for (auto socket: sockets.all) {
            snapshot << socket.value << " ";
        }
        snapshot.close();
    } else {
        Genode::warning("Snapshot file exists -- verification;");
        std::vector<Fd_proxy::Pfd> vector = std::vector<Fd_proxy::Pfd>(6);
        for (int i = 0; i < 6; ++i) {
            snapshot >> vector[i].value;
        }
        snapshot.close();
        testcase_sockets sockets = testcase_sockets::from_vector(vector);
        verify(sockets);
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