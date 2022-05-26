#include <nic_trickster/control/stopper.h>

using namespace Nic_control;
using Genode::addr_t;

Net::Interface *to_local = nullptr;
Net::Interface *to_remote = nullptr;

Genode::Mutex Nic_control_impl::mutex;
bool Nic_control_impl::is_restore = false;

void Nic_control_impl::suspend() {
    mutex.acquire();
}

void Nic_control_impl::resume() {
    mutex.release();
}

void Nic_control_impl::set_restore_mode(bool enabled) {
    Genode::log("Restore mode toggle: ", enabled);
    is_restore = enabled;
}

void Nic_control_impl::send_packet(size_t len, Dataspace_capability cap) {
    Genode::log(__func__);
    addr_t ds_attach_addr = _env.rm().attach(cap);
    to_local->send(*(Net::Ethernet_frame *) ds_attach_addr, len);
    _env.rm().detach(ds_attach_addr);
}

Nic_socket_metadata
Nic_control_impl::get_md_value(Nic_socket_id id, Dataspace_capability cap, Dataspace_capability ackCap) {
    log(__func__, " Reading entries. md address:", (get_tracker().md_list));
    log(__func__, " looked up id ", id._remote, " ", id._remote_port, " ", id._local_port, " ");

    Genode::addr_t ds_attach_addr = _env.rm().attach(cap);
    Genode::addr_t ack_addr = _env.rm().attach(ackCap);

    auto item = get_tracker().lookup(id);
    if (!item) {
        Genode::error(__func__, " No metadata");
        throw Id_not_found();
    }

    Genode::memcpy((void *) ds_attach_addr, item->synFrame, item->md->_eth_size);
    _env.rm().detach(ds_attach_addr);

    Genode::memcpy((void *) ack_addr, item->ackFrame, item->md->_ack_size);
    _env.rm().detach(ack_addr);

    return *item->md;
}
