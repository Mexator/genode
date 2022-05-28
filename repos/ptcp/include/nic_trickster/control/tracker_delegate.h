#ifndef _PTCP_TRACKER_DELEGATE_H_
#define _PTCP_TRACKER_DELEGATE_H_

#include <base/allocator.h>
#include "../../../../os/include/net/ethernet.h" // This shit clashes with libc net/ethernet.h
#include <net/ipv4.h>

#ifndef TRACKER_DEBUG
#define TRACKER_DEBUG true
#endif

#ifndef TRACKER_LOOKUP_DEBUG
#define TRACKER_LOOKUP_DEBUG false
#endif

using Genode::log;

namespace Tracker {
    struct Nic_socket_metadata;
    struct Nic_socket_id;
    struct Nic_socket_metadata;
}
using uint8_t = Genode::uint8_t;
using uint16_t = Genode::uint16_t;
using uint32_t = Genode::uint32_t;

// Y - initial SEQ of localhost
// X - initial SEQ of remote
struct Tracker::Nic_socket_metadata {
    int _eth_size;
    int _ack_size;
    uint32_t out_seq_offset; // n
    uint32_t seq; // Last packet ACKed by remote (Y + n)
    uint32_t ack; // Last packet ACKed by local (X + m)
};

/** Each socket is identified by 4-tuple:
    local address, local port, remote address, remote port.

    Assume we have only one client. Then we can omit local address */
struct Tracker::Nic_socket_id {
    uint16_t _local_port;
    Net::Ipv4_address _remote;
    uint16_t _remote_port;

    bool operator==(const Nic_socket_id &other);
};

using namespace Tracker;

/** Packets that this class observes should be TCP */
struct Tracker_delegate {

    struct list_item {
        Nic_socket_id *id;
        Nic_socket_metadata *md;
        Net::Ethernet_frame *synFrame;
        Net::Ethernet_frame *ackFrame;
        list_item *next;
    };

    list_item *md_list = nullptr;
    Genode::Allocator &_alloc;

    Tracker_delegate(Genode::Allocator &allocator);

    void packet_from_host(Net::Ethernet_frame &packet);

    void packet_to_host(const Net::Ethernet_frame &packet);

    list_item *lookup(Nic_socket_id &id);
};

extern Tracker_delegate *delegate;

Tracker_delegate &get_tracker();

#endif //_PTCP_TRACKER_DELEGATE_H_
