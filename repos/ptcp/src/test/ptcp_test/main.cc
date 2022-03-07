#include <libc/component.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libc-plugin/fd_alloc.h>
#include <pthread.h>
#include <ptcp_client/fd_proxy.h>
#include <base/heap.h>

void _main(Libc::Env *env) {
    static Genode::Heap heap(env->ram(), env->rm());
    Ptcp::Fd_proxy *proxy = Ptcp::get_fd_proxy(heap);

    // Assume app remembered fd 100 and we restored it as socket with fd=2

    int libc_fd = proxy->map_fd(Ptcp::Fd_proxy::Fd_space::Id{100});
    if (libc_fd == 2) {
        Genode::warning("Fd_proxy: Test succeded"); // Should be 2
    }

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
        open("/socket/nonexistent", O_RDONLY); // Make vfs plugin aware that libc is initialized

        pthread_t t;
        pthread_create(&t, nullptr, (void *(*)(void *)) (_main), &env);
    });
}