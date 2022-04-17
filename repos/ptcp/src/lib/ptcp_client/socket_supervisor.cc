// Ptcp includes
#include <ptcp_client/socket_supervisor.h>
#include <ptcp_client/supervisor_helper.h>

Socket_supervisor::Socket_supervisor(Genode::Allocator &alloc) : _md_alloc(alloc), _sockets() {
    supervisor_helper = new(alloc) Supervisor_helper(*this);
}

void Socket_supervisor::supervise(socket_entry &entry) {
    auto node = new(_md_alloc) Socket_md_node(entry);
    _sockets.insert(node);
    debug_log(SOCKET_SUPERVISOR_DEBUG, "Socket_supervisor now watches ", entry);
}

void Socket_supervisor::abandon(Ptcp::Fd_proxy::Pfd &fd) {
    Socket_md_node *result;

    _sockets.for_each([&](const Socket_md_node &node) {
        debug_log(SOCKET_SUPERVISOR_DEBUG, "node ", &node);
        debug_log(SOCKET_SUPERVISOR_DEBUG, "known ", node._entry.ptcpId.known);
        debug_log(SOCKET_SUPERVISOR_DEBUG, "id ", node._entry.ptcpId.id);
        debug_log(SOCKET_SUPERVISOR_DEBUG, "path ", node._entry.fsPath);

        auto id = node._entry.ptcpId;
        if (id.known) {
            if (fd.value == id.id) result = const_cast<Socket_md_node *>(&node);
        } else {
            Genode::error("Socket_supervisor: socket tree contains corrupted socket!");
        }
    });
    if (result == nullptr)
        Genode::error("Socket_supervisor: can't abandon socket with id ", fd);

    _sockets.remove(result);
}

Socket_supervisor *socket_supervisor;