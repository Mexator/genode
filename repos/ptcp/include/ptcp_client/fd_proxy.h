#ifndef _PTCP_FD_PROXY_H_
#define _PTCP_FD_PROXY_H_

// Genode includes
#include <base/id_space.h>
#include <base/allocator.h>
#include <base/mutex.h>
#include <util/reconstructible.h>

// Socket api
#include <sys/socket.h>

// Debug includes
#include <logging/mylog.h>

#ifndef FD_PROXY_DEBUG
#define FD_PROXY_DEBUG false
#endif

namespace Ptcp {
    class Fd_proxy;

    struct Fd_handle;
    typedef Genode::Id_space<Fd_handle> Fd_space;
    typedef Fd_space::Element Fd_element;
    // Persistent file descriptor. Is valid even after restart
    typedef Fd_space::Id Pfd;
}


// Wrapper to allow deleting of objects (see repos/libports/include/libc-plugin/fd_alloc.h)
// I have a little idea how it works. But it does.
struct Ptcp::Fd_handle {
    Fd_element elem;
    int _libc_fd;

    explicit Fd_handle(int libc_fd, Fd_space &space) : elem(*this, space), _libc_fd(libc_fd) {}

    explicit Fd_handle(int libc_fd, int proxy_fd, Fd_space &space) :
            elem(
                    *this,
                    space,
                    Fd_space::Id{(unsigned long) proxy_fd}),
            _libc_fd(libc_fd) {}
};

/**
 * Class that lets its users use persistent file descriptors
 */
class Ptcp::Fd_proxy {
private:
    Fd_space fd_space;
    Genode::Allocator &_alloc;
    Genode::Mutex socket_creation_mutex;

    /**
     * Resolve Fd in [fd_space] by [id]
     */
    Fd_handle *lookup(Fd_space::Id id) {

        Fd_handle *result = nullptr;

        try {
            fd_space.apply<Fd_handle>(id, [&](Fd_handle &fd) {
                result = &fd;
            });
        } catch (Fd_space::Unknown_id) {
            Genode::warning("Unknown id ", id);
            return nullptr;
        }

        return result;
    }

    /**
     * Register [libc_fd] and returns handle to it
     */
    Fd_space::Id register_fd(int libc_fd) {
        Fd_handle *element = new(_alloc) Fd_handle(libc_fd, fd_space);
        debug_log(FD_PROXY_DEBUG, "created proxy_fd=", element->elem.id(), " to libc_fd=", libc_fd);
        return element->elem.id();
    }

public:

    explicit Fd_proxy(Genode::Allocator &alloc) : _alloc(alloc), socket_creation_mutex() {}

    /**
     * Register [proxy_fd] as alias for [libc_fd].
     * Used only for restoration
     */
    void set(int libc_fd, int proxy_fd);
    Pfd supervised_socket(int domain, int type, int protocol);
    Pfd accept(Pfd &sockfd, struct sockaddr *addr, socklen_t *addrlen);
    void close(Pfd &sockfd);

    /**
     * Retrieve libc file descriptor by fd handle
     */
    int map_fd(Fd_space::Id id) {
        Fd_handle *res = lookup(id);
        if (res == nullptr) {
            return -1;
        }
        return res->_libc_fd;
    }
};

extern Ptcp::Fd_proxy *fd_proxy;

#endif //_PTCP_FD_PROXY_H_
