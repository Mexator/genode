#include <vfs_ptcp/save.h>
#include <logging/mylog.h>

void save_lwip_state(std::ostream &out, const Ptcp::Snapshot::Lwip_state &state) {
    debug_log(LOG_SAVE_LWIP, "Heap attach_addr", (void *)state.heap_attach_addr);
    debug_log(LOG_SAVE_LWIP, "Dataspaces number", state.dataspaces_number);

    out << state.dataspaces_number << std::endl;
    for (size_t i = 0; i < state.dataspaces_number; ++i) {
        Ptcp::Snapshot::Lwip_state::Dataspace ds = state.dataspaces[i];
        out << ds.size << " ";
        out << ds.addr << " ";
        debug_log(LOG_SAVE_LWIP_VERBOSE, "Dataspace addr ", ds.addr, " size ", ds.size);
        out.write((char *) (state.heap_attach_addr + ds.addr), ds.size);
    }
}