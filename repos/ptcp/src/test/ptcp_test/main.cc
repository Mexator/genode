#include <libc/component.h>
#include <stdio.h>

// socket api
#include <netinet/in.h> // in_addr
#include <sys/socket.h> // socket
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
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

        test();
    });
}