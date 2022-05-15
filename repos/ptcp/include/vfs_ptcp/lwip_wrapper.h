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

using namespace Vfs;

struct Supervision_delegate {
    Genode::Env &_env;
    Genode::Allocator &_alloc;
    Vfs::File_io_service &_fs;

    Supervision_delegate(Genode::Env &env, Genode::Allocator &alloc, Vfs::File_io_service &fs) : _env(env),
                                                                                                 _alloc(alloc),
                                                                                                 _fs(fs) {}

    class Submitter_thread : public Genode::Thread {
    private:
        socket_entry &_sock;
    public:
        Submitter_thread(Genode::Env &env, socket_entry &sock) :
                Genode::Thread(env, "submitter", 0x400),
                _sock(sock) {}

        void entry() override {
            supervisor_helper->set_pending_entry(_sock);
        }
    };

    void on_open(const char *path, Vfs_handle &handle) {
        Genode::Path<Vfs::MAX_PATH_LEN> gpath(path);

        { // Process opening of new sockets
            if (gpath == "/tcp/new_socket" ||
                (0 == Genode::strcmp(gpath.last_element(), "accept_socket"))) {
                // Create new socket entry
                socket_entry *sock = new(_alloc) socket_entry(handle);
                // Read socket path
                _fs.complete_read(&handle, sock->socketPath.base() + 1, Vfs::MAX_PATH_LEN, sock->pathLen);
                // set_pending_entry may block. Execute in another thread to avoid deadlock
                (new(_alloc) Submitter_thread(_env, *sock))->start();
                return;
            }
        }

        { // Add child handles to socket metadata
            socket_entry *entry = supervisor_helper->get_entry_for(path);
            if (entry == nullptr) {
                debug_log(VFS_LOG_DEBUG, "handle with ", path, " has no parent dir");
                return;
            }

            if (0 == Genode::strcmp(gpath.last_element(), "bind")) {
                entry->_bind_handle = &handle;
                debug_log(VFS_LOG_DEBUG, "bind handle ", path, " assigned");
            }

            if (0 == Genode::strcmp(gpath.last_element(), "listen")) {
                entry->_listen_handle = &handle;
                debug_log(VFS_LOG_DEBUG, "listen handle ", path, " assigned");
            }
        }
    }

    void on_write(Vfs_handle *vfs_handle, const char *buf, file_size buf_size) {
        socket_entry *entry = supervisor_helper->get_entry_for(*vfs_handle);
        if (entry == nullptr) return;
        if (vfs_handle == entry->_bind_handle) {
            Genode::copy_cstring((char *) &entry->boundAddress, buf, buf_size + 1);
            return;
        }
        if (vfs_handle == entry->_listen_handle) {
            debug_log(VFS_LOG_DEBUG, "listen handle write logged");
            entry->tcpState = LISTEN;
            return;
        }
        debug_log(VFS_LOG_DEBUG, "unmatched handle ", vfs_handle);
    }
};

class Ptcp::Vfs_wrapper : public Proxy_fs {
    Supervision_delegate delegate;
public:
    Vfs_wrapper(Env &env, File_system &lwip_fs) : Proxy_fs(lwip_fs, Ptcp::mutex),
                                                  delegate(env.env(), env.alloc(), *this) {}

    Open_result open(const char *path, unsigned int mode, Vfs_handle **handle, Genode::Allocator &alloc) override {
        auto result = Proxy_fs::open(path, mode, handle, alloc);
        delegate.on_open(path, **handle);

        debug_log(VFS_LOG_DEBUG, "Opened ", path, "; handle address ", *handle);
        return result;
    }

    void close(Vfs_handle *handle) override {
        debug_log(VFS_LOG_DEBUG, "ptcp Closed ", handle);

        Proxy_fs::close(handle);
    }

    Write_result write(Vfs_handle *vfs_handle, const char *buf, file_size buf_size, file_size &out_count) override {
        debug_log(VFS_LOG_DEBUG, "write to ", vfs_handle);
        auto result = Proxy_fs::write(vfs_handle, buf, buf_size, out_count);

        if (result == WRITE_OK)
            delegate.on_write(vfs_handle, buf, buf_size);

        return result;
    }

    const char *type() override {
        return "ptcp";
    }
};

#endif //_PTCP_LWIP_WRAPPER_H_
