#ifndef _PTCP_LWIP_WRAPPER_H_
#define _PTCP_LWIP_WRAPPER_H_

// VFS includes
#include <vfs/env.h>

// Util VFSs
#include <vfs_collection/proxy_fs.h>

namespace Ptcp {
    struct Vfs_wrapper;
}

class Ptcp::Vfs_wrapper : public Vfs::Proxy_fs {
private:
    Vfs::Env &_env;

    bool initialized = false;
    Ptcp::Snapshot::Load_manager load_manager;
    Genode::Mutex _m;

public:
    Vfs_wrapper(Vfs::Env &env, File_system &lwip_fs, Ptcp::Snapshot::Load_manager load_manager)
            : Vfs::Proxy_fs(lwip_fs), _env(env), load_manager(load_manager) {}

    Stat_result stat(const char *path, Stat &stat) override {
        // XXX
        // libc state can be restored only after libc is initialized.
        // I have no idea how to detect this from within VFS plugin. Only solution I came up with
        // is to wait until app accesses any file in plugin directory.

        // Perform double check to do init AND avoid deadlock
        if (!initialized) {
            _m.acquire();
            if (!initialized) {
                initialized = true;
                // Loading state
                load_manager.read_snapshot_file();
                load_manager.restore_state();
            }
            _m.release();
        }
        return Proxy_fs::stat(path, stat);
    }

    const char *type() override {
        return "ptcp";
    }
};

#endif //_PTCP_LWIP_WRAPPER_H_
