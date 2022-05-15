// Ptcp includes
#include <ptcp_client/supervisor_helper.h>

// Debug includes
#include <logging/mylog.h>

Supervisor_helper::Supervisor_helper(Socket_supervisor &supervisor) :
        _md(nullptr),
        _socket_supervisor(supervisor),
        _can_pend(1),
        _can_submit(0) {}

// Consumer
void Supervisor_helper::submit_entry(Ptcp::Pfd &fd) {
    debug_log(MULTI_THREAD_SUBMIT_DEBUG, "enter ", __func__, " sem1=", _can_pend.cnt(), " sem2=", _can_submit.cnt());
    _can_submit.down();
    debug_log(MULTI_THREAD_SUBMIT_DEBUG, __func__);

    if (_md == nullptr)
        Genode::error(__func__, " no pending entry to assign id");
    _md->ptcpId = ptcp_id{true, fd.value};
    _socket_supervisor.supervise(*_md);
    _md = nullptr;
    _can_pend.up();
}

// Producer
void Supervisor_helper::set_pending_entry(socket_entry &metadata) {
    debug_log(MULTI_THREAD_SUBMIT_DEBUG, "enter ", __func__, " sem1=", _can_pend.cnt(), " sem2=", _can_submit.cnt());
    _can_pend.down();
    debug_log(MULTI_THREAD_SUBMIT_DEBUG, __func__);

    if (_md != nullptr)
        Genode::error(__func__, " tried to set pending entry while there already was entry pending");
    _md = &metadata;
    _can_submit.up();
}

void Supervisor_helper::close(Ptcp::Pfd &fd) {
    _socket_supervisor.abandon(fd);
}

socket_entry *Supervisor_helper::get_entry_for(const char *path) {
    if (_md && 0 == Genode::strcmp(path, _md->socketPath.string(), _md->pathLen)) {
        return _md;
    }
    return _socket_supervisor.get_entry_for(path);
}

socket_entry *Supervisor_helper::get_entry_for(Vfs::Vfs_handle &handle) {
    if (_md && _md->belongs_to_this(handle)) return _md;
    return _socket_supervisor.get_entry_for(handle);
}

Supervisor_helper *supervisor_helper;