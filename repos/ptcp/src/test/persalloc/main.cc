#include <libc/component.h>
#include <persalloc/pers_heap.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>

constexpr char app_snap_path[] = "/snapshot/persalloc.app";
constexpr char heap_snap_path[] = "/snapshot/persalloc";

char *restore_app() {
    std::ifstream app;
    app.open(app_snap_path);
    Genode::addr_t pointer;
    app >> pointer;
    app.close();
    return (char *) pointer;
}

int my_main(Libc::Env &env) {
    Persalloc::Heap heap(env.ram(), env.rm());
    char *p = restore_app();
    if (nullptr != p) {
        Genode::log("Restoring");
        // restore
        std::ifstream heapdump;
        heapdump.open(heap_snap_path);
        unsigned long addr;
        Genode::size_t size;
        while (heapdump >> size >> addr) {
            heapdump.seekg(1, std::ios_base::cur);
            Genode::log("addr ", addr, " size ", size);
            heap.alloc_addr(size, addr);
            heapdump.read((char *) addr, size);
        }
        heapdump.close();
    } else {
        Genode::log("Starting from scratch");
        // no snapshot
        p = new(heap) char[32];
    }

    printf("p address %p\n", p);
    printf("old p value %s\n", p);
    strcpy(p, "hello world");
    printf("new p value %s\n", p);

    // save app
    std::ofstream app;
    app.open(app_snap_path);
    app << (Genode::addr_t) p;
    app.close();

    // save heap
    std::ofstream file;
    file.open(heap_snap_path);
    heap.for_each_region([&](void *addr, Genode::size_t size) {
        file << size << " " << (unsigned long) addr << " ";
        file.write((char *) addr, size);
        Genode::log("addr ", addr, " size ", size);
    });

    file.close();
    return 0;
}

void Libc::Component::construct(Libc::Env &env) {
    with_libc([&] { my_main(env); });
}
