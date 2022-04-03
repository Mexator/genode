#include <vfs_ptcp/save.h>
#include <logging/mylog.h>

void save_state(const Ptcp::Snapshot::Composed_state &state) {
    debug_log(LOG_SAVE, __func__);
    std::ofstream output;
    output.open(SNAPSHOT_FILENAME);
    output << SNAPSHOT_VERSION << std::endl;

    debug_log(LOG_SAVE, "Saving libc");
    output << "libc" << std::endl;
    save_libc_state(output, state.libc_state);
    debug_log(LOG_SAVE, "Saved libc");

    output << "lwip" << std::endl;
    debug_log(LOG_SAVE, "Saving lwip");
    save_lwip_state(output, state.lwip_state);
    debug_log(LOG_SAVE, "Saved lwip");

    output.close();
}
