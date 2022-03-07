#ifndef _PTCP_FD_PROXY_H_
#define _PTCP_FD_PROXY_H_

#include <base/id_space.h>
#include <base/allocator.h>
#include <base/registry.h>

namespace Ptcp {
    class Fd_proxy;
    namespace Snapshot {
        class Load_manager;
        struct Composed_state;
        Composed_state form_snapshot(Genode::Allocator &alloc);
    }

    Fd_proxy *get_fd_proxy(Genode::Allocator &alloc);
}

class Ptcp::Fd_proxy {
    struct Fd_handle;
public:
    typedef Genode::Id_space<Fd_handle> Fd_space;
    typedef Fd_space::Element Fd_element;
private:
    friend class Ptcp::Snapshot::Load_manager;
    friend Ptcp::Snapshot::Composed_state Ptcp::Snapshot::form_snapshot(Genode::Allocator &alloc);

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
            Genode::warning("Unknown id", id);
            return nullptr;
        }

        return result;
    }

    /**
     * Register [proxy_fd] as alias for [libc_fd]
     */
    void set(int libc_fd, int proxy_fd) {
        new(_alloc) Fd_handle(libc_fd, proxy_fd, fd_space);
    }

public:
    explicit Fd_proxy(Genode::Allocator &alloc) : _alloc(alloc) {}

    /**
     * Register [libc_fd] and returns handle to it
     */
    Fd_space::Id register_fd(int libc_fd) {
        Fd_handle *element = new(_alloc) Fd_handle(libc_fd, fd_space);
        return element->elem.id();
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

    /**
     * Destroy fd handle and return libc file descriptor it pointed to
     */
    int close_fd(Fd_space::Id id) {
        Fd_handle *res = lookup(id);
        if (res == nullptr) {
            return -1;
        }
        int ret = res->_libc_fd;
        res->~Fd_handle();
        return ret;
    }
};

#endif //_PTCP_FD_PROXY_H_
