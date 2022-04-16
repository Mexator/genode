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
#define FD_PROXY_DEBUG true
#endif

namespace Ptcp {
    class Fd_proxy;
}

/**
 * Class that lets its users use persistent file descriptors
 */
class Ptcp::Fd_proxy {
private:

    struct Fd_handle;
    typedef Genode::Id_space<Fd_handle> Fd_space;
    typedef Fd_space::Element Fd_element;

    // Wrapper to allow deleting of objects (see repos/libports/include/libc-plugin/fd_alloc.h)
    // I have a little idea how it works. But it does.
    struct Fd_handle {
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
     * Register [proxy_fd] as alias for [libc_fd].
     * Used only for restoration
     */
    void set(int libc_fd, int proxy_fd) {
        debug_log(FD_PROXY_DEBUG, "set proxy_fd=", proxy_fd, " to libc_fd=", libc_fd);
        new(_alloc) Fd_handle(libc_fd, proxy_fd, fd_space);
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
    // Persistent file descriptor. Is valid even after restart
    typedef Fd_space::Id Pfd;

    explicit Fd_proxy(Genode::Allocator &alloc) : _alloc(alloc), socket_creation_mutex() {}

    Fd_space::Id supervised_socket(int domain, int type, int protocol) {
        debug_log(FD_PROXY_DEBUG, __func__);
        Genode::Mutex::Guard _(socket_creation_mutex);
        int libc_fd = socket(domain, type, protocol);
        Pfd pfd = register_fd(libc_fd);
        // TODO actually supervise socket
        return pfd;
    }

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

static Genode::Constructible<Ptcp::Fd_proxy> fd_proxy;

#endif //_PTCP_FD_PROXY_H_
