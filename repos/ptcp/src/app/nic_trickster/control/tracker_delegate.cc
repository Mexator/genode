// Genode includes
#include <net/tcp.h>

// Local includes
#include <nic_trickster/control/tracker_delegate.h>

// Debug includes
#include <logging/mylog.h>

using namespace Net;

bool Tracker::Nic_socket_id::operator==(const Tracker::Nic_socket_id &other) {
    debug_log(TRACKER_DEBUG, "(", __func__,
              "\nthis.local port ", _local_port,
              "\nthis.remote addr ", _remote,
              "\nthis.remote port ", _remote_port,
              "\nother.local port ", other._local_port,
              "\nother.remote addr ", other._remote,
              "\nother.remote port ", other._remote_port,
              "\n)\033[0m ");
    return _local_port == other._local_port &&
           _remote == other._remote &&
           _remote_port == other._remote_port;
}

Tracker_delegate::Tracker_delegate(Genode::Allocator &allocator) : _alloc{allocator} {}

void Tracker_delegate::packet_from_host(const Ethernet_frame &packet) {
}

void Tracker_delegate::packet_to_host(const Ethernet_frame &packet) {
    if (packet.type() == Ethernet_frame::Type::IPV4) {
        Size_guard size_guard(~0UL);
        auto &ipv4 = packet.data < Ipv4_packet const>(size_guard);
        if (ipv4.protocol() == Ipv4_packet::Protocol::TCP) {
            Size_guard g(~0UL);
            auto &tcp = ipv4.data<const Tcp_packet>(g);

            if (tcp.syn()) { // Process new connections to the local host
                debug_log(TRACKER_DEBUG, __func__, " Adding new entry. md address:", &md_list);
                list_item *item = new(_alloc) list_item;
                item->id = new(_alloc) Nic_socket_id{
                        tcp.dst_port().value,
                        ipv4.src(), tcp.src_port().value
                };

                int len = sizeof(Ethernet_frame) + ipv4.total_length();
                item->synFrame = (Ethernet_frame *) new(_alloc) char[len];
                Genode::memcpy(item->synFrame, &packet, len);

                item->md = new(_alloc) Nic_socket_metadata{
                        len, 0
                };

                item->next = md_list;
                md_list = item;
            } else {
                // TODO: lookup item in list and increase seq/ack
            }

            uint16_t mask = 0x0FFF;
            bool ack_only = (tcp.flags() & mask) == 0x10;
            if (ack_only) { // Process the 3rd ACK in three-way handshake
                Nic_socket_id id{
                        tcp.dst_port().value,
                        ipv4.src(), tcp.src_port().value
                };
                auto md = lookup(id);
                if (md && md->ackFrame == nullptr) { // This is socket that awaits the 3rd ack
                    debug_log(TRACKER_DEBUG, __func__, " ACK#3 detected");
                    debug_log(TRACKER_DEBUG, __func__, " ACK#3 flags:", Genode::Hex(tcp.flags() & mask));

                    int len = sizeof(Ethernet_frame) + ipv4.total_length();
                    md->ackFrame = (Ethernet_frame *) new(_alloc) char[len];
                    Genode::memcpy(md->ackFrame, &packet, len);

                    md->md->_ack_size = len;
                    debug_log(TRACKER_DEBUG, __func__, " ACK#3 processed");
                }
            }
        }
    }
}

Tracker_delegate::list_item *Tracker_delegate::lookup(Nic_socket_id &id) {
    debug_log(TRACKER_DEBUG, __func__);
    auto item = md_list;
    while (item != nullptr) {
        if (*item->id == id) return item;
        item = item->next;
    }
    return item;
}

Tracker_delegate *delegate;

Tracker_delegate &get_tracker() {
    if (!delegate) throw Genode::Exception();
    return *delegate;
}
