// Ptcp includes
#include <ptcp_client/ptcp_lock.h>
#include <ptcp_client/socket_supervisor.h>
#include <ptcp_client/supervisor_helper.h>
#include <ptcp_client/serialized/serialized_socket_state.h>

Socket_supervisor::Socket_supervisor(
        Genode::Allocator &alloc,
        Nic_control::Connection &conn
) : _md_alloc(alloc), _sockets(), _conn(conn) {
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

void Socket_supervisor::dump(std::ostream &out) {
    /*
     * While dumping we need
     * 1) Block all outgoing sends. To do this I simply lock mutex used by my VFS plugin
     * 2) Block all incoming packets. This is not that simple, as I do not want them to be ACKed by TCP.
     *    To overcome this, I created proxy NIC component that stops submitting packets to clients after suspend() call
     */
    Genode::Mutex::Guard _(Ptcp::mutex);
    _conn.suspend();
    debug_log(SOCKET_SUPERVISOR_SAVE_LOG, "socket_supervisor: Dumping...");

    int count = 0;
    _sockets.for_each([&](const Socket_md_node &) {
        count++;
    });
    out << count << "\n";
    _sockets.for_each([&](const Socket_md_node &node) {
        auto entry = node._entry;
        std::string str(entry.boundAddress);
        serialized_socket sock{entry.ptcpId.id, entry.tcpState, !str.empty(), entry.boundAddress};
        sock.save(out);
    });

    debug_log(SOCKET_SUPERVISOR_SAVE_LOG, "Socket_supervisor: dump done");
    _conn.resume();
}

socket_entry *Socket_supervisor::get_entry_for(const char *path) {
    socket_entry *found = nullptr;
    _sockets.for_each([&](const Socket_md_node &node) {
        auto &socket = node._entry;
        if (0 == Genode::strcmp(path, socket.socketPath.string(), socket.pathLen)) {
            found = &socket;
        }
    });
    return found;
}

socket_entry *Socket_supervisor::get_entry_for(Vfs::Vfs_handle &handle) {
    socket_entry *found = nullptr;
    _sockets.for_each([&](const Socket_md_node &node) {
        auto &socket = node._entry;
        if (socket.belongs_to_this(handle)) found = &socket;
    });
    return found;
}

Socket_supervisor *socket_supervisor;