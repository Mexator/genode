#ifndef _PTCP_SOCKET_SNAPSHOT_H_
#define _PTCP_SOCKET_SNAPSHOT_H_

#define SO_INTERNAL_STATE 0x00000011

struct socket_state {
    unsigned int proto;
    unsigned int state;
};

#endif //_PTCP_SOCKET_SNAPSHOT_H_
