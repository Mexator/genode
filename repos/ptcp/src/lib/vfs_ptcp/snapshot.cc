#include <vfs_ptcp/snapshot.h>
#include <lwip/priv/tcp_priv.h>

Lwip_snapshot form_snapshot() {
    return Lwip_snapshot{
        tcp_bound_pcbs
    };
}
