// Libcxx includes
#include <fstream>
#include <istream>

// PTCP includes
#include <vfs_ptcp/persist.h>
#include <vfs_ptcp/load.h>

using namespace Ptcp::Snapshot;

void Load_manager::read_snapshot_file() {
    // This function should be called after all libc plugins are initialized,
    // because it access VFS using C++ stream api.
    std::ifstream input;
    input.open(SNAPSHOT_FILENAME);

    int version;
    input >> version;
    if (version != SNAPSHOT_VERSION) {
        Genode::warning("Incompatible snapshot version: ", version);
        return;
    }

    std::string header;

    input >> header;
    Libc::Plugin_state libc_state = LIBC_EMPTY;
    if (header == "libc") {
        libc_state = read_libc_state(input, _alloc);
    } else {
        Genode::error("Check snapshot format! Missing libc header");
    }

    input >> header;
    Lwip_state lwip_state = LWIP_EMPTY;
    if (header == "lwip") {
        lwip_state = read_lwip_state(input, _alloc);
    } else {
        Genode::error("Check snapshot format! Missing lwip header");
    }

    input.close();
    Genode::log("Finished reading state");
    _state = new(_alloc) Composed_state{
            libc_state,
            lwip_state
    };
}

void Load_manager::restore_state() {
    if (_state == nullptr) return;
    // Composed state should be initialized at this point
    Genode::log("Starting libc state restoration");
    restore_libc_state(_state->libc_state,_alloc);
}

