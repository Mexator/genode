// Ptcp includes
#include <ptcp_client/fd_proxy.h>
#include <ptcp_client/supervisor_helper.h>

// Libc includes
#include <unistd.h>

// Debug includes
#include <logging/mylog.h>

using namespace Ptcp;

void Fd_proxy::set(int libc_fd, int proxy_fd) {
    debug_log(FD_PROXY_DEBUG, "set proxy_fd=", proxy_fd, " to libc_fd=", libc_fd);
    auto handle =  new(_alloc) Fd_handle(libc_fd, proxy_fd, fd_space);
    Pfd fd = handle->elem.id();
    supervisor_helper->submit_entry(fd);
}

Pfd Fd_proxy::register_fd(int libc_fd) {
    Genode::Mutex::Guard _(socket_creation_mutex);
    Fd_handle *element = new(_alloc) Fd_handle(libc_fd, fd_space);
    debug_log(FD_PROXY_DEBUG, "created proxy_fd=", element->elem.id(), " to libc_fd=", libc_fd);
    Pfd fd = element->elem.id();
    supervisor_helper->submit_entry(fd);
    return fd;
}

void Fd_proxy::close(Pfd &sockfd) {
    debug_log(FD_PROXY_DEBUG, __func__);
    int libc_fd = map_fd(sockfd);
    supervisor_helper->close(sockfd);
    ::close(libc_fd);
}

Ptcp::Fd_proxy *fd_proxy;
