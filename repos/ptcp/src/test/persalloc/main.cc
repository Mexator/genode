#include <libc/component.h>
#include <persalloc/pers_heap.h>

int my_main(Libc::Env &env) {
    Persalloc::Heap heap(env.ram(), env.rm());
    int *p = (int *) heap.alloc(sizeof(int));
    Genode::warning(*p);
    *p = 69; // lmao
    Genode::warning(*p);
    return 0;
}

void Libc::Component::construct(Libc::Env &env) {
    with_libc([&] { my_main(env); });
}
