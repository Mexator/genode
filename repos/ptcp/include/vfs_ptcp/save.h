#ifndef _PTCP_SAVE_H_
#define _PTCP_SAVE_H_

#include <fstream>
#include <vfs_ptcp/snapshot.h>
#include <vfs_ptcp/persist.h>

void save_libc_state(std::ostream &out, const Ptcp::Snapshot::Libc::Plugin_state &state);

void save_lwip_state(std::ostream &out, const Ptcp::Snapshot::Lwip_state &state);

void save_state(const Ptcp::Snapshot::Composed_state &state);

#endif //_PTCP_SAVE_H_
