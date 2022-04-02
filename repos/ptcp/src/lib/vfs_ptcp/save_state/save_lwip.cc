#include <vfs_ptcp/save.h>

void save_lwip_state(std::ostream &out, const Ptcp::Snapshot::Lwip_state &state) {
    for (size_t i = 0; i < state.dataspaces_number; ++i) {
        Ptcp::Snapshot::Lwip_state::Dataspace ds = state.dataspaces[i];
        out << ds.size << " ";
        out << ds.addr << " ";
        out << std::endl;
        out.write((char *) ds.addr, ds.size);
    }
}