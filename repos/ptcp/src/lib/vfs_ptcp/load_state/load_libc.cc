// libc includes
#include <sys/socket.h>
#include <unistd.h>
#include <internal/snapshot/socket_snapshot.h>

// PTCP includes
#include <vfs_ptcp/load.h>
#include <ptcp_client/fd_proxy.h>

using namespace Ptcp::Snapshot::Libc;

void read_ip_addr(std::istream &input, sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    input >> addr->sin_addr.s_addr;
    input >> addr->sin_port;
}

Plugin_state read_libc_state(std::istream &input, Genode::Allocator &alloc) {
    size_t sockets_number;
    input >> sockets_number;
    if (!sockets_number) {
        Genode::warning("No sockets in snapshot file");
        return LIBC_EMPTY;
    }

    Socket_state *sockets = new(alloc) Socket_state[sockets_number];
    for (size_t i = 0; i < sockets_number; ++i) {
        Socket_state *socket = &sockets[i];
        *socket = Socket_state();
        input >> socket->proxy_handle >> socket->proto >> socket->state;

        sockaddr_in *addr = &socket->addr;
        read_ip_addr(input, addr);
    }

    return Plugin_state{
            sockets,
            sockets_number
    };
}

class Ptcp::Snapshot::Restore_state_accessor {
public:
    static void set(Ptcp::Fd_proxy &proxy, const int libc_fd, const unsigned long proxy_fd) {
        proxy.set(libc_fd, proxy_fd);
    }
};

constexpr char RESTORE_PREFIX[] = "DURING RESTORE";

void restore_libc_state(const Plugin_state &state, Genode::Allocator &alloc) {
    // Reopen sockets and set states
    Ptcp::Fd_proxy *proxy = Ptcp::get_fd_proxy(alloc);

    for (size_t i = 0; i < state.sockets_number; ++i) {
        Socket_state sock_state = state.socket_states[i];

        int sock_fd = socket(AF_INET, sock_state.proto, 0);
        if (sock_fd == -1) {
            Genode::error(RESTORE_PREFIX, "while creating socket");
            continue;
        }

        if (0 != bind(sock_fd, (struct sockaddr *) &sock_state.addr, sizeof(sock_state.addr))) {
            Genode::error(RESTORE_PREFIX, "while calling bind()");
            close(sock_fd);
            continue;
        }

        socket_state internal_state = socket_state{sock_state.proto, sock_state.state};

        if (0 != setsockopt(sock_fd, SOL_SOCKET, SO_INTERNAL_STATE, &internal_state, sizeof(internal_state))) {
            Genode::error(RESTORE_PREFIX, "while calling setsockopt()");
            close(sock_fd);
            continue;
        }

        Ptcp::Snapshot::Restore_state_accessor::set(*proxy, sock_fd, sock_state.proxy_handle);
    }
}