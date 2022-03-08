#include <libc/component.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <ptcp_client/fd_proxy.h>
#include <base/heap.h>
#include <fstream>

void _main(Libc::Env *env) {
    static Genode::Heap heap(env->ram(), env->rm());
    Ptcp::Fd_proxy *proxy = Ptcp::get_fd_proxy(heap);

    Ptcp::Fd_proxy::Fd_space::Id sock;
    Ptcp::Fd_proxy::Fd_space::Id sock2;

    try {
        std::ifstream snapshot;
        snapshot.open("/snapshot/app_state");
        if (!snapshot.is_open()) throw Genode::Exception{};
        unsigned long id1, id2;
        snapshot >> id1 >> id2;
        sock = Ptcp::Fd_proxy::Fd_space::Id{id1};
        sock2 = Ptcp::Fd_proxy::Fd_space::Id{id2};
        Genode::log("State restored!");
        snapshot.close();
    } catch (...) {
        Genode::error("failed state restoration. Fallback to normal startup");
        sock = proxy->register_fd(socket(AF_INET, SOCK_STREAM, 0));
        sock2 = proxy->register_fd(socket(AF_INET, SOCK_DGRAM, 0));
        std::ofstream snapshot;
        snapshot.open("/snapshot/app_state");
        snapshot << sock.value << " " << sock2.value;
        snapshot.close();
    }

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;

    if (0 != bind(proxy->map_fd(sock), (struct sockaddr *) &in_addr, sizeof(in_addr)))
        Genode::error("while calling bind()");

    in_addr.sin_port = htons(85);
    if (0 != bind(proxy->map_fd(sock2), (struct sockaddr *) &in_addr, sizeof(in_addr)))
        Genode::error("while calling bind()");

    listen(proxy->map_fd(sock), 1);

    while (true) {
        struct sockaddr_in in_addr2;
        socklen_t sock_len = sizeof(sockaddr_in);
        accept(proxy->map_fd(sock), (sockaddr *) &in_addr2, &sock_len);
        Genode::log("accepted", in_addr2.sin_addr.s_addr);
        sleep(1);
    }
}

void Libc::Component::construct(Libc::Env &env) {
    with_libc([&]() {
        pthread_t t;
        pthread_create(&t, nullptr, (void *(*)(void *)) (_main), &env);
    });
}