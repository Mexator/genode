#ifndef _PTCP_SOCKET_SUPERVISOR_H_
#define _PTCP_SOCKET_SUPERVISOR_H_

// Genode includes
#include <util/reconstructible.h>
#include <util/avl_tree.h>

// Ptcp includes
#include <ptcp_client/socket_state.h>
#include <ptcp_client/fd_proxy.h>

// Debug includes
#include <logging/mylog.h>

#ifndef SOCKET_SUPERVISOR_DEBUG
#define SOCKET_SUPERVISOR_DEBUG false
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
        bool higher(Socket_md_node* node) {
            return _entry.ptcpId.id > node->_entry.ptcpId.id;
        }
    };

    Genode::Allocator &_md_alloc;
    Genode::Avl_tree<Socket_md_node> _sockets;

public:
    explicit Socket_supervisor(Genode::Allocator &alloc);

    void supervise(socket_entry &entry);

    void abandon(Ptcp::Fd_proxy::Pfd &fd);
};

extern Socket_supervisor *socket_supervisor;

#endif //_PTCP_SOCKET_SUPERVISOR_H_
