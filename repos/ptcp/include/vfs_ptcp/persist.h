#ifndef _PTCP_PERSIST_H_
#define _PTCP_PERSIST_H_

namespace Ptcp {
    namespace Snapshot {
        struct Composed_state;
    }
}

void persist_saved_state(Ptcp::Snapshot::Composed_state state);

#endif //_PTCP_PERSIST_H_
