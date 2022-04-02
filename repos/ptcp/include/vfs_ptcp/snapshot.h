
#ifndef _PTCP_SNAPSHOT_H_
#define _PTCP_SNAPSHOT_H_
// Genode includes
#include <base/allocator.h>

// libc includes
#include <sys/socket.h>
#include <netinet/in.h>

// "internal" includes
#include <persalloc/pers_heap.h>

namespace Ptcp {
    namespace Snapshot {
        namespace Libc {
            struct Plugin_state;
            struct Socket_state;
        }

        struct Lwip_state;

        struct Composed_state;

        Composed_state form_snapshot(Genode::Allocator &alloc);

        void set(Persalloc::Heap *h);
        Persalloc::Heap *get();
    }
}

// Libc socket plugin state
struct Ptcp::Snapshot::Libc::Socket_state {
    int proto;
    unsigned int state;
    unsigned long proxy_handle;
    sockaddr_in addr;
};

struct Ptcp::Snapshot::Libc::Plugin_state {
    Socket_state *socket_states;
    size_t sockets_number;
};
const Ptcp::Snapshot::Libc::Plugin_state LIBC_EMPTY{nullptr, 0};

struct Ptcp::Snapshot::Lwip_state {
    struct Dataspace {
        Persalloc::Heap::Local_address addr;
        Genode::size_t size;
    } *dataspaces;

    int dataspaces_number;
};
const Ptcp::Snapshot::Lwip_state LWIP_EMPTY{nullptr, 0};

struct Ptcp::Snapshot::Composed_state {
    Libc::Plugin_state libc_state;
    Lwip_state lwip_state;
};

#endif //_PTCP_SNAPSHOT_H_
