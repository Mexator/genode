#ifndef _BUFFER_FS_H_
#define _BUFFER_FS_H_

// Genode includes
#include <util/string.h>

// VFS includes
#include <vfs/single_file_system.h>

using Genode::strlen;
using Genode::strcmp;

namespace Vfs {
    class Buffer_fs;
}

class Buffer_file_system : public Vfs::Single_file_system {
private:
    char const *_path;
    char const *const _content;
    Vfs::file_size const _size;

    /*
    * Noncopyable
    */
    Buffer_file_system(Buffer_file_system const &);

    Buffer_file_system &operator=(Buffer_file_system const &);

    class Buffer_file_handle : public Single_vfs_handle {
    private:

        char const *const _content;
        Vfs::file_size const _size;

        /*
         * Noncopyable
         */
        Buffer_file_handle(Buffer_file_handle const &);

        Buffer_file_handle &operator=(Buffer_file_handle const &);

    public:

        Buffer_file_handle(Directory_service &ds,
                           File_io_service &fs,
                           Genode::Allocator &alloc,
                           char const *const base,
                           Vfs::file_size const size)
                : Single_vfs_handle(ds, fs, alloc, 0),
                  _content(base), _size(size) {}

        Read_result read(char *dst, Vfs::file_size count,
                         Vfs::file_size &out_count) override;

        Write_result write(char const *, Vfs::file_size,
                           Vfs::file_size &out_count) override;

        bool read_ready() override { return true; }
    };

public:
    Buffer_file_system(const char *content, Vfs::file_size size, const char *path) :
            Single_file_system(Vfs::Node_type::CONTINUOUS_FILE, path,
                               Vfs::Node_rwx::ro(), Genode::Xml_node("<a/>")
            ), _path{path}, _content{content}, _size{size} {

    };

    char const *type() override { return "buffer"; }

    Open_result open(char const *path, unsigned,
                     Vfs::Vfs_handle **out_handle,
                     Vfs::Allocator &alloc) override;

    Stat_result stat(char const *path, Stat &out) override;
};

#endif //_BUFFER_FS_H_
