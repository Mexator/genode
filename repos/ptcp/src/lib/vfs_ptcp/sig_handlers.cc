#include <base/rpc_client.h>
#include <base/component.h>
#include <base/connection.h>
#include <snapshot_notifier/session.h>
#include <vfs_ptcp/snapshot.h>

namespace Ptcp {
    struct Session_client;
    struct Connection;
    struct Signals_entrypoint;
}

struct Ptcp::Session_client : Genode::Rpc_client<Snapshot_notifier::Session> {
    explicit Session_client(Genode::Capability<Snapshot_notifier::Session> cap)
            : Genode::Rpc_client<Snapshot_notifier::Session>(cap) {}

    void add_handler(Genode::Signal_context_capability cap) override {
        call<Rpc_add_handler>(cap);
    }
};

struct Ptcp::Connection : Genode::Connection<Snapshot_notifier::Session>, Session_client {
    Connection(Genode::Env &env)
            : Genode::Connection<Snapshot_notifier::Session>(env, session(env.parent(), "")),
              Session_client(cap()) {}
};

struct Ptcp::Signals_entrypoint {
    Genode::Env &_env;

    Genode::Signal_handler<Signals_entrypoint> _sig_snap_handler{_env.ep(), *this, &Signals_entrypoint::do_snapshot};

    Ptcp::Connection _snapshot_notifier{_env};

    explicit Signals_entrypoint(Genode::Env &env) : _env{env} {
        _snapshot_notifier.add_handler(_sig_snap_handler);
    };

    void do_snapshot() {
        Genode::log("do_snapshot() of client called");
        Lwip_snapshot a = form_snapshot();
        if (tcp_pcb *bound = a.tcp_bound_pcbs) {
            Genode::log("do_snapshot: tcp_bound_pcbs->local_port. If it is 80, test is OK\n",
                        bound->local_port);
        } else {
            Genode::log("bound is nullptr");
        }
    }
};

void setup_sig_handlers(Genode::Env &env) {
    static Ptcp::Signals_entrypoint _{env};
}