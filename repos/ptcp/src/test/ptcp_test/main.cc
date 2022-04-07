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
#include <vector>

int read_snapshot(std::vector<u_long> *result) {
    std::ifstream snapshot;
    snapshot.open("/snapshot/app_state");
    if (!snapshot.is_open()) return -1;
    unsigned long id1, id2;
    snapshot >> id1 >> id2;
    result->push_back(id1);
    result->push_back(id2);
    Genode::log("App state read!");
    snapshot.close();

    return 0;
}

int check_snapshot(Ptcp::Fd_proxy &proxy, std::vector<unsigned long> &ids) {
    for (unsigned long id: ids) {
        int res = proxy.map_fd(Ptcp::Fd_proxy::Fd_space::Id{id});
        if (res == -1) return -1;
    }
    return 0;
}

void save_app_state(std::vector<Ptcp::Fd_proxy::Fd_space::Id> result) {
    std::ofstream snapshot;
    snapshot.open("/snapshot/app_state");
    snapshot << result[0].value << " " << result[1].value;
    snapshot.close();
}

void _main(Libc::Env *env) {
    Genode::warning("_main");

    static Genode::Heap heap(env->ram(), env->rm());
    Ptcp::Fd_proxy *proxy = Ptcp::get_fd_proxy(heap);

    Ptcp::Fd_proxy::Fd_space::Id sock;
    Ptcp::Fd_proxy::Fd_space::Id sock2;
    {
        std::vector<u_long> socks;
        if ((0 == read_snapshot(&socks)) && (0 == check_snapshot(*proxy, socks))) {
            Genode::log("Restoring");
            sock = Ptcp::Fd_proxy::Fd_space::Id{socks[0]};
            sock2 = Ptcp::Fd_proxy::Fd_space::Id{socks[1]};
        } else {
            Genode::error("failed state restoration. Fallback to normal startup");
            sock = proxy->register_fd(socket(AF_INET, SOCK_STREAM, 0));
            sock2 = proxy->register_fd(socket(AF_INET, SOCK_DGRAM, 0));

            struct sockaddr_in in_addr;
            in_addr.sin_family = AF_INET;
            in_addr.sin_port = htons(80);
            in_addr.sin_addr.s_addr = INADDR_ANY;

            if (0 != bind(proxy->map_fd(sock), (struct sockaddr *) &in_addr, sizeof(in_addr)))
                Genode::error("while calling bind()");

            save_app_state(std::vector<Ptcp::Fd_proxy::Fd_space::Id>{sock, sock2});
        }
    }

    Genode::log("Bind");
    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_addr.s_addr = INADDR_ANY;
    in_addr.sin_port = htons(85);
    if (0 != bind(proxy->map_fd(sock2), (struct sockaddr *) &in_addr, sizeof(in_addr)))
        Genode::error("while calling bind()");
    Genode::log("Bind ok");

    Genode::log("Listen");
    if (0 != listen(proxy->map_fd(sock), 1)) {
        Genode::error("while calling listen()");
    }
    Genode::log("Listen ok");

    while (true) {
        struct sockaddr_in in_addr2;
        socklen_t sock_len = sizeof(sockaddr_in);
        auto i = proxy->register_fd(accept(proxy->map_fd(sock), (sockaddr *) &in_addr2, &sock_len));
        Genode::log("accepted", in_addr2.sin_addr.s_addr);
        char a[16];
        read(proxy->map_fd(i), a, 16);
        printf("read %s", a);
        write(proxy->map_fd(i),"ABOBUS !!!", 10);
        sleep(4);
        close(proxy->close_fd(i));
    }
}

void Libc::Component::construct(Libc::Env &env) {
    Genode::warning("construct");
    with_libc([&]() {
        pthread_t t;
        pthread_create(&t, nullptr, (void *(*)(void *)) (_main), &env);
    });
}

/* exceptions
 *Extern_factory::create(Vfs::Env &, Genode::Xml_node )
 * Persistent heap - 0x11000000 .. 0x12000000
 * Lwip_fs -         0x11014000    0x1100a2b6
 *
 * page fault, pd='init -> ptcp_test' thread='ep' cpu=0 ip=0x12667f9 address=0x1100a2b6 stack pointer=0x401fe5d0 qualifiers=0x4 irUwp reason=1
 *
[init -> ptcp_test]   0x1000000 .. 0x10ffffff: linker area
[init -> ptcp_test]   0x40000000 .. 0x4fffffff: stack area
[init -> ptcp_test]   0x30000 .. 0x14dfff: ld.lib.so
[init -> ptcp_test]   0x10e1c000 .. 0x10ffffff: libc.lib.so
[init -> ptcp_test]   0x10d77000 .. 0x10e1bfff: vfs.lib.so
[init -> ptcp_test]   0x10d35000 .. 0x10d76fff: libm.lib.so
[init -> ptcp_test]   0x1008000 .. 0x100bfff: persalloc.lib.so
[init -> ptcp_test]   0x100c000 .. 0x100dfff: ptcp_client.lib.so
[init -> ptcp_test]   0x100e000 .. 0x11cefff: stdcxx.lib.so
[init -> ptcp_test]   0x10d16000 .. 0x10d2cfff: vfs_rump.lib.so
[init -> ptcp_test]   0x11cf000 .. 0x13dbfff: rump.lib.so
[init -> ptcp_test]   0x13dc000 .. 0x14eafff: rump_fs.lib.so
[init -> ptcp_test]   0x10ce9000 .. 0x10d15fff: vfs_ptcp.lib.so
[init -> ptcp_test]   0x14eb000 .. 0x1522fff: lwip_dl.lib.so
[init -> ptcp_test]   0x10cbd000 .. 0x10ce8fff: vfs_lwip_dl.lib.so

 */