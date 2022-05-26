#ifndef _STOPPER_H_
#define _STOPPER_H_

// Genode includes
#include <base/env.h>
#include <base/mutex.h>
#include <base/component.h>
#include <base/rpc_server.h>
#include <root/component.h>

// Local includes
#include "session.h"
#include <nic_trickster/interface.h>

extern Net::Interface *to_local;
extern Net::Interface *to_remote;

using namespace Nic_control;

// Server RPC object implementation for Nic_control
class Nic_control_impl : public Genode::Rpc_object<Nic_control::Session> {
    Genode::Env &_env;
public:
    Nic_control_impl(Genode::Env &env) : _env(env) {}

    static Genode::Mutex mutex;
    static bool is_restore;

    void suspend() override;

    void resume() override;

    void set_restore_mode(bool enabled) override;

    void send_packet(size_t len, Dataspace_capability cap) override;

    Nic_socket_metadata get_md_value(Nic_socket_id id, Dataspace_capability cap, Dataspace_capability ackCap) override;
};

class Stopper_root : public Genode::Root_component<Nic_control_impl> {
    Genode::Env &_env;
protected:
    Nic_control_impl *_create_session(const char *) override {
        return new(md_alloc()) Nic_control_impl(_env);
    }

public:
    Stopper_root(Genode::Env &env, Genode::Entrypoint &ep, Genode::Allocator &alloc) :
            Genode::Root_component<Nic_control_impl>(ep, alloc),
            _env(env) {}
};

#endif //_STOPPER_H_
