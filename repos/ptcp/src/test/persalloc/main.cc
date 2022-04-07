#include <libc/component.h>
#include <persalloc/pers_heap.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <base/heap.h>
#include <rm_session/connection.h>
#include <region_map/client.h>
#include <dataspace/client.h>

constexpr char app_snap_path[] = "/snapshot/persalloc.app";
constexpr char heap_snap_path[] = "/snapshot/persalloc";

char *restore_app() {
    std::ifstream app;
    app.open(app_snap_path);
    Genode::addr_t pointer = 0;
    app >> pointer;
    app.close();
    return (char *) pointer;
}

constexpr Genode::size_t SIZE = 1024 * 1024 * 16;

Genode::Region_map &create_pers_region_map(Genode::Env &env, Genode::Allocator &alloc) {
    static Genode::Rm_connection rm(env);

    Genode::log("Creating region map with size ", SIZE);
    Genode::Region_map_client *region_map = new(alloc)Genode::Region_map_client(rm.create(SIZE));

    return *region_map;
}

int my_main(Libc::Env &env) {
    static Genode::Sliced_heap metadata_heap(env.ram(), env.rm());

    Genode::Region_map &persistent_rm = create_pers_region_map(env, metadata_heap);

    Genode::log("Attaching persistent RM to local address space");
    void *address = env.rm().attach_at(persistent_rm.dataspace(), 0x11000000);
    Genode::log("Attached persistent RM to local address ", address);

    static Persalloc::Heap heap(env.ram(), persistent_rm, metadata_heap, Genode::addr_t(address));

    char *p = restore_app();
    if (nullptr != p) {
        Genode::log("Restoring");
        // restore
        std::ifstream heapdump;
        heapdump.open(heap_snap_path);
        unsigned long addr;
        Genode::size_t size;

        while (heapdump >> size >> addr) {
            Genode::log("addr ", (void *) addr, " size ", size);
            heap.alloc_addr(size, Persalloc::Heap::Region_map_address(addr));

            printf("Before read %lx %lu \n", addr, size);
            auto local_addr = (address + Genode::addr_t(addr));
            heapdump.read((char *) local_addr, size);
        }
        Genode::log("Heapdump read ok");
        heapdump.close();
    } else {
        Genode::log("Starting from scratch");
        // no snapshot
        p = new(heap) char[32];
    }
    Genode::log("Init/Restore done");

    printf("p address %p\n", p);
    printf("old p value %s\n", p);
    strcpy(p, "hello world");
    printf("new p value %s\n", p);

    auto q = new(heap) char[32];
    printf("q address %p\n", q);
    strcpy(q, "AHAHA OVERWRITE!");
    heap.free(q, 0);

    // save app
    std::ofstream app;
    app.open(app_snap_path);
    app << (Genode::addr_t) p;
    app.close();

    // save heap
    std::ofstream file;
    file.open(heap_snap_path);
    for (Persalloc::Heap::Dataspace const *ds = heap.ds_pool().first(); ds; ds = ds->next()) {
        auto local_addr = (address + Genode::addr_t(ds->local_addr));
        file << ds->size << " " << (unsigned long) ds->local_addr << " ";
        file.write((char *) local_addr, ds->size);
        Genode::log("addr ", ds->local_addr, " size ", ds->size);
    }

    file.close();
    return 0;
}

void Libc::Component::construct(Libc::Env &env) {
    with_libc([&] { my_main(env); });
}
