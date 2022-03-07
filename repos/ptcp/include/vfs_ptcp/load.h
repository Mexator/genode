#ifndef _PTCP_LOAD_H_
#define _PTCP_LOAD_H_

// Genode includes
#include <base/allocator.h>

namespace Ptcp {

    namespace Snapshot {
        class Load_manager {
        private:
            Genode::Allocator &_alloc;

        public:
            explicit Load_manager(Genode::Allocator &alloc) : _alloc{alloc} {}

        public:
            void load_libc_state();
        };
    }
}

#endif //_PTCP_LOAD_H_
