#ifndef _PTCP_SERIALIZED_SOCKET_STATE_H_
#define _PTCP_SERIALIZED_SOCKET_STATE_H_

// Genode includes
#include <base/output.h>

// Libc includes
#include <iostream>

// PTCP includes
#include <ptcp_client/socket_state.h>

/**
 * Socket structure as it is persisted on disk
 */
struct serialized_socket {
    unsigned long pfd;
    tcp_meta_state state;
    char *boundAddress;
    char *remoteAddress;
    int syn_packet_len;
    char *syn_packet;
    int ack_packet_len;
    char *ack_packet;

    void save(std::ostream &out);

    static serialized_socket& load(std::istream &in);

    void print(Genode::Output &out) const;
};


#endif //_PTCP_SERIALIZED_SOCKET_STATE_H_
