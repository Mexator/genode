// Ptcp includes
#include <ptcp_client/supervisor_helper.h>

Supervisor_helper::Supervisor_helper(Socket_supervisor &supervisor) : _md(nullptr), _socket_supervisor(supervisor) {}

void Supervisor_helper::assign_id(Ptcp::Fd_proxy::Pfd &fd) {
    if (_md == nullptr)
        Genode::error(__func__, " no pending entry to assign id");
    _md->ptcpId = ptcp_id{true, fd.value};
    _socket_supervisor.supervise(*_md);
    _md = nullptr;
}

void Supervisor_helper::set_pending_entry(socket_entry &metadata) {
    if (_md != nullptr)
        Genode::error(__func__, " tried to set pending entry while there already was entry pending");
    _md = &metadata;
}

void Supervisor_helper::close(Ptcp::Fd_proxy::Pfd &fd) {
    _socket_supervisor.abandon(fd);
}

Supervisor_helper *supervisor_helper;