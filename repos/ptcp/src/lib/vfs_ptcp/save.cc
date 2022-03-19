#include <vfs_ptcp/save.h>

void save_state(const Ptcp::Snapshot::Composed_state &state) {
    std::ofstream output;
    output.open(SNAPSHOT_FILENAME);
    output << SNAPSHOT_VERSION << std::endl;

    output << "libc" << std::endl;
    save_libc_state(output, state.libc_state);

    output << "lwip" << std::endl;
    save_lwip_state(output, state.lwip_state);

    output.close();
}
