#ifndef _PTCP_PROXY_FS_H_
#define _PTCP_PROXY_FS_H_

#include <vfs/file_system.h>

#ifndef PROXY_FS_DBG
#define PROXY_FS_DBG false
#endif

namespace Vfs {
    class Proxy_fs;

    class Proxy_handle;
    class Proxy_watch_handle;
}

class Vfs::Proxy_handle : public Vfs::Vfs_handle, private Io_response_handler {
    Vfs_handle &_real_handle;
public:
    Vfs_handle &actual() { return _real_handle; }

    Proxy_handle(Vfs_handle &real_handle,
                 Directory_service &ds,
                 File_io_service &fs,
                 Genode::Allocator &alloc,
                 int status_flags) : Vfs_handle(ds, fs, alloc, status_flags), _real_handle(real_handle) {
        _real_handle.handler(this);
    }

private:
    void read_ready_response() override {
        Vfs_handle::read_ready_response();
    }

    void io_progress_response() override {
        Vfs_handle::io_progress_response();
    }
};

class Vfs::Proxy_watch_handle : public Vfs::Vfs_watch_handle, private Vfs::Watch_response_handler {
    Vfs_watch_handle &_real_handle;
public:
    Vfs_watch_handle &actual() { return _real_handle; }

    Proxy_watch_handle(Vfs_watch_handle &real_handle,
                       Directory_service &ds,
                       Genode::Allocator &alloc) : Vfs_watch_handle(ds, alloc), _real_handle(real_handle) {
        _real_handle.handler(this);
    }

private:
    void watch_response() override {
        Vfs_watch_handle::watch_response();
    }
};

/**
 * File system that delegates all calls to another filesystem
 * Calls to the backing fs are executed under mutex
 */
class Vfs::Proxy_fs : public Vfs::File_system {
private:
    Vfs::File_system &_fs;
    Genode::Mutex &_mutex;
public:
    explicit Proxy_fs(Vfs::File_system &fs, Genode::Mutex &mutex) : _fs{fs}, _mutex(mutex) {}

    Stat_result stat(const char *path, Stat &) override;

    Dataspace_capability dataspace(const char *path) override;

    void release(const char *path, Dataspace_capability capability) override;

    Open_result open(const char *path, unsigned int mode, Vfs_handle **handle, Allocator &alloc) override;

    Opendir_result opendir(const char *string, bool b, Vfs_handle **pHandle, Allocator &allocator) override;

    Openlink_result openlink(const char *string, bool b, Vfs_handle **pHandle, Allocator &allocator) override;

    void close(Vfs_handle *handle) override;

    Watch_result watch(const char *path, Vfs_watch_handle **pHandle, Allocator &allocator) override;

    void close(Vfs_watch_handle *handle) override;

    Unlink_result unlink(const char *path) override;

    Rename_result rename(const char *from, const char *to) override;

    file_size num_dirent(const char *path) override;

    bool directory(const char *path) override;

    const char *leaf_path(const char *path) override;

    Write_result write(Vfs_handle *vfs_handle, const char *buf, file_size buf_size, file_size &out_count) override;

    bool queue_read(Vfs_handle *handle, file_size size) override;

    Read_result complete_read(Vfs_handle *handle, char *string, file_size size, file_size &fileSize) override;

    bool read_ready(Vfs_handle *handle) override;

    bool notify_read_ready(Vfs_handle *handle) override;

    Ftruncate_result ftruncate(Vfs_handle *vfs_handle, file_size len) override;

    Ioctl_result ioctl(Vfs_handle *handle, Ioctl_opcode opcode, Ioctl_arg arg, Ioctl_out &out) override;

    void register_read_ready_sigh(Vfs_handle *handle, Signal_context_capability capability) override;

    bool queue_sync(Vfs_handle *handle) override;

    Sync_result complete_sync(Vfs_handle *handle) override;

    bool update_modification_timestamp(Vfs_handle *handle, Vfs::Timestamp timestamp) override;

    void apply_config(const Xml_node &node) override;

    const char *type() override;
};

#endif //_PTCP_PROXY_FS_H_
