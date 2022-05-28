// Local includes
#include <nic_trickster/control/tracker_delegate.h>
#include <nic_trickster/tcp.h>

// Debug includes
#include <logging/mylog.h>

using namespace Net;
using Genode::Hex;

bool Tracker::Nic_socket_id::operator==(const Tracker::Nic_socket_id &other) {
    debug_log(TRACKER_LOOKUP_DEBUG, "(", __func__,
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

void Tracker_delegate::packet_from_host(Ethernet_frame &packet) {

    if (packet.type() == Ethernet_frame::Type::IPV4) {
        Size_guard ip_guard(~0UL);
        auto &ipv4 = packet.data < Ipv4_packet const>(ip_guard);
        if (ipv4.protocol() == Ipv4_packet::Protocol::TCP) {
            Size_guard tcp_guard(~0UL);
            Tcp_packet &tcp = const_cast<Tcp_packet &>(ipv4.data<Tcp_packet>(tcp_guard));

            Nic_socket_id &id = *new(_alloc) Nic_socket_id{
                    tcp.src_port().value,
                    ipv4.dst(), tcp.dst_port().value
            };
            list_item *md = lookup(id);

            if (tcp.ack()) {
                if (!md) {
                    Genode::warning(__func__, " Unknown socket");
                } else {
                    md->md->ack = tcp.ack_nr();
                    debug_log(TRACKER_DEBUG, "Local ack increased, new ack = ", md->md->ack);
                }
            }

            // offset seq number of out-coming packets
            uint32_t seq_offset = 0;
            if (md) { seq_offset = md->md->out_seq_offset; }

            // I had hypothesis that packets get rejected because of wrong timestamps.
            // This code might be useful so I kept it
//
//            Genode::warning("here");
//            int tsStart=0;
//            char *ptr = (char *) &tcp._data;
//            int tcpLen = ipv4.total_length() - sizeof(ipv4) - sizeof(tcp);
//            if (tcpLen != 0) {
//                Genode::warning("tspLen", tcpLen);
//                for (int i = 0; i < tcpLen; ++i) {
//                    uint8_t tsval = ptr[i];
//                    if (tsval == 0x8) {
//                        tsStart = i;
//                        break;
//                    }
//                }
//                tsStart += 1;
//                uint32_t tsVal;
//                Genode::memcpy(&tsVal, ptr + tsStart + 1, 4);
//                uint32_t tsEcho;
//                Genode::memcpy(&tsEcho, ptr + tsStart + 4 + 1, 4);
//                tsVal = host_to_big_endian(tsVal);
//                tsEcho = host_to_big_endian(tsEcho);
//                Genode::warning("tsVal_old", Hex(tsVal));
//                uint16_t oft = 0;
//                if (seq_offset) {
//                    oft = 10000;
//                }
//                tsVal += oft;
//                Genode::warning("tsVal", Hex(tsVal));
//
//                tsVal = host_to_big_endian(tsVal);
//                tsEcho = host_to_big_endian(tsEcho);
//                Genode::memcpy(ptr + tsStart + 1, &tsVal, 4);
//                Genode::memcpy(&tsVal, ptr + tsStart + 1, 4);
//                Genode::warning("tsVal_updated", Hex(tsVal));
//
//                uint16_t newch = ~(~tcp.checksum() + oft);
//                tcp._checksum = host_to_big_endian(newch);
//            }
            tcp._seq_nr = host_to_big_endian(tcp.seq_nr() + seq_offset);
            uint16_t newch = ~(~tcp.checksum() + seq_offset);
            tcp._checksum = host_to_big_endian(newch);
        }
    }
}

void Tracker_delegate::packet_to_host(const Ethernet_frame &packet) {
    if (packet.type() == Ethernet_frame::Type::IPV4) {
        Size_guard size_guard(~0UL);
        auto &ipv4 = packet.data < Ipv4_packet const>(size_guard);
        if (ipv4.protocol() == Ipv4_packet::Protocol::TCP) {
            Size_guard g(~0UL);
            Tcp_packet &tcp = const_cast<Tcp_packet &>(ipv4.data<Tcp_packet>(g));

            Nic_socket_id &id = *new(_alloc) Nic_socket_id{
                    tcp.dst_port().value,
                    ipv4.src(), tcp.src_port().value
            };

            if (tcp.syn()) { // Process new connections to the local host
                list_item *item = new(_alloc) list_item;
                debug_log(TRACKER_DEBUG, __func__, " Adding new entry. md address:", &md_list);
                item->id = &id;

                int len = sizeof(Ethernet_frame) + ipv4.total_length();
                item->synFrame = (Ethernet_frame *) new(_alloc) char[len];
                Genode::memcpy(item->synFrame, &packet, len);

                item->md = new(_alloc) Nic_socket_metadata{
                        len, 0, 0, tcp.seq_nr(), 0
                };

                item->next = md_list;
                md_list = item;
            }

            list_item *item = lookup(id);

            if (tcp.ack() && item) { // update ack
                item->md->seq = tcp.ack_nr();
                debug_log(TRACKER_DEBUG, "Local seq increased, new seq = ", item->md->seq);
            }

            // offset ack number of incoming packets to match offset of out-coming packets
            uint32_t seq_offset = 0;
            if (item) { seq_offset = item->md->out_seq_offset; }
            tcp._ack_nr = host_to_big_endian(tcp.ack_nr() - seq_offset);
            uint16_t newch = ~(~tcp.checksum() - seq_offset);
            tcp._checksum = host_to_big_endian(newch);

            uint16_t mask = 0x0FFF;
            bool ack_only = (tcp.flags() & mask) == 0x10;
            if (ack_only && item) { // Process the 3rd ACK in three-way handshake
                if (item->ackFrame == nullptr) { // This is socket that awaits the 3rd ack
                    debug_log(TRACKER_DEBUG, __func__, " ACK#3 detected");
                    debug_log(TRACKER_DEBUG, __func__, " ACK#3 flags:", Genode::Hex(tcp.flags() & mask));

                    int len = sizeof(Ethernet_frame) + ipv4.total_length();
                    item->ackFrame = (Ethernet_frame *) new(_alloc) char[len];
                    Genode::memcpy(item->ackFrame, &packet, len);

                    item->md->_ack_size = len;
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
