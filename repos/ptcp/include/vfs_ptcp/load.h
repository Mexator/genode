#ifndef _PTCP_LOAD_H_
#define _PTCP_LOAD_H_

// Genode includes
#include <base/allocator.h>

// Stdcxx includes
#include <iostream>

// Ptcp includes
#include <vfs_ptcp/snapshot.h>
#include <logging/mylog.h>

#ifndef LOG_LOAD
#define LOG_LOAD true
#endif

#ifndef VERBOSE_LOAD
#define VERBOSE_LOAD false
#endif

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
void restore_lwip_state(const Ptcp::Snapshot::Lwip_state &state, Persalloc::Heap &heap);

#endif //_PTCP_LOAD_H_
