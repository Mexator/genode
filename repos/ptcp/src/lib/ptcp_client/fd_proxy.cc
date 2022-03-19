#include <ptcp_client/fd_proxy.h>

Ptcp::Fd_proxy *instance = nullptr;

Ptcp::Fd_proxy *Ptcp::get_fd_proxy(Genode::Allocator &alloc) {
    if (instance == nullptr) {
        instance = new(alloc) Ptcp::Fd_proxy(alloc);
    }
    return instance;
}