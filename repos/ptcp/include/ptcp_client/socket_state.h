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

enum tcp_state {
    CLOSED      = 0,
    LISTEN      = 1,
// I am not sure which states I will handle later, so commented out unused ones
//    SYN_SENT    = 2,
//    SYN_RCVD    = 3,
//    ESTABLISHED = 4,
//    FIN_WAIT_1  = 5,
//    FIN_WAIT_2  = 6,
//    CLOSE_WAIT  = 7,
//    CLOSING     = 8,
//    LAST_ACK    = 9,
//    TIME_WAIT   = 10
};

/**
 * Runtime socket tracking metadata
 */
struct socket_entry {
    Vfs_handle &_handle; // socket dir handle
    Vfs_handle *_bind_handle;
    Vfs_handle *_listen_handle;
    ptcp_id ptcpId; // id in fd_proxy

    Genode::Path<MAX_PATH_LEN> socketPath; // path to socket dir
    file_size pathLen;

    char boundAddress[NI_MAXHOST + NI_MAXSERV];
    tcp_state tcpState;

    socket_entry(Vfs_handle &handle);

    void print(Genode::Output &out) const;

    bool belongs_to_this(Vfs_handle &handle);
};

#endif //_PTCP_SOCKET_STATE_H_
