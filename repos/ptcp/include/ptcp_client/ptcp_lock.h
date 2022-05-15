#ifndef _PTCP_PTCP_LOCK_H_
#define _PTCP_PTCP_LOCK_H_

#include <base/mutex.h>

namespace Ptcp {
    // Mutex to stop VFS during snapshot
    extern Genode::Mutex mutex;
}

#endif //_PTCP_PTCP_LOCK_H_
