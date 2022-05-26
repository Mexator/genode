// Genode includes
#include <net/ipv4.h>

// Ptcp includes
#include <ptcp_client/ptcp_lock.h>
#include <ptcp_client/socket_supervisor.h>
#include <ptcp_client/supervisor_helper.h>
#include <ptcp_client/serialized/serialized_socket_state.h>

// libc includes
#include <string>

// Debug includes
#include <logging/mylog.h>

Socket_supervisor::Socket_supervisor(
        Genode::Allocator &alloc,
        Genode::Env &env,
        Nic_control::Connection &conn
) : _md_alloc(alloc), _sockets(), _env(env), _conn(conn) {
    supervisor_helper = new(alloc) Supervisor_helper(*this);
}

void Socket_supervisor::supervise(socket_entry &entry) {
    auto node = new(_md_alloc) Socket_md_node(entry);
    _sockets.insert(node);
    debug_log(SOCKET_SUPERVISOR_DEBUG, "Socket_supervisor now watches ", entry);
}

void Socket_supervisor::abandon(Ptcp::Pfd &fd) {
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

constexpr int DS_SIZE = 2 * 1024;

void Socket_supervisor::dump(std::ostream &out) {
    /*
     * While dumping we need
     * 1) Block all outgoing sends. To do this I simply lock mutex used by my VFS plugin
     * 2) Block all incoming packets. This is not that simple, as I do not want them to be ACKed by TCP.
     *    To overcome this, I created proxy NIC component that stops submitting packets to clients after suspend() call
     * 3) For each socket read a local-stored state
     * 4) For sockets in ESTABLISHED state that arise from accept() call we should also know
     * initiator packets and last known seq/ack numbers.
     */
    Genode::Mutex::Guard _(Ptcp::mutex);
    _conn.suspend();
    debug_log(SOCKET_SUPERVISOR_SAVE_LOG, "socket_supervisor: Dumping...");

    int count = 0;
    _sockets.for_each([&](const Socket_md_node &) {
        count++;
    });
    out << count << "\n";

    // Dataspaces for future RPC calls
    auto ds = _env.ram().alloc(DS_SIZE);
    auto ackDs = _env.ram().alloc(DS_SIZE);

    _sockets.for_each([&](const Socket_md_node &node) {

        int ethLen = 0;
        int ackLen = 0;
        char *ethBuf = nullptr;
        char *ackBuf = nullptr;

        if (node._entry.tcpState == ESTABLISHED) { // Ask Nic trickster for actual SEQ/ACKs and initiator packets
            Net::Ipv4_address addr;
            Net::ascii_to(node._entry.remoteAddress, addr);

            std::string str1(node._entry.boundAddress);
            std::string port_str1 = str1.substr(str1.find(":") + 1);
            Genode::uint16_t local_port = atoi(port_str1.c_str());

            std::string str2(node._entry.remoteAddress);
            std::string port_str2 = str2.substr(str2.find(":") + 1);
            Genode::uint16_t remote_port = atoi(port_str2.c_str());

            Tracker::Nic_socket_id id{local_port, addr, remote_port};

            Genode::addr_t ds_attach_addr = _env.rm().attach(ds);
            Genode::addr_t ack_attach_addr = _env.rm().attach(ackDs);

            log(__func__, " looked up id ", id._remote, " ", id._remote_port, " ", id._local_port, " ");

            Nic_socket_metadata nic_md = _conn.get_md_value(id, ds, ackDs);

            ethLen = nic_md._eth_size;
            ethBuf = new char[ethLen];
            Genode::memcpy(ethBuf, (void *) ds_attach_addr, ethLen);

            Genode::log("\033[95m(", nic_md._eth_size, *(Net::Ethernet_frame *) ethBuf, ")\033[0m ");

            ackLen = nic_md._ack_size;
            ackBuf = new char[ackLen];
            Genode::memcpy(ackBuf, (void *) ack_attach_addr, ackLen);

            Genode::log("\033[95m(", nic_md._ack_size, *(Net::Ethernet_frame *) ackBuf, ")\033[0m ");
        }

        auto entry = node._entry;
        std::string str(entry.boundAddress);
        serialized_socket sock{
                entry.ptcpId.id,
                entry.tcpState,
                entry.boundAddress,
                entry.remoteAddress,
                ethLen, ethBuf,
                ackLen, ackBuf
        };
        sock.save(out);
    });

    _env.ram().free(ds);
    _env.ram().free(ackDs);

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