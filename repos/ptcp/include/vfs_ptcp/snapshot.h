
#ifndef _PTCP_SNAPSHOT_H_
#define _PTCP_SNAPSHOT_H_

/**
 * Libc includes
 */
#include <internal/snapshot/socket_snapshot.h>
#include <cstdio>

/**
 * Lwip includes
 */
#define LWIP_NO_STDINT_H 0
#define LWIP_NO_UNISTD_H 0
#define SSIZE_MAX
#include <lwip/tcp.h>

namespace Ptcp {
    namespace Snapshot {
        struct Libc_plugin_state;
        struct Lwip_state;

        struct Composed_state;

        Composed_state form_snapshot();
    }
}

// Libc socket plugin state
struct Ptcp::Snapshot::Libc_plugin_state {
    socket_state *socket_states;
    size_t sockets_number;
};

struct Ptcp::Snapshot::Lwip_state{
    tcp_pcb* bound_pcbs;
};

struct Ptcp::Snapshot::Composed_state {
    Libc_plugin_state libc_state;
    Lwip_state lwip_state;
};

#endif //_PTCP_SNAPSHOT_H_
