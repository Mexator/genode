// Genode includes
#include <base/snprintf.h>

// PTCP includes
#include <vfs_ptcp/load.h>

// Local includes
#include <vfs_ptcp/lwip_wrapper.h>

void Ptcp::Snapshot::Load_manager::load_saved_state() {
    Genode::error("load_saved_state() not implemented");
    // Todo reopen fds and populate fd_map with actual data

    _fd_map.insert(new(_alloc) Fd_map(0, 0));
    _fd_map_size++;
    _fd_map.insert(new(_alloc) Fd_map(-1, -1));
    _fd_map_size++;
}

void Ptcp::Snapshot::Load_manager::inject_state(Ptcp::Vfs_wrapper &wrapper) {
    // 2 file descriptors, single space and newline
    Genode::size_t str_len = 2 * MAX_FD_STRING_LEN + 1 + 1;
    char *buffer = new(_alloc) char[str_len * _fd_map_size];

    Genode::size_t offset = 0;
    Fd_map *element = _fd_map.first();
    while (element != nullptr) {
        Genode::size_t written = Genode::snprintf(
                buffer + offset, str_len, "%d %d\n", element->_old_fd, element->_new_fd
        );

        offset += written;
        element = element->next();
    }

    wrapper.initialize_readonly_fs(buffer, offset);
}

