#ifndef _STOPPER_H_
#define _STOPPER_H_

#include <base/mutex.h>
#include <base/component.h>
#include <base/rpc_server.h>
#include <root/component.h>
#include "session.h"

class Stopper : public Genode::Rpc_object<Nic_control::Session> {
public:
    static Genode::Mutex mutex;

    void suspend() override;

    void resume() override;
};

class Stopper_root : public Genode::Root_component<Stopper> {
protected:
    Stopper *_create_session(const char *) override {
        return new(md_alloc()) Stopper();
    }

public:
    Stopper_root(Genode::Entrypoint &ep, Genode::Allocator &alloc) :
            Genode::Root_component<Stopper>(ep, alloc) {}
};

#endif //_STOPPER_H_
