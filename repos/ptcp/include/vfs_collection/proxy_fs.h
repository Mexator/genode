#ifndef _PROXY_FS_H_
#define _PROXY_FS_H_

#include <vfs/file_system.h>

namespace Vfs {
    class Proxy_fs;
}

/**
 * File system that delegates all calls to another filesystem
 */
class Vfs::Proxy_fs : public Vfs::File_system {
private:
    Vfs::File_system &_fs;
public:
    explicit Proxy_fs(Vfs::File_system &fs);

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

    bool check_unblock(Vfs_handle *handle, bool b, bool b1, bool b2) override;

    void register_read_ready_sigh(Vfs_handle *handle, Signal_context_capability capability) override;

    bool queue_sync(Vfs_handle *handle) override;

    Sync_result complete_sync(Vfs_handle *handle) override;

    bool update_modification_timestamp(Vfs_handle *handle, Vfs::Timestamp timestamp) override;

    void apply_config(const Xml_node &node) override;

    const char *type() override;
};

#endif //_PROXY_FS_H_
