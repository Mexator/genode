// Genode includes
#include <base/heap.h>
#include <base/shared_object.h>

#include <vfs/file_system_factory.h>

// PTCP includes
#include <vfs_ptcp/lwip_wrapper.h>

extern "C" Vfs::File_system_factory *vfs_file_system_factory(void) {

    struct Factory : Vfs::File_system_factory {
        Vfs::File_system_factory *lwip_factory = nullptr;

        Vfs::File_system_factory *get_lwip_factory(Vfs::Env &vfs_env) {
            if (lwip_factory == nullptr)
                load_lwip_factory(vfs_env);

            return lwip_factory;
        }

        Vfs::File_system *create(Vfs::Env &vfs_env, Genode::Xml_node config) override {
            Vfs::File_system *lwip_fs = get_lwip_factory(vfs_env)->create(vfs_env, config);

            Ptcp::Vfs_wrapper *wrapper = new(vfs_env.alloc()) Ptcp::Vfs_wrapper(
                    vfs_env,
                    *lwip_fs
            );
            return wrapper;
        }

        void load_lwip_factory(Vfs::Env &env) {
            Genode::Shared_object obj(env.env(),
                                      env.alloc(),
                                      "vfs_lwip.lib.so",
                                      Genode::Shared_object::BIND_NOW,
                                      Genode::Shared_object::KEEP);
            typedef Vfs::File_system_factory *(*Query_fn)();
            Query_fn f = obj.lookup<Query_fn>("vfs_file_system_factory");
            lwip_factory = f();
        }
    };

    static Factory f;
    return &f;
}
