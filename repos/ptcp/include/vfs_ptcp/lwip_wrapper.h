#ifndef _PTCP_LWIP_WRAPPER_H_
#define _PTCP_LWIP_WRAPPER_H_

// VFS includes
#include <vfs/env.h>

// PTCP includes
#include <vfs_ptcp/proxy_fs.h>

namespace Ptcp {
    struct Vfs_wrapper;
}

class Ptcp::Vfs_wrapper : public Vfs::Proxy_fs {
public:
    Vfs_wrapper(Vfs::Env &env, File_system &lwip_fs) : Vfs::Proxy_fs(lwip_fs) {}

    Open_result open(const char *path, unsigned int mode, Vfs::Vfs_handle **handle, Genode::Allocator &alloc) override {
        auto result = Proxy_fs::open(path, mode, handle, alloc);
        Genode::log("Opened ", path, "; handle address ", *handle);
        return result;
    }

    void close(Vfs::Vfs_handle *handle) override {
        Genode::log("ptcp Closed ", handle);

        Proxy_fs::close(handle);
    }

    const char *type() override {
        return "ptcp";
    }
};

#endif //_PTCP_LWIP_WRAPPER_H_
