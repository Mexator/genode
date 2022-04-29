// Ptcp includes
#include <ptcp_client/socket_state.h>

using namespace Vfs;

socket_entry::socket_entry(Vfs_handle &handle)
        : _handle{handle}, _bind_handle{nullptr},
          ptcpId{false, 0}, pathLen{0}, boundAddress{} {}

void socket_entry::print(Genode::Output &out) const {
    if (ptcpId.known)
        Genode::print(out, ptcpId.id);
    else
        Genode::print(out, "unknown id");
}

bool socket_entry::belongs_to_this(Vfs::Vfs_handle &handle) {
    if (&_handle == &handle) return true;
    if (_bind_handle == &handle) return true;
    return false;
}