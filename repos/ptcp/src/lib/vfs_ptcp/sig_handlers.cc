#include <base/rpc_client.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/connection.h>
#include <snapshot_notifier/session.h>
#include <vfs_ptcp/snapshot.h>
#include <vfs_ptcp/persist.h>
#include <libc/component.h>

namespace Ptcp {
    struct Session_client;
    struct Connection;
    struct Signals_entrypoint;
}

struct Ptcp::Session_client : Genode::Rpc_client<Snapshot_notifier::Session> {
    explicit Session_client(const Genode::Capability<Snapshot_notifier::Session> &cap)
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
    Genode::Allocator &_alloc;

    Genode::Signal_handler<Signals_entrypoint> _sig_snap_handler{_env.ep(), *this, &Signals_entrypoint::do_snapshot};

    Ptcp::Connection _snapshot_notifier{_env};

    explicit Signals_entrypoint(Genode::Env &env, Genode::Allocator &alloc) : _env{env}, _alloc{alloc} {
        _snapshot_notifier.add_handler(_sig_snap_handler);
    };

    void do_snapshot() {
        Genode::log("do_snapshot() of client called");
        Libc::with_libc([&] {
            Snapshot::Composed_state state = Snapshot::form_snapshot(_alloc);
            Genode::log("do_snapshot(): state formed");
            persist_saved_state(state);
            Genode::log("Saved state of ", state.libc_state.sockets_number, " sockets");
        });
    }
};

void setup_sig_handlers(Genode::Env &env) {
    static Genode::Heap heap(env.ram(), env.rm());
    static Ptcp::Signals_entrypoint _(env, heap);
}