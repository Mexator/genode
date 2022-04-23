#include <base/thread.h>
#include <base/allocator.h>

namespace Net { class Interface; }

struct Submitter : Genode::Thread {
    Net::Interface &_i;

    Submitter(Net::Interface &i, Genode::Env &env) :
            Genode::Thread(env, "submitter", 0x40),
            _i(i) {}

    void entry() override;
};

struct Submitter_factory {
    Net::Interface &_i;
    Genode::Env &_env;
    Genode::Allocator &_alloc;
    Submitter *running;

    Submitter_factory(Net::Interface &i, Genode::Env &env, Genode::Allocator &alloc);

    void start_submitter_thread();
};
