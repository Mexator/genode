#ifndef _PTCP_LWIP_WRAPPER_H_
#define _PTCP_LWIP_WRAPPER_H_

// VFS includes
#include <vfs/env.h>
#include <vfs/types.h>

// PTCP includes
#include <vfs_ptcp/proxy_fs.h>
#include <ptcp_client/socket_state.h>
#include <ptcp_client/socket_supervisor.h>
#include <ptcp_client/supervisor_helper.h>
#include <ptcp_client/ptcp_lock.h>

// Debug includes
#include <logging/mylog.h>

#ifndef VFS_LOG_DEBUG
#define VFS_LOG_DEBUG false
#endif

namespace Ptcp {
    struct Vfs_wrapper;
}

struct Supervision_delegate {
    Genode::Allocator &_alloc;

    Supervision_delegate(Genode::Allocator &alloc) : _alloc(alloc) {}

    void on_open(const char *path) {
        Genode::Path<Vfs::MAX_PATH_LEN> gpath(path); // XXX is 128 enough? Maybe there is some constant with max path length

        socket_entry *sock = new(_alloc) socket_entry(path);
        if (gpath == "/tcp/new_socket") {
            supervisor_helper->set_pending_entry(*sock);
        }
        if (0 == Genode::strcmp(gpath.last_element(), "accept_socket")) {
            supervisor_helper->set_pending_entry(*sock);
        }
    }
};

class Ptcp::Vfs_wrapper : public Vfs::Proxy_fs {
    Supervision_delegate delegate;
public:
    Vfs_wrapper(Vfs::Env &env, File_system &lwip_fs) : Vfs::Proxy_fs(lwip_fs, Ptcp::mutex), delegate(env.alloc()) {}

    Open_result open(const char *path, unsigned int mode, Vfs::Vfs_handle **handle, Genode::Allocator &alloc) override {
        auto result = Proxy_fs::open(path, mode, handle, alloc);
        delegate.on_open(path);
        debug_log(VFS_LOG_DEBUG, "Opened ", path, "; handle address ", *handle);
        return result;
    }

    void close(Vfs::Vfs_handle *handle) override {
        debug_log(VFS_LOG_DEBUG, "ptcp Closed ", handle);

        Proxy_fs::close(handle);
    }

    const char *type() override {
        return "ptcp";
    }
};

#endif //_PTCP_LWIP_WRAPPER_H_
