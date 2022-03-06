#include <vfs_collection/buffer_fs.h>

Buffer_file_system::Read_result
Buffer_file_system::Buffer_file_handle::read(char *dst, Vfs::file_size count, Vfs::file_size &out_count) {
    /* file read limit is the size of the dataspace */
    Vfs::file_size const max_size = _size;

    /* current read offset */
    Vfs::file_size const read_offset = seek();

    /* maximum read offset, clamped to dataspace size */
    Vfs::file_size const end_offset = Genode::min(count + read_offset, max_size);

    /* source address within the dataspace */
    char const *src = _content + read_offset;

    /* check if end of file is reached */
    if (read_offset >= end_offset) {
        out_count = 0;
        return READ_OK;
    }

    /* copy-out bytes from ROM dataspace */
    Genode::size_t const num_bytes = (Genode::size_t) (end_offset - read_offset);

    Genode::memcpy(dst, src, num_bytes);

    out_count = num_bytes;
    return READ_OK;
}

Buffer_file_system::Write_result
Buffer_file_system::Buffer_file_handle::write(const char *, Vfs::file_size, Vfs::file_size &out_count) {
    out_count = 0;
    return WRITE_ERR_INVALID;
}

inline bool compare_paths(char const *incoming, char const *saved) {
    return (strlen(incoming) == (strlen(saved) + 1)) &&
           (strcmp(&incoming[1], saved) == 0);
}

Buffer_file_system::Stat_result Buffer_file_system::stat(const char *path, Stat &out) {
    Stat_result const result = Single_file_system::stat(path, out);
    if (compare_paths(path, _path)) {
        out.size = _size;
    }
    return result;
}

Buffer_file_system::Open_result
Buffer_file_system::open(const char *path, unsigned int, Vfs::Vfs_handle **out_handle, Vfs::Allocator &alloc) {
    if (compare_paths(path, _path)) {
        *out_handle = new(alloc)
                Buffer_file_handle(*this, *this, alloc, _content, _size);
        return OPEN_OK;
    }
    return OPEN_ERR_UNACCESSIBLE;
}