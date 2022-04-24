#ifndef _PTCP_SERIALIZED_SOCKET_STATE_H_
#define _PTCP_SERIALIZED_SOCKET_STATE_H_

// Libc includes
#include <iostream>

/**
 * Socket structure as it is persisted on disk
 */
struct serialized_socket {
    unsigned long pfd;

    void save(std::ostream &out);

    static serialized_socket load(std::istream &in);
};


#endif //_PTCP_SERIALIZED_SOCKET_STATE_H_
