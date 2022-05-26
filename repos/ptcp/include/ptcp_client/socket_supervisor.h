#ifndef _PTCP_SOCKET_SUPERVISOR_H_
#define _PTCP_SOCKET_SUPERVISOR_H_

// Genode includes
#include <util/avl_tree.h>

// Ptcp includes
#include <ptcp_client/socket_state.h>
#include <ptcp_client/fd_proxy.h>

// Nic control includes
#include <nic_trickster/control/session.h>

// Libcxx includes
#include <iostream>

#ifndef SOCKET_SUPERVISOR_DEBUG
#define SOCKET_SUPERVISOR_DEBUG false
#endif

#ifndef SOCKET_SUPERVISOR_SAVE_LOG
#define SOCKET_SUPERVISOR_SAVE_LOG true
#endif

/**
 * This class holds socket snapshot metadata
 */
class Socket_supervisor {
    friend class Supervisor_helper;

    class Socket_md_node : public Genode::Avl_node<Socket_md_node> {
    public:
        socket_entry &_entry;

        explicit Socket_md_node(socket_entry &entry) : _entry(entry) {}

        bool higher(Socket_md_node *node) {
            return _entry.ptcpId.id > node->_entry.ptcpId.id;
        }
    };

    Genode::Allocator &_md_alloc;
    Genode::Avl_tree<Socket_md_node> _sockets;
    Genode::Env &_env;

public:
    Nic_control::Connection &_conn;

    Socket_supervisor(
            Genode::Allocator &alloc,
            Genode::Env &env,
            Nic_control::Connection &conn
    );

    void supervise(socket_entry &entry);

    void abandon(Ptcp::Pfd &fd);

    void dump(std::ostream &out);

    socket_entry *get_entry_for(const char *path);

    socket_entry *get_entry_for(Vfs::Vfs_handle &handle);
};

extern Socket_supervisor *socket_supervisor;

#endif //_PTCP_SOCKET_SUPERVISOR_H_
