// Ptcp includes
#include <ptcp_client/fd_proxy.h>
#include <ptcp_client/supervisor_helper.h>

// Socket api
#include <sys/socket.h>
#include <unistd.h>

using namespace Ptcp;

void Fd_proxy::set(int libc_fd, int proxy_fd) {
    debug_log(FD_PROXY_DEBUG, "set proxy_fd=", proxy_fd, " to libc_fd=", libc_fd);
    auto handle =  new(_alloc) Fd_handle(libc_fd, proxy_fd, fd_space);
    Pfd fd = handle->elem.id();
    supervisor_helper->submit_entry(fd);
}

Fd_proxy::Pfd Fd_proxy::supervised_socket(int domain, int type, int protocol) {
    Genode::Mutex::Guard _(socket_creation_mutex);
    debug_log(FD_PROXY_DEBUG, __func__);
    int libc_fd = socket(domain, type, protocol);
    Pfd pfd = register_fd(libc_fd);
    supervisor_helper->submit_entry(pfd);
    return pfd;
}

Fd_proxy::Pfd Fd_proxy::accept(Pfd &sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    Genode::Mutex::Guard _(socket_creation_mutex);
    debug_log(FD_PROXY_DEBUG, __func__);
    int libc_fd = ::accept(map_fd(sockfd), addr, addrlen);
    Pfd pfd = register_fd(libc_fd);
    supervisor_helper->submit_entry(pfd);
    return pfd;
}

void Fd_proxy::close(Pfd &sockfd) {
    debug_log(FD_PROXY_DEBUG, __func__);
    int libc_fd = map_fd(sockfd);
    supervisor_helper->close(sockfd);
    ::close(libc_fd);
}