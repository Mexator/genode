#include <base/heap.h>
#include <base/shared_object.h>
#include <vfs/file_system_factory.h>
#include <vfs/simple_env.h>
#include <rm_session/connection.h>
#include <region_map/client.h>
#include <persalloc/pers_heap.h>

#include <vfs_ptcp/sig_handlers.h>
#include <vfs_ptcp/snapshot.h>
#include <vfs_ptcp/load.h>
#include <vfs_ptcp/lwip_wrapper.h>
#include <vfs_collection/myenv.h>

constexpr Genode::size_t SIZE = 1024 * 1024 * 16;

Genode::Region_map &create_pers_region_map(Genode::Env &env, Genode::Allocator &alloc) {
    static Genode::Rm_connection rm(env);

    Genode::log("Creating region map with size ", SIZE);
    Genode::Region_map_client *region_map = new(alloc)Genode::Region_map_client(rm.create(SIZE));

    return *region_map;
}

extern "C" Vfs::File_system_factory *vfs_file_system_factory(void) {

    struct Factory : Vfs::File_system_factory {
        Vfs::File_system_factory *lwip_factory = nullptr;

        Vfs::File_system_factory *get_lwip_factory(Vfs::Env &vfs_env) {
            if (lwip_factory == nullptr)
                load_lwip_factory(vfs_env.env());
            return lwip_factory;
        }

        Vfs::File_system *create(Vfs::Env &vfs_env, Genode::Xml_node config) override {
            auto &env = vfs_env.env();
            static Genode::Sliced_heap metadata_heap(env.ram(), env.rm());
            static Genode::Region_map &persistent_rm = create_pers_region_map(env, metadata_heap);
            void *address = env.rm().attach_at(persistent_rm.dataspace(), 0x11000000);
            Persalloc::Heap *heap = new(vfs_env.alloc()) Persalloc::Heap(
                    env.ram(),
                    persistent_rm,
                    metadata_heap,
                    Genode::addr_t(address));
            Genode::warning("Heap init");
            Ptcp::Snapshot::set(heap);

            static TwoAllocEnv lwip_env(vfs_env.env(), vfs_env.alloc(), *heap, config);

            Vfs::File_system *lwip_fs = get_lwip_factory(vfs_env)->create(lwip_env, config);
            // Create load manager
            Ptcp::Snapshot::Load_manager manager{vfs_env.alloc()};
            // Create FS
            Ptcp::Vfs_wrapper *wrapper = new(vfs_env.alloc()) Ptcp::Vfs_wrapper(
                    vfs_env,
                    *lwip_fs,
                    manager
            );
            // Enable processing of SIG_SNAP
            setup_sig_handlers(vfs_env.env());

            return wrapper;
        }

        // XXX: I think this is a shitty approach to load binary. I wish there was a better solution...
        void load_lwip_factory(Genode::Env &env) {
            Genode::Sliced_heap h(env.ram(), env.rm());
            Genode::Shared_object obj(env,
                                      h,
                                      "vfs_lwip_dl.lib.so",
                                      Genode::Shared_object::BIND_LAZY,
                                      Genode::Shared_object::KEEP);
            typedef Vfs::File_system_factory *(*Query_fn)();
            Query_fn f = obj.lookup<Query_fn>("vfs_file_system_factory");
            lwip_factory = f();
        }
    };

    static Factory f;
    return &f;
}
