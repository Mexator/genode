#include <vfs_ptcp/snapshot.h>
#include <ptcp_client/fd_proxy.h>
#include <errno.h>
#include <list>

#include <internal/snapshot/socket_snapshot.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace Ptcp::Snapshot;

void log_wrong_fd(int err, int libc_fd);

constexpr char FORM_SNAPSHOT_PREFIX[] = "FORMING SNAPSHOT";

static Persalloc::Heap *_heap;

void Ptcp::Snapshot::set(Persalloc::Heap *h) {
    _heap = h;
}

Persalloc::Heap *Ptcp::Snapshot::get() {
    return _heap;
}

struct Ptcp::Snapshot::Composed_state Ptcp::Snapshot::form_snapshot(Genode::Allocator &alloc) {
    typedef Ptcp::Fd_proxy::Fd_handle Fd_handle;

    // save libc
    Ptcp::Fd_proxy *fd_proxy = Ptcp::get_fd_proxy(alloc);

    std::list<Fd_handle *> fds = {};
    fd_proxy->fd_space.for_each<Fd_handle>([&fds](Fd_handle &fd) {
        fds.push_back(&fd);
    });

    std::list<Libc::Socket_state> states;

    for (auto &item: fds) {
        struct sockaddr_in addr = {};
        socklen_t len = sizeof(sockaddr_in);

        if (0 == getsockname(item->_libc_fd, (sockaddr *) &addr, &len)) {
            if (addr.sin_family != AF_INET && addr.sin_family != AF_INET6) {
                Genode::warning("Address family ", addr.sin_family, " not fully supported");
            }

            struct socket_state state = {};
            socklen_t state_len = sizeof(socket_state);
            if (0 == getsockopt(item->_libc_fd, SOL_SOCKET, SO_INTERNAL_STATE, &state, &state_len)) {
                states.push_back(Libc::Socket_state{
                        state.proto,
                        state.state,
                        item->elem.id().value,
                        addr
                });
            } else {
                Genode::warning("getsockopt returned non-zero!");
            }
        } else {
            log_wrong_fd(errno, item->_libc_fd);
        }
    }

    // Lwip state

    auto heap = Snapshot::get();
    if (heap == nullptr) {
        Genode::error("heap is null! ", heap);
    }
    std::list<Snapshot::Lwip_state::Dataspace> dataspaces;
    for (Persalloc::Heap::Dataspace const *ds = heap->ds_pool().first(); ds; ds = ds->next()) {
        dataspaces.push_back(
                Snapshot::Lwip_state::Dataspace{
                        nullptr,
                        ds->local_addr,
                        ds->size
                }
        );
    }

    Libc::Socket_state *arr = new Libc::Socket_state[states.size()];
    std::copy(states.begin(), states.end(), arr);

    Lwip_state::Dataspace *arr2 = new Lwip_state::Dataspace[dataspaces.size()];
    std::copy(dataspaces.begin(), dataspaces.end(), arr2);

    return Composed_state{
            Libc::Plugin_state{
                    arr,
                    static_cast<size_t>(states.size())
            },
            Lwip_state{
                    arr2,
                    static_cast<int>(dataspaces.size()),
                    (Genode::addr_t) heap->region_addr_to_local(0)
            }
    };
}

void log_wrong_fd(int err, int libc_fd) {
    Genode::log(FORM_SNAPSHOT_PREFIX);
    switch (err) {
        case EBADF:
            Genode::log("FD ", libc_fd, " does not belongs to libc socket plugin");
            break;
        case ENOTSOCK:
            Genode::log("FD ", libc_fd, " is not a socket");
            break;
        default:
            Genode::log("Unknown error, errno = ", err);
    }
}
