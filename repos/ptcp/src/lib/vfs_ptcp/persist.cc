#include <base/log.h>
#include <vfs_ptcp/persist.h>
#include <vfs_ptcp/snapshot.h>
#include <ostream>
#include <fstream>
#include <cerrno>

using namespace std;

void persist_saved_state(Ptcp::Snapshot::Composed_state state) {
    ofstream output;
    output.open("/snapshot/saved.txt");

    Ptcp::Snapshot::Libc_plugin_state &libc_state = state.libc_state;
    output << libc_state.sockets_number << endl;

    for (size_t i = 0; i < libc_state.sockets_number; ++i) {
        socket_state sock_state = libc_state.socket_states[i];
        output << sock_state.proto << " " << sock_state.state << endl;
    }
    output.close();
}