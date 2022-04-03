// PTCP includes
#include <vfs_ptcp/load.h>

#include <cstring>

using namespace Ptcp::Snapshot;

Lwip_state read_lwip_state(std::istream &input, Genode::Allocator &alloc) {
    int dataspaces_number;
    input >> dataspaces_number;
    debug_log(LOG_LOAD, dataspaces_number, " dataspaces");
    Lwip_state::Dataspace *dataspaces = new(alloc) Lwip_state::Dataspace[dataspaces_number];

    for (int i = 0; i < dataspaces_number; i++) {
        debug_log(VERBOSE_LOAD, "position ", (void *) (long) input.tellg());
        input >> dataspaces[i].size >> dataspaces[i].addr;
        input.seekg(1, std::ios::cur);

        debug_log(VERBOSE_LOAD, "Dataspace ", i, " ", dataspaces[i].size, " ", dataspaces[i].addr);

        dataspaces[i].content = alloc.alloc(dataspaces[i].size);
        input.read((char *) dataspaces[i].content, dataspaces[i].size);
    }
    return Lwip_state{
            dataspaces,
            dataspaces_number,
            0
    };
}

void restore_lwip_state(const Ptcp::Snapshot::Lwip_state &state, Persalloc::Heap &heap) {
    debug_log(LOG_LOAD, __func__);
    for (int i = 0; i < state.dataspaces_number; i++) {
        auto &ds = state.dataspaces[i];

        heap.alloc_addr(ds.size, ds.addr);
        std::memcpy(heap.region_addr_to_local(ds.addr), ds.content, ds.size);
    }
    debug_log(LOG_LOAD, __func__ ," done");
}