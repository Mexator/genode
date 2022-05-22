#ifndef _PTCP_SOCKET_STATE_H_
#define _PTCP_SOCKET_STATE_H_

// libc includes
#include <netdb.h>

// Vfs includes
#include <vfs/vfs_handle.h>

struct ptcp_id {
    bool known;
    unsigned long id;
};

using namespace Vfs;

enum tcp_meta_state {
    CLOSED = 0,
    BOUND = 1,
    LISTEN = 2,
    ESTABLISHED = 3,
};

constexpr int MAX_ADDR_LEN = NI_MAXHOST + NI_MAXSERV;

/**
 * Runtime socket tracking metadata
 */
struct socket_entry {
    Vfs_handle &_handle; // socket dir handle
    bool _accepted;

    Vfs_handle *_bind_handle = nullptr;
    Vfs_handle *_listen_handle = nullptr;
    ptcp_id ptcpId; // id in fd_proxy

    Genode::Path<MAX_PATH_LEN> socketPath; // path to socket dir
    file_size pathLen;

    char boundAddress[MAX_ADDR_LEN];
    char remoteAddress[MAX_ADDR_LEN];
    tcp_meta_state tcpState;

    socket_entry(Vfs_handle &handle, bool accepted);

    void print(Genode::Output &out) const;

    bool belongs_to_this(Vfs_handle &handle);
};

#endif //_PTCP_SOCKET_STATE_H_
