// Ptcp includes
#include <ptcp_client/socket_state.h>

socket_entry::socket_entry(const char *path) : ptcpId({false, 0}),
                                               fsPath(path) {}

void socket_entry::print(Genode::Output &out) const {
    if (ptcpId.known)
        Genode::print(out, ptcpId.id);
    else
        Genode::print(out, "unknown id");
}