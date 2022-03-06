#ifndef _LWIP_WRAPPER_H_
#define _LWIP_WRAPPER_H_

// VFS includes
#include <vfs/env.h>

// Util VFSs
#include <vfs_collection/proxy_fs.h>
#include <vfs_collection/buffer_fs.h>

namespace Ptcp {
    struct Vfs_wrapper;
}

class Ptcp::Vfs_wrapper : public Vfs::Proxy_fs {
private:
    Vfs::Env &_env;

    /**
     * Stores mapping between old libc fds and new ones
     */
    Buffer_file_system *readonly_fs = nullptr;

    Buffer_file_system *get_readonly_fs() {
        while (readonly_fs == nullptr) {}
        return readonly_fs;
    };

public:
    Vfs_wrapper(Vfs::Env &env, File_system &fs) : Vfs::Proxy_fs(fs), _env(env) {}

    void initialize_readonly_fs(char *content, Genode::size_t size) {
        readonly_fs = new(_env.alloc()) Buffer_file_system(content, size, "fd_mapping");
    }

    Open_result open(const char *path, unsigned int mode, Vfs::Vfs_handle **handle, Genode::Allocator &alloc) override {
        Open_result a = get_readonly_fs()->open(path, mode, handle, alloc);
        if (a == OPEN_OK) return a;
        return Proxy_fs::open(path, mode, handle, alloc);
    }

    Stat_result stat(const char *path, Stat &stat) override {
        Stat_result a = get_readonly_fs()->stat(path, stat);
        if (a == STAT_OK) return a;
        return Proxy_fs::stat(path, stat);
    }

    const char *type() override {
        return "ptcp";
    }
};

#endif //_LWIP_WRAPPER_H_
