
#ifndef _PTCP_SNAPSHOT_H_
#define _PTCP_SNAPSHOT_H_

/**
 * Libc includes
 */
#include <cstdio>

/**
 * Lwip includes
 */
#define LWIP_NO_STDINT_H 0
#define LWIP_NO_UNISTD_H 0
#define SSIZE_MAX

// Genode includes
#include <base/allocator.h>

// lwip includes
#include <lwip/tcp.h>

namespace Ptcp {
    namespace Snapshot {
        namespace Libc {
            struct Plugin_state;
            struct Socket_state;
        }

        struct Lwip_state;

        struct Composed_state;

        Composed_state form_snapshot(Genode::Allocator &alloc);
    }
}

// Libc socket plugin state
struct Ptcp::Snapshot::Libc::Socket_state {
    int proto;
    unsigned int state;
    unsigned long proxy_handle;
};

struct Ptcp::Snapshot::Libc::Plugin_state {
    Socket_state *socket_states;
    size_t sockets_number;
};
const Ptcp::Snapshot::Libc::Plugin_state LIBC_EMPTY{nullptr, 0};

struct Ptcp::Snapshot::Lwip_state {
    tcp_pcb *bound_pcbs;
};
const Ptcp::Snapshot::Lwip_state LWIP_EMPTY{nullptr};

struct Ptcp::Snapshot::Composed_state {
    Libc::Plugin_state libc_state;
    Lwip_state lwip_state;
};

#endif //_PTCP_SNAPSHOT_H_
