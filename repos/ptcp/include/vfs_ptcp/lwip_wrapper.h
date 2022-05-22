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

enum class File_handle_type {
    NEW_SOCKET, ACCEPT, BIND, LISTEN,
    UNKNOWN = ~0L
};

File_handle_type type_by_path(const char *path) {
    Genode::Path<Vfs::MAX_PATH_LEN> gpath(path);
    if (gpath == "/tcp/new_socket") return File_handle_type::NEW_SOCKET;
    if (0 == Genode::strcmp(gpath.last_element(), "accept_socket")) return File_handle_type::ACCEPT;
    if (0 == Genode::strcmp(gpath.last_element(), "bind")) return File_handle_type::BIND;
    if (0 == Genode::strcmp(gpath.last_element(), "listen")) return File_handle_type::LISTEN;
    return File_handle_type::UNKNOWN;
}

struct Supervision_delegate {
    Genode::Env &_env;
    Genode::Allocator &_alloc;
    Vfs::File_system &_fs;

    Supervision_delegate(Genode::Env &env, Genode::Allocator &alloc, Vfs::File_system &fs) : _env(env),
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
        File_handle_type handle_type = type_by_path(path);

        switch (handle_type) {
            case File_handle_type::NEW_SOCKET: {
                // Create new socket entry
                socket_entry *sock = new(_alloc) socket_entry(handle, false);
                // Read socket path
                auto result = _fs.complete_read(&handle, sock->socketPath.base() + 1, Vfs::MAX_PATH_LEN, sock->pathLen);
                if (result != Vfs::File_io_service::READ_OK) {
                    Genode::warning("New socket: path read with error");
                }
                // set_pending_entry may block. Execute in another thread to avoid deadlock
                (new(_alloc) Submitter_thread(_env, *sock))->start();
                break;
            }
            case File_handle_type::ACCEPT: {
                socket_entry *entry = supervisor_helper->get_entry_for(path);
                // Create new socket entry
                socket_entry *sock = new(_alloc) socket_entry(handle, true);
                // Inherit bound address. [entry] should be the listen entry
                Genode::copy_cstring((char *) &sock->boundAddress, entry->boundAddress, sizeof(entry->boundAddress));
                // set_pending_entry may block. Execute in another thread to avoid deadlock
                (new(_alloc) Submitter_thread(_env, *sock))->start();
                break;
            }
            case File_handle_type::BIND: {
                socket_entry *entry = supervisor_helper->get_entry_for(path);
                if (entry == nullptr) {
                    debug_log(VFS_LOG_DEBUG, "handle with ", path, " has no parent dir");
                    return;
                }
                entry->_bind_handle = &handle;
                debug_log(VFS_LOG_DEBUG, "bind handle ", path, " assigned");
                break;
            }
            case File_handle_type::LISTEN: {
                socket_entry *entry = supervisor_helper->get_entry_for(path);
                if (entry == nullptr) {
                    debug_log(VFS_LOG_DEBUG, "handle with ", path, " has no parent dir");
                    return;
                }
                entry->_listen_handle = &handle;
                debug_log(VFS_LOG_DEBUG, "listen handle ", path, " assigned");
                break;
            }
            case File_handle_type::UNKNOWN:
                break;
        }
    }

    void on_write(Vfs_handle *vfs_handle, const char *buf, file_size buf_size) {
        socket_entry *entry = supervisor_helper->get_entry_for(*vfs_handle);
        if (entry == nullptr) return;
        if (vfs_handle == entry->_bind_handle) {
            Genode::copy_cstring((char *) &entry->boundAddress, buf, buf_size + 1);
            entry->tcpState = std::max(BOUND, entry->tcpState);
            return;
        }
        if (vfs_handle == entry->_listen_handle) {
            debug_log(VFS_LOG_DEBUG, "listen handle write logged");
            entry->tcpState = LISTEN;
            return;
        }
        debug_log(VFS_LOG_DEBUG, "unmatched handle ", vfs_handle);
    }

    void on_complete_read(Vfs_handle *handle, const char *string, int read) {
        socket_entry *entry = supervisor_helper->get_entry_for(*handle);

        if (entry && &entry->_handle == handle && entry->_accepted) {
            // Fill in the accepted socket path and remote address.

            // At this point the connection is established but client does not know yet
            Genode::copy_cstring(entry->socketPath.base() + 1, string, read);
            entry->pathLen = read;
            debug_log(VFS_LOG_DEBUG, read, " accepted socket assigned path: ", (const char *) string);

            // Read remote address
            Genode::Path<Vfs::MAX_PATH_LEN> gpath(entry->socketPath);
            gpath.append_element("remote");
            Vfs_handle *h;
            auto result = _fs.open(gpath.string(), 0, &h, _alloc);
            if (result != Vfs::Directory_service::OPEN_OK) {
                Genode::error("opening", gpath, " in ", __func__, " failed");
                return;
            }
            file_size addr_read_len = 0;
            _fs.complete_read(h, entry->remoteAddress, MAX_ADDR_LEN, addr_read_len);
            entry->remoteAddress[addr_read_len - 1] = '\0'; // Remove trailing newline
            debug_log(VFS_LOG_DEBUG, "accepted socket given remote address: ", (const char *) entry->remoteAddress);
            entry->tcpState = ESTABLISHED;
        }
    }
};

class Ptcp::Vfs_wrapper : public Proxy_fs {
    Supervision_delegate delegate;
public:
    Vfs_wrapper(Env &env, File_system &lwip_fs) : Proxy_fs(lwip_fs, Ptcp::mutex),
                                                  delegate(env.env(), env.alloc(), *this) {}

    Open_result open(const char *path, unsigned int mode, Vfs_handle **handle, Genode::Allocator &alloc) override {
        debug_log(VFS_LOG_DEBUG, "Opening ", path);
        auto result = Proxy_fs::open(path, mode, handle, alloc);
        debug_log(VFS_LOG_DEBUG, "Opened ", path, "; handle address ", *handle);

        if (result == OPEN_OK)
            delegate.on_open(path, **handle);

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

    Read_result complete_read(Vfs_handle *handle, char *string, file_size size, file_size &outCount) override {
        auto result = Proxy_fs::complete_read(handle, string, size, outCount);
        debug_log(VFS_LOG_DEBUG, __func__, " ", handle);

        if (result == READ_OK)
            delegate.on_complete_read(handle, string, outCount);

        return result;
    }

    const char *type() override {
        return "ptcp";
    }
};

#endif //_PTCP_LWIP_WRAPPER_H_
