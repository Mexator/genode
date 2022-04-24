#ifndef _PTCP_SUPERVISOR_HELPER_H_
#define _PTCP_SUPERVISOR_HELPER_H_

// Ptcp includes
#include <ptcp_client/socket_state.h>
#include <ptcp_client/fd_proxy.h>
#include <ptcp_client/socket_supervisor.h>

/**
 * [socket_entry] instances are created without id assigned.
 * This class ensures that every socket supervised by [Socket_supervisor] is
 * assigned id.
 */
class Supervisor_helper {
    friend class Socket_supervisor;

    socket_entry *_md;
    Socket_supervisor &_socket_supervisor;

public:
    Supervisor_helper(Socket_supervisor &supervisor);

    void set_pending_entry(socket_entry &metadata);

    void submit_entry(Ptcp::Fd_proxy::Pfd &fd);

    void close(Ptcp::Fd_proxy::Pfd &fd);
};

extern Supervisor_helper *supervisor_helper;

#endif //_PTCP_SUPERVISOR_HELPER_H_
