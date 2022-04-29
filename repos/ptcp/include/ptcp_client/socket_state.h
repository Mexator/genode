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

/**
 * Runtime socket tracking metadata
 */
struct socket_entry {
    Vfs_handle &_handle; // socket dir handle
    Vfs_handle *_bind_handle;
    ptcp_id ptcpId; // id in fd_proxy

    Genode::Path<MAX_PATH_LEN> socketPath; // path to socket dir
    file_size pathLen;

    char boundAddress[NI_MAXHOST + NI_MAXSERV];

    socket_entry(Vfs_handle &handle);

    void print(Genode::Output &out) const;

    bool belongs_to_this(Vfs_handle &handle);
};

#endif //_PTCP_SOCKET_STATE_H_
