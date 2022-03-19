#ifndef _SNAPSHOT_NOTIFIER_SESSION_H_
#define _SNAPSHOT_NOTIFIER_SESSION_H_

#include <base/rpc.h>
#include <session/session.h>
#include <base/signal.h>

namespace Snapshot_notifier { struct Session; }

struct Snapshot_notifier::Session : Genode::Session {

    /**
     * \noapi
     */
    static const char *service_name() { return "SIG_SNAP"; }

    enum {
        CAP_QUOTA = 2
    };

    virtual void add_handler(Genode::Signal_context_capability cap) = 0;

    /*******************
    ** RPC interface **
    *******************/

    GENODE_RPC(Rpc_add_handler, void, add_handler, Genode::Signal_context_capability);

    GENODE_RPC_INTERFACE(Rpc_add_handler);
};

#endif