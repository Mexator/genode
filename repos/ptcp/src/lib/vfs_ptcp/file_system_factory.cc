#include <base/heap.h>
#include <base/shared_object.h>
#include <vfs/file_system_factory.h>

#include <vfs_ptcp/sig_handlers.h>

extern "C" Vfs::File_system_factory *vfs_file_system_factory(void) {

    struct Factory : Vfs::File_system_factory {
        Vfs::File_system_factory *lwip_factory = nullptr;

        Vfs::File_system *create(Vfs::Env &vfs_env, Genode::Xml_node config) override {
            setup_sig_handlers(vfs_env.env());

            if (lwip_factory == nullptr)
                load_lwip_factory(vfs_env.env());
            return lwip_factory->create(vfs_env, config);
        }

    private:
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
