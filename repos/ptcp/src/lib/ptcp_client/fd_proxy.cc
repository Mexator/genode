// Ptcp includes
#include <ptcp_client/fd_proxy.h>
#include <ptcp_client/supervisor_helper.h>

// Socket api
#include <sys/socket.h>
#include <unistd.h>

using namespace Ptcp;

Fd_proxy::Pfd Fd_proxy::supervised_socket(int domain, int type, int protocol) {
    Genode::Mutex::Guard _(socket_creation_mutex);
    debug_log(FD_PROXY_DEBUG, __func__);
    int libc_fd = socket(domain, type, protocol);
    Pfd pfd = register_fd(libc_fd);
    supervisor_helper->assign_id(pfd);
    return pfd;
}

Fd_proxy::Pfd Fd_proxy::accept(Pfd &sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    Genode::Mutex::Guard _(socket_creation_mutex);
    debug_log(FD_PROXY_DEBUG, __func__);
    int libc_fd = ::accept(map_fd(sockfd), addr, addrlen);
    Pfd pfd = register_fd(libc_fd);
    supervisor_helper->assign_id(pfd);
    return pfd;
}

void Fd_proxy::close(Pfd &sockfd) {
    debug_log(FD_PROXY_DEBUG, __func__);
    int libc_fd = map_fd(sockfd);
    supervisor_helper->close(sockfd);
    ::close(libc_fd);
}