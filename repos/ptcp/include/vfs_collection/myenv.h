#ifndef _PTCP_MYENV_H_
#define _PTCP_MYENV_H_

#include <vfs/simple_env.h>

class TwoAllocEnv : public Vfs::Simple_env {
private:
    Genode::Allocator &_persalloc;

public:
    TwoAllocEnv(Genode::Env &env,
                Genode::Allocator &transalloc,
                Genode::Allocator &persalloc,
                Genode::Xml_node config)
            : Vfs::Simple_env(env, transalloc, config), _persalloc{persalloc} {}

    Genode::Allocator &persalloc() { return _persalloc; }
};

#endif //_PTCP_MYENV_H_
