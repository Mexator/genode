#ifndef _PTCP_SESSION_H_
#define _PTCP_SESSION_H_

#include <session/session.h>
#include <base/connection.h>

namespace Nic_control {
    class Session;

    class Session_client;

    class Connection;
}

struct Nic_control::Session : Genode::Session {
    static const char *service_name() { return "Nic_control"; }

    // No caps needed
    static const int CAP_QUOTA = 0;

    virtual void suspend() = 0;

    virtual void resume() = 0;

    GENODE_RPC(Rpc_suspend, void, suspend);

    GENODE_RPC(Rpc_resume, void, resume);

    GENODE_RPC_INTERFACE(Rpc_suspend, Rpc_resume);
};

struct Nic_control::Session_client : Genode::Rpc_client<Session> {
    Session_client(Genode::Capability<Session> cap)
            : Genode::Rpc_client<Session>(cap) {}

    void suspend() override { call<Rpc_suspend>(); }

    void resume() override { call<Rpc_resume>(); }
};

struct Nic_control::Connection : Genode::Connection<Session>, Nic_control::Session_client {
    Connection(Genode::Env &env) :
            Genode::Connection<Session>(
                    env,
                    session(env.parent(), "ram_quota=6K, cap_quota=4")
            ),
            Session_client(cap()) {}
};

#endif //_PTCP_SESSION_H_
