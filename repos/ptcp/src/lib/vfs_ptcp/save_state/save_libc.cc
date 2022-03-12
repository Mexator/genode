#include <vfs_ptcp/save.h>

void save_addr(std::ostream &out, sockaddr_in addr) {
    out << addr.sin_addr.s_addr << " ";
    out << addr.sin_port << " ";
}

void save_libc_state(std::ostream &out, const Ptcp::Snapshot::Libc::Plugin_state &state) {
    out << state.sockets_number << std::endl;

    for (size_t i = 0; i < state.sockets_number; ++i) {
        Ptcp::Snapshot::Libc::Socket_state sock_state = state.socket_states[i];
        out << sock_state.proxy_handle << " ";
        out << sock_state.proto << " ";
        out << sock_state.state << " ";
        save_addr(out, sock_state.addr);
        out << std::endl;
    }
}