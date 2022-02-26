#include <libc/component.h>
#include <stdio.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <libc-plugin/fd_alloc.h>
#include <list>

void test() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in in_addr;
    in_addr.sin_family = AF_INET;
    in_addr.sin_port = htons(80);
    in_addr.sin_addr.s_addr = INADDR_ANY;

    if (0 != bind(sock, (struct sockaddr *) &in_addr, sizeof(in_addr)))
        Genode::error("while calling bind()");
}

void Libc::Component::construct(Libc::Env &env) {
    with_libc([&]() {
        puts("hello world");
        // test persistent storage
        int desc = open("/test/file", O_RDWR | O_CREAT);
        Genode::log("file opened", desc, " ", errno);

        int r = write(desc, "hello hello!!", 14);
        fsync(desc);
        Genode::log("write finished", r, " ", errno);

        char a[125] = "";
        r = read(desc, a, 125);
        Genode::log("read finished", r, " ", errno, " ");
        puts(a);
        puts("a");
        puts(a);

        //test ptcp as a networked plugin
        test();

        // test get all files
        Genode::log("Opened file descriptors:");

        std::list<int> fds = {};
        file_descriptor_allocator()->idSpace().for_each<File_descriptor>([&fds](File_descriptor &fd) {
            fds.push_back(fd.libc_fd);
        });

        for (const int &item: fds) {
            struct sockaddr addr = {};
            socklen_t len = sizeof(sockaddr);

            if (-1 == getsockname(item, &addr, &len)) {
                switch (errno) {
                    case EBADF:
                        Genode::log("FD ", item, " does not belongs to libc socket plugin");
                        break;
                    case ENOTSOCK:
                        Genode::log("FD ", item, " is not a socket");
                        break;
                    default:
                        Genode::log("Unknown error, errno = ", errno);
                }
            } else {
                Genode::log("FD ", item, " is a socket with address family ", addr.sa_family);
            }
        }
    });
}