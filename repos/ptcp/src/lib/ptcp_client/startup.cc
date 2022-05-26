// Libc includes
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <fstream>

// Ptcp includes
#include <ptcp_client/startup.h>
#include <ptcp_client/socket_supervisor.h>
#include <ptcp_client/serialized/serialized_socket_state.h>
#include <ptcp_client/fd_proxy.h>

using Genode::log;
using Genode::error;
using namespace Ptcp;

void init(Genode::Env &env, Genode::Allocator &alloc) {
    fd_proxy = new(alloc) Fd_proxy(alloc);

    Genode::log("Creating Nic_control connection...");
    Nic_control::Connection *conn = new(alloc) Nic_control::Connection(env);
    Genode::log("Nic_control Connected!");

    socket_supervisor = new(alloc) Socket_supervisor(alloc, env, *conn);
}

Pfd find_listening_parent(serialized_socket &entry, serialized_socket *items, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        auto item = items[i];
        Genode::warning(entry, item);
        if (item.state == LISTEN && Genode::strcmp(item.boundAddress, entry.boundAddress) == 0) {
            return Pfd{item.pfd};
        }
    }
    Genode::error("No listening parent found for ", entry);
    throw Genode::Exception();
}

void restore_sockets_state(Genode::Env &env, Nic_control::Connection &conn) {
    std::ifstream snapshot;
    snapshot.open("/snapshot/sockets");
    if (!snapshot.is_open()) {
        snapshot.close();
        Genode::warning("Can't open snapshot");
        return;
    }

    // Read
    int len = 0;
    snapshot >> len;
    if (len == 0) {
        snapshot.close();
        return;
    }
    serialized_socket entries[len];
    for (int i = 0; i < len; ++i) {
        entries[i] = serialized_socket::load(snapshot);
        entries[i].save(std::cout); // Load logging
    }

    // Reopen
    for (int i = 0; i < len; ++i) {
        Genode::warning(entries[i]);
        if (entries[i].state != ESTABLISHED) {
            // Established sockets will be recreated later with accept call
            int libc_fd = socket(AF_INET, SOCK_STREAM, 0);
            fd_proxy->set(libc_fd, entries[i].pfd);
        }
    }

    // Restore state
    for (int i = 0; i < len; ++i) {
        auto pfd = entries[i].pfd;
        int libc_fd = fd_proxy->map_fd(Pfd{pfd});
        if (entries[i].state == BOUND || entries[i].state == LISTEN) {
            Genode::warning("restoring bound socket ", pfd);

            std::string str(entries[i].boundAddress);
            std::string port_str = str.substr(str.find(":") + 1);

            struct sockaddr_in in_addr;
            in_addr.sin_family = AF_INET;

            inet_aton((const char *) str.c_str(), &in_addr.sin_addr);
            in_addr.sin_port = htons(atoi(port_str.c_str()));

            if (0 != bind(libc_fd, (struct sockaddr *) &in_addr, sizeof(in_addr))) {
                error("while calling bind() IN RESTORE, errno=", errno);
                return;
            }
            log("Socket bound IN RESTORE");
        }
        if (entries[i].state == LISTEN) {
            /* I don't know how it works, but even without a backlog connections
             * are still possible even if handshake was performed before an
             * accept call.
             *
             * So, basically we can set backlog to 0. */
            if (0 != listen(libc_fd, 0)) {
                error("while calling listen(), errno=", errno);
            } else {
                log("Socket listens IN RESTORE");
            }
        }
    }
    conn.set_restore_mode(true);

    int SIZE = 4096 * 1024;
    auto ds = env.ram().alloc(SIZE);
    Genode::addr_t ds_attach_addr = env.rm().attach(ds);

    for (int i = 0; i < len; ++i) {
        if (entries[i].state == ESTABLISHED) {

            Genode::memcpy((void *) ds_attach_addr, entries[i].syn_packet, entries[i].syn_packet_len);
            Genode::log(entries[i].syn_packet_len, *(Net::Ethernet_frame *) entries[i].syn_packet);
            Genode::log(entries[i].syn_packet_len, *(Net::Ethernet_frame *) ds_attach_addr);
            conn.send_packet(entries[i].syn_packet_len, ds);

            usleep(250000); // Yield execution to tcp stack so it send SYN/ACK packets

            Genode::memcpy((void *) ds_attach_addr, entries[i].ack_packet, entries[i].ack_packet_len);
            Genode::log(entries[i].syn_packet_len, *(Net::Ethernet_frame *) entries[i].ack_packet);
            Genode::log(entries[i].syn_packet_len, *(Net::Ethernet_frame *) ds_attach_addr);
            conn.send_packet(entries[i].ack_packet_len, ds);

            usleep(250000); // Yield execution to tcp stack so it receive ACK#3 packets

            Pfd parent = find_listening_parent(entries[i], entries, len);
            Genode::warning("Trying to accept, parent=", parent);
            int sock = accept(fd_proxy->map_fd(parent), nullptr, nullptr);
            Genode::warning("Accepted! ", sock);
            fd_proxy->set(sock, entries[i].pfd);
            write(fd_proxy->map_fd(Pfd{entries[i].pfd}), "IS THIS WORKS!!!!???", 21);
        }
    }
    conn.set_restore_mode(false);
    env.rm().detach(ds_attach_addr);
    snapshot.close();
}

[[noreturn]] void dump_loop() {
    while (true) {
        sleep(5);
        std::ofstream snapshot;
        snapshot.open("/snapshot/sockets");
        socket_supervisor->dump(snapshot);
        snapshot.close();
    }
}

void startup_callback(Genode::Env &env, Genode::Allocator &alloc) {
    // init controller classes
    init(env, alloc);
    // load state
    restore_sockets_state(env, socket_supervisor->_conn);
    // Start dump loop
    pthread_t dumper;
    pthread_create(&dumper, nullptr, (void *(*)(void *)) (dump_loop), nullptr);
}