// Genode includes
#include <base/snprintf.h>

// PTCP includes
#include <vfs_ptcp/load.h>
#include <ptcp_client/fd_proxy.h>

// Local includes
#include <vfs_ptcp/lwip_wrapper.h>

void Ptcp::Snapshot::Load_manager::load_libc_state() {
    Genode::error("load_libc_state() not implemented");
    // Todo reopen fds and populate fd_map with actual data

    Fd_proxy* proxy = get_fd_proxy(_alloc);
    // Assume app remembered fd 100
    // Assume we restored fd 100 and socket() returned 2
    proxy->set(2, 100);
}

