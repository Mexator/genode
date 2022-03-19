#ifndef _PTCP_LOAD_H_
#define _PTCP_LOAD_H_

// Genode includes
#include <base/allocator.h>

// Stdcxx includes
#include <iostream>

// Ptcp includes
#include <vfs_ptcp/snapshot.h>

namespace Ptcp {

    namespace Snapshot {
        class Load_manager {
        private:
            Genode::Allocator &_alloc;
            Composed_state *_state = nullptr;

        public:
            explicit Load_manager(Genode::Allocator &alloc) : _alloc{alloc} {}

        public:
            void read_snapshot_file();

            void restore_state();
        };
    }
}

Ptcp::Snapshot::Libc::Plugin_state read_libc_state(std::istream &input, Genode::Allocator &alloc);
void restore_libc_state(const Ptcp::Snapshot::Libc::Plugin_state &state, Genode::Allocator &alloc);

Ptcp::Snapshot::Lwip_state read_lwip_state(std::istream &input, Genode::Allocator &alloc);

#endif //_PTCP_LOAD_H_
