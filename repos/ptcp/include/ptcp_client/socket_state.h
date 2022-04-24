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

/**
 * Runtime socket tracking metadata
 */
struct socket_entry {

    ptcp_id ptcpId; // id in fd_proxy

    socket_entry();
    void print(Genode::Output &out) const;
};

#endif //_PTCP_SOCKET_STATE_H_
