// Ptcp includes
#include <ptcp_client/supervisor_helper.h>

Supervisor_helper::Supervisor_helper(Socket_supervisor &supervisor) : _md(nullptr), _socket_supervisor(supervisor) {}

void Supervisor_helper::submit_entry(Ptcp::Pfd &fd) {
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

void Supervisor_helper::close(Ptcp::Pfd &fd) {
    _socket_supervisor.abandon(fd);
}

socket_entry *Supervisor_helper::get_entry_for(const char *path) {
    if (0 == Genode::strcmp(path, _md->socketPath.string(), _md->pathLen)) {
        return _md;
    }
    return _socket_supervisor.get_entry_for(path);
}

socket_entry *Supervisor_helper::get_entry_for(Vfs::Vfs_handle &handle) {
    if (_md && _md->belongs_to_this(handle)) return _md;
    return _socket_supervisor.get_entry_for(handle);
}

Supervisor_helper *supervisor_helper;