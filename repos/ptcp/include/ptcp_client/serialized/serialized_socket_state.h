#ifndef _PTCP_SERIALIZED_SOCKET_STATE_H_
#define _PTCP_SERIALIZED_SOCKET_STATE_H_

// Genode includes
#include <base/output.h>

// Libc includes
#include <iostream>

/**
 * Socket structure as it is persisted on disk
 */
struct serialized_socket {
    unsigned long pfd;
    char *boundAddress;

    void save(std::ostream &out);

    static serialized_socket load(std::istream &in);

    void print(Genode::Output &out) const;
};


#endif //_PTCP_SERIALIZED_SOCKET_STATE_H_
