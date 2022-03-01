#include <vfs_ptcp/snapshot.h>
#include <libc-plugin/fd_alloc.h>
#include <errno.h>
#include <list>

#include <lwip/priv/tcp_priv.h>

typedef Libc::File_descriptor File_descriptor;

void log_wrong_fd(int err, int libc_fd);

using namespace Ptcp::Snapshot;

struct Ptcp::Snapshot::Composed_state Ptcp::Snapshot::form_snapshot() {

    // save libc
    std::list<int> fds = {};
    Libc::file_descriptor_allocator()->idSpace().for_each<File_descriptor>([&fds](File_descriptor &fd) {
        fds.push_back(fd.libc_fd);
    });

    std::list<socket_state> states;

    for (const int &item: fds) {
        struct sockaddr addr = {};
        socklen_t len = sizeof(sockaddr);

        if (0 == getsockname(item, &addr, &len)) {
            Genode::log("FD ", item, " is a socket with address family ", addr.sa_family);

            struct socket_state state = {};
            socklen_t state_len = sizeof(socket_state);
            if (0 == getsockopt(item, SOL_SOCKET, SO_INTERNAL_STATE, &state, &state_len)) {
                states.push_back(state);
            }
        } else {
            log_wrong_fd(errno, item);
        }
    }
    socket_state *arr = new socket_state[states.size()];
    std::copy(states.begin(), states.end(), arr);
    return Composed_state{
            Libc_plugin_state{
                    arr,
                    static_cast<size_t>(states.size())
            },
            Lwip_state {
                    tcp_bound_pcbs
            }
    };
}

void log_wrong_fd(int err, int libc_fd) {
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