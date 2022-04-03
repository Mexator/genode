// Libcxx includes
#include <fstream>
#include <istream>

// PTCP includes
#include <vfs_ptcp/persist.h>
#include <vfs_ptcp/load.h>
#include <vfs_ptcp/save.h>

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

    debug_log(LOG_LOAD, "Reading libc state");
    input >> header;
    Libc::Plugin_state libc_state = LIBC_EMPTY;
    if (header == "libc") {
        libc_state = read_libc_state(input, _alloc);
    } else {
        Genode::error("Check snapshot format! Missing libc header");
    }
    debug_log(LOG_LOAD, "finish reading libc state");

    debug_log(LOG_LOAD, "Reading lwip state");
    input >> header;
    Lwip_state lwip_state = LWIP_EMPTY;
    if (header == "lwip") {
        lwip_state = read_lwip_state(input, _alloc);
    } else {
        Genode::error("Check snapshot format! Missing lwip header");
    }
    debug_log(LOG_LOAD, "Read lwip state");

    input.close();
    debug_log(LOG_LOAD, "Finished reading state");
    _state = new(_alloc) Composed_state{
            libc_state,
            lwip_state
    };
}

void Load_manager::restore_state() {
    debug_log(LOG_LOAD, __func__);
    if (_state == nullptr) return;
    // Composed state should be initialized at this point
    debug_log(LOG_LOAD, "Starting libc state restoration");
    restore_libc_state(_state->libc_state, _alloc);
    debug_log(LOG_LOAD, "Starting lwip state restoration");
    restore_lwip_state(_state->lwip_state, *Ptcp::Snapshot::get());
    debug_log(LOG_LOAD, "State restored");
}

