#ifndef _PTCP_LOAD_H_
#define _PTCP_LOAD_H_

// Genode includes
#include <base/allocator.h>

// Ptcp includes
#include <vfs_ptcp/snapshot.h>

namespace Ptcp {

    namespace Snapshot {
        class Load_manager {
        private:
            Genode::Allocator &_alloc;
            Composed_state* _state = nullptr;

        public:
            explicit Load_manager(Genode::Allocator &alloc) : _alloc{alloc} {}

        public:
            void read_snapshot_file();

            void load_libc_state();
        };
    }
}

#endif //_PTCP_LOAD_H_
