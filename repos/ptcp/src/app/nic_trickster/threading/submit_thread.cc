#include "submit_thread.h"
#include <nic_trickster/interface.h>
#include <nic_trickster/control/nic_control_impl.h>

void Submitter::entry() {
    while (_i._sink().packet_avail()) {
        Genode::Mutex::Guard _(Nic_control_impl::mutex);

        ::Nic::Packet_descriptor const pkt = _i._sink().get_packet();
        if (!pkt.size() || !_i._sink().packet_valid(pkt)) {
            continue;
        }

        _i._handle_eth(_i._sink().packet_content(pkt), pkt.size(), pkt);

        if (!_i._sink().ready_to_ack()) {
            Genode::error("ack state FULL");
            return;
        }
        _i._sink().acknowledge_packet(pkt);
    }
}

Submitter_factory::Submitter_factory(Net::Interface &i, Genode::Env &env, Genode::Allocator &alloc) :
        _i(i),
        _env(env),
        _alloc(alloc),
        running(nullptr) {
}

void Submitter_factory::start_submitter_thread() {
    if (running != nullptr) {
        running->join();
        running->~Submitter();
        _alloc.free(running, 0);
    }
    running = new(_alloc) Submitter(_i, _env);
    running->start();
}
