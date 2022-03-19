#include <base/registry.h>
#include <root/component.h>
#include <base/component.h>
#include <base/heap.h>
#include <timer_session/connection.h>

#include <snapshot_notifier/session.h>

namespace Snapshot_notifier {
    struct Session_component;
    struct Main;
    struct Root;
}

struct Snapshot_notifier::Session_component : Genode::Rpc_object<Session> {

    Genode::Signal_context_capability _handler{};

    void add_handler(Genode::Signal_context_capability cap) override {
        Genode::log("add_handler called");
        _handler = cap;
    }
};

struct Snapshot_notifier::Root : Genode::Root_component<Session_component> {
protected:
    Genode::Registry<Genode::Registered<Session_component>> _sessions{};

    Session_component *_create_session(const char *) override {
        return new(md_alloc())
                Genode::Registered<Session_component>(_sessions);
    }

public:
    void notify_handlers() {
        _sessions.for_each([&](Session_component &session) {
            Genode::Signal_transmitter(session._handler).submit();
        });
    }

    Root(Genode::Env &env, Genode::Allocator &md_alloc)
            : Genode::Root_component<Session_component>{&env.ep().rpc_ep(), &md_alloc} {}
};

class Snapshot_notifier::Main {
    Genode::Env &_env;

    /**
     * Common RPC members
     */
    Genode::Sliced_heap sliced_heap{_env.ram(), _env.rm()};
    Root root{_env, sliced_heap};

    /**
     * Timer related members
     */
    Timer::Connection _timer{_env};
    Genode::Signal_handler<Main> _timeout_handler{_env.ep(), *this, &Main::send_signal};

    /**
     * Actual code of signal sending
     */
    void send_signal() {
        Genode::log("woke up at ", _timer.elapsed_ms(), " ms");
        root.notify_handlers();
    }

public:
    explicit Main(Genode::Env &env) : _env(env) {
        _timer.sigh(_timeout_handler);
        _timer.trigger_periodic(3 * 1000 * 1000);
        env.parent().announce(env.ep().manage(root));
    }
};

void Component::construct(Genode::Env &env) {
    Genode::log("Hello from emitter");
    static Snapshot_notifier::Main _(env);
}
