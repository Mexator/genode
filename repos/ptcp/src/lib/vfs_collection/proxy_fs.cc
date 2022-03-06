#include <vfs_collection/proxy_fs.h>

Vfs::Proxy_fs::Proxy_fs(Vfs::File_system &fs) : _fs{fs} {}

Vfs::Directory_service::Stat_result Vfs::Proxy_fs::stat(const char *path, Stat &st) {
    return _fs.stat(path, st);
}

Genode::Dataspace_capability Vfs::Proxy_fs::dataspace(const char *path) {
    return _fs.dataspace(path);
}

void Vfs::Proxy_fs::release(const char *path, Genode::Dataspace_capability capability) {
    return _fs.release(path, capability);
}

Vfs::Directory_service::Open_result
Vfs::Proxy_fs::open(const char *path, unsigned int mode, Vfs::Vfs_handle **handle, Genode::Allocator &alloc) {
    return _fs.open(path, mode, handle, alloc);
}

Vfs::Directory_service::Opendir_result
Vfs::Proxy_fs::opendir(const char *string, bool b, Vfs::Vfs_handle **pHandle, Genode::Allocator &allocator) {
    return _fs.opendir(string, b, pHandle, allocator);
}

Vfs::Directory_service::Openlink_result
Vfs::Proxy_fs::openlink(const char *string, bool b, Vfs::Vfs_handle **pHandle, Genode::Allocator &allocator) {
    return _fs.openlink(string, b, pHandle, allocator);
}

void Vfs::Proxy_fs::close(Vfs::Vfs_handle *handle) {
    _fs.close(handle);
}

Vfs::Directory_service::Watch_result
Vfs::Proxy_fs::watch(const char *path, Vfs::Vfs_watch_handle **pHandle, Genode::Allocator &allocator) {
    return _fs.watch(path, pHandle, allocator);
}

void Vfs::Proxy_fs::close(Vfs::Vfs_watch_handle *handle) {
    _fs.close(handle);
}

Vfs::Directory_service::Unlink_result Vfs::Proxy_fs::unlink(const char *path) {
    return _fs.unlink(path);
}

Vfs::Directory_service::Rename_result Vfs::Proxy_fs::rename(const char *from, const char *to) {
    return _fs.rename(from, to);
}

Vfs::file_size Vfs::Proxy_fs::num_dirent(const char *path) {
    return _fs.num_dirent(path);
}

bool Vfs::Proxy_fs::directory(const char *path) {
    return _fs.directory(path);
}

const char *Vfs::Proxy_fs::leaf_path(const char *path) {
    return _fs.leaf_path(path);
}

Vfs::File_io_service::Write_result
Vfs::Proxy_fs::write(Vfs::Vfs_handle *vfs_handle, const char *buf, Vfs::file_size buf_size, Vfs::file_size &out_count) {
    return _fs.write(vfs_handle, buf, buf_size, out_count);
}

bool Vfs::Proxy_fs::queue_read(Vfs::Vfs_handle *handle, Vfs::file_size size) {
    return _fs.queue_read(handle, size);
}

Vfs::File_io_service::Read_result
Vfs::Proxy_fs::complete_read(Vfs::Vfs_handle *handle, char *string, Vfs::file_size size, Vfs::file_size &fileSize) {
    return _fs.complete_read(handle, string, size, fileSize);
}

bool Vfs::Proxy_fs::read_ready(Vfs::Vfs_handle *handle) {
    return _fs.read_ready(handle);
}

bool Vfs::Proxy_fs::notify_read_ready(Vfs::Vfs_handle *handle) {
    return _fs.notify_read_ready(handle);
}

Vfs::File_io_service::Ftruncate_result Vfs::Proxy_fs::ftruncate(Vfs::Vfs_handle *vfs_handle, Vfs::file_size len) {
    return _fs.ftruncate(vfs_handle, len);
}

Vfs::File_io_service::Ioctl_result
Vfs::Proxy_fs::ioctl(Vfs::Vfs_handle *handle, Vfs::File_io_service::Ioctl_opcode opcode,
                     Vfs::File_io_service::Ioctl_arg arg, Vfs::File_io_service::Ioctl_out &out) {
    return _fs.ioctl(handle, opcode, arg, out);
}

bool Vfs::Proxy_fs::check_unblock(Vfs::Vfs_handle *handle, bool b, bool b1, bool b2) {
    return _fs.check_unblock(handle, b, b1, b2);
}

void Vfs::Proxy_fs::register_read_ready_sigh(Vfs::Vfs_handle *handle, Genode::Signal_context_capability capability) {
    _fs.register_read_ready_sigh(handle, capability);
}

bool Vfs::Proxy_fs::queue_sync(Vfs::Vfs_handle *handle) {
    return _fs.queue_sync(handle);
}

Vfs::File_io_service::Sync_result Vfs::Proxy_fs::complete_sync(Vfs::Vfs_handle *handle) {
    return _fs.complete_sync(handle);
}

bool Vfs::Proxy_fs::update_modification_timestamp(Vfs::Vfs_handle *handle, Vfs::Timestamp timestamp) {
    return _fs.update_modification_timestamp(handle, timestamp);
}

void Vfs::Proxy_fs::apply_config(const Genode::Xml_node &node) {
    _fs.apply_config(node);
}

const char *Vfs::Proxy_fs::type() {
    return _fs.type();
}
