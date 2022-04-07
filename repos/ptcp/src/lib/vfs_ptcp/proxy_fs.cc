#include <logging/mylog.h>
#include <vfs_ptcp/proxy_fs.h>

Vfs::Proxy_handle *cast_handle(Vfs::Vfs_handle *h) {
    return dynamic_cast<Vfs::Proxy_handle *>(h);
}

Vfs::Proxy_watch_handle *cast_handle(Vfs::Vfs_watch_handle *h) {
    return dynamic_cast<Vfs::Proxy_watch_handle *>(h);
}

Vfs::Directory_service::Stat_result Vfs::Proxy_fs::stat(const char *path, Stat &st) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.stat(path, st);
}

Genode::Dataspace_capability Vfs::Proxy_fs::dataspace(const char *path) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.dataspace(path);
}

void Vfs::Proxy_fs::release(const char *path, Genode::Dataspace_capability capability) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.release(path, capability);
}

Vfs::Directory_service::Open_result
Vfs::Proxy_fs::open(const char *path, unsigned int mode, Vfs::Vfs_handle **handle, Genode::Allocator &alloc) {
    debug_log(PROXY_FS_DBG, __func__);
    Vfs_handle *_handle = 0;

    auto result = _fs.open(path, mode, &_handle, alloc);
    *handle = new(alloc) Proxy_handle(
            *_handle,
            *this,
            *this,
            _handle->alloc(),
            _handle->status_flags()
    );
    return result;
}

Vfs::Directory_service::Opendir_result
Vfs::Proxy_fs::opendir(const char *string, bool b, Vfs::Vfs_handle **pHandle, Genode::Allocator &allocator) {
    debug_log(PROXY_FS_DBG, __func__);
    Vfs_handle *_handle = 0;

    auto result = _fs.opendir(string, b, &_handle, allocator);
    *pHandle = new(allocator) Proxy_handle(
            *_handle,
            *this,
            *this,
            _handle->alloc(),
            _handle->status_flags());
    return result;
}

Vfs::Directory_service::Openlink_result
Vfs::Proxy_fs::openlink(const char *string, bool b, Vfs::Vfs_handle **pHandle, Genode::Allocator &allocator) {
    debug_log(PROXY_FS_DBG, __func__);
    Vfs_handle *_handle = 0;
    auto result = _fs.openlink(string, b, &_handle, allocator);
    *pHandle = new(allocator) Proxy_handle(
            *_handle,
            *this,
            *this,
            _handle->alloc(),
            _handle->status_flags());
    return result;
}

void Vfs::Proxy_fs::close(Vfs::Vfs_handle *handle) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    _fs.close(&h->actual());
    Genode::destroy(h->alloc(), handle);
}

Vfs::Directory_service::Watch_result
Vfs::Proxy_fs::watch(const char *path, Vfs::Vfs_watch_handle **pHandle, Genode::Allocator &allocator) {
    debug_log(PROXY_FS_DBG, __func__);
    Vfs_watch_handle *_handle = 0;
    auto result = _fs.watch(path, &_handle, allocator);
    *pHandle = new(allocator) Proxy_watch_handle(
            *_handle,
            *this,
            _handle->alloc());
    return result;
}

void Vfs::Proxy_fs::close(Vfs::Vfs_watch_handle *handle) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    _fs.close(&h->actual());
    Genode::destroy(h->alloc(), handle);
}

Vfs::Directory_service::Unlink_result Vfs::Proxy_fs::unlink(const char *path) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.unlink(path);
}

Vfs::Directory_service::Rename_result Vfs::Proxy_fs::rename(const char *from, const char *to) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.rename(from, to);
}

Vfs::file_size Vfs::Proxy_fs::num_dirent(const char *path) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.num_dirent(path);
}

bool Vfs::Proxy_fs::directory(const char *path) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.directory(path);
}

const char *Vfs::Proxy_fs::leaf_path(const char *path) {
    debug_log(PROXY_FS_DBG, __func__);
    return _fs.leaf_path(path);
}

Vfs::File_io_service::Write_result
Vfs::Proxy_fs::write(Vfs::Vfs_handle *vfs_handle, const char *buf, Vfs::file_size buf_size, Vfs::file_size &out_count) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(vfs_handle);
    return _fs.write(&h->actual(), buf, buf_size, out_count);
}

bool Vfs::Proxy_fs::queue_read(Vfs::Vfs_handle *handle, Vfs::file_size size) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.queue_read(&h->actual(), size);
}

Vfs::File_io_service::Read_result
Vfs::Proxy_fs::complete_read(Vfs::Vfs_handle *handle, char *string, Vfs::file_size size, Vfs::file_size &fileSize) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.complete_read(&h->actual(), string, size, fileSize);
}

bool Vfs::Proxy_fs::read_ready(Vfs::Vfs_handle *handle) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.read_ready(&h->actual());
}

bool Vfs::Proxy_fs::notify_read_ready(Vfs::Vfs_handle *handle) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.notify_read_ready(&h->actual());
}

Vfs::File_io_service::Ftruncate_result Vfs::Proxy_fs::ftruncate(Vfs::Vfs_handle *vfs_handle, Vfs::file_size len) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(vfs_handle);
    return _fs.ftruncate(&h->actual(), len);
}

Vfs::File_io_service::Ioctl_result
Vfs::Proxy_fs::ioctl(Vfs::Vfs_handle *handle, Vfs::File_io_service::Ioctl_opcode opcode,
                     Vfs::File_io_service::Ioctl_arg arg, Vfs::File_io_service::Ioctl_out &out) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.ioctl(&h->actual(), opcode, arg, out);
}

void Vfs::Proxy_fs::register_read_ready_sigh(Vfs::Vfs_handle *handle, Genode::Signal_context_capability capability) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    _fs.register_read_ready_sigh(&h->actual(), capability);
}

bool Vfs::Proxy_fs::queue_sync(Vfs::Vfs_handle *handle) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.queue_sync(&h->actual());
}

Vfs::File_io_service::Sync_result Vfs::Proxy_fs::complete_sync(Vfs::Vfs_handle *handle) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.complete_sync(&h->actual());
}

bool Vfs::Proxy_fs::update_modification_timestamp(Vfs::Vfs_handle *handle, Vfs::Timestamp timestamp) {
    debug_log(PROXY_FS_DBG, __func__);
    auto h = cast_handle(handle);
    return _fs.update_modification_timestamp(&h->actual(), timestamp);
}

void Vfs::Proxy_fs::apply_config(const Genode::Xml_node &node) {
    _fs.apply_config(node);
}

const char *Vfs::Proxy_fs::type() {
    return _fs.type();
}
