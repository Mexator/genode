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

struct socket_entry {

    ptcp_id ptcpId; // id in fd_proxy
    const char *fsPath; // to track child VFS handles in open(call)

    socket_entry(const char *path);
    void print(Genode::Output &out) const;
};

#endif //_PTCP_SOCKET_STATE_H_
