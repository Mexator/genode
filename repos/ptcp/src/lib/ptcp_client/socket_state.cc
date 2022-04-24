// Ptcp includes
#include <ptcp_client/socket_state.h>

socket_entry::socket_entry() : ptcpId{false, 0} {}

void socket_entry::print(Genode::Output &out) const {
    if (ptcpId.known)
        Genode::print(out, ptcpId.id);
    else
        Genode::print(out, "unknown id");
}