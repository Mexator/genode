#ifndef _PTCP_LOAD_H_
#define _PTCP_LOAD_H_

// Genode includes
#include <base/allocator.h>
#include <util/list.h>

namespace Ptcp {
    namespace Snapshot {
        class Load_manager;
    }
    class Vfs_wrapper;
}


class Ptcp::Snapshot::Load_manager {
public:
    struct Fd_map : Genode::List<Fd_map>::Element {
        int _old_fd;
        int _new_fd;

        Fd_map(int old_fd, int new_fd) : _old_fd{old_fd}, _new_fd{new_fd} {}
    };

private:
    Genode::Allocator &_alloc;

    Genode::List<Fd_map> _fd_map = Genode::List<Fd_map>{};
    int _fd_map_size = 0;

    static const int MAX_FD_STRING_LEN = 8;

public:
    explicit Load_manager(Genode::Allocator &alloc) : _alloc{alloc} {}

    void load_saved_state();

    void inject_state(Ptcp::Vfs_wrapper &);
};

#endif //_PTCP_LOAD_H_
