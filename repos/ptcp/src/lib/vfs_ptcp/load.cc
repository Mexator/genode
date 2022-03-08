// Libcxx includes
#include <fstream>
#include <istream>

// libc includes
#include <sys/socket.h> // socket
#include <internal/snapshot/socket_snapshot.h>

// PTCP includes
#include <vfs_ptcp/load.h>
#include <ptcp_client/fd_proxy.h>


using namespace Ptcp::Snapshot;

const Libc::Plugin_state LIBC_EMPTY{nullptr, 0};
const Lwip_state LWIP_EMPTY{nullptr};

Libc::Plugin_state read_libc_state(std::istream &input, Genode::Allocator &alloc) {
    size_t sockets_number;
    input >> sockets_number;
    if (!sockets_number) {
        Genode::warning("No sockets in snapshot file");
        return LIBC_EMPTY;
    }

    Libc::Socket_state *sockets = new(alloc) Libc::Socket_state[sockets_number];
    for (size_t i = 0; i < sockets_number; ++i) {
        Libc::Socket_state *socket = &sockets[i];
        *socket = Libc::Socket_state();
        input >> socket->proxy_handle >> socket->proto >> socket->state;
    }

    return Libc::Plugin_state{
            sockets,
            sockets_number
    };
}

void Load_manager::read_snapshot_file() {
    // This function should be called after all libc plugins are initialized,
    // because it access VFS using C++ stream api.
    std::ifstream input;
    input.open("/snapshot/saved.txt");

    std::string header;

    input >> header;
    Libc::Plugin_state libc_state = LIBC_EMPTY;
    if (header == "libc") {
        libc_state = read_libc_state(input, _alloc);
    } else {
        Genode::error("Check snapshot format! Missing libc header");
    }

    input >> header;
    Lwip_state lwip_state = LWIP_EMPTY;
    if (header == "lwip") {
        // Todo read lwip
    } else {
        Genode::error("Check snapshot format! Missing lwip header");
    }

    input.close();
    Genode::log("Finished reading state");
    _state = new(_alloc) Composed_state{
            libc_state,
            lwip_state
    };
}

void Load_manager::load_libc_state() {
    // Composed state should be initialized at this point
    Genode::log("Starting libc state restoration");

    // Reopen sockets and set states
    Libc::Plugin_state &state = _state->libc_state;

    Fd_proxy *proxy = get_fd_proxy(_alloc);

    for (size_t i = 0; i < state.sockets_number; ++i) {
        Libc::Socket_state sock_state = state.socket_states[i];

        int sock_fd = socket(AF_INET, sock_state.proto, 0);
        proxy->set(sock_fd, sock_state.proxy_handle);

        socket_state internal_state = socket_state{sock_state.proto, sock_state.state};

        setsockopt(sock_fd, SOL_SOCKET, SO_INTERNAL_STATE, &internal_state, sizeof(internal_state));
    }
}

