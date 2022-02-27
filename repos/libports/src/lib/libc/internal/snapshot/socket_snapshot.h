//
// Created by anton on 27.02.2022.
//

#ifndef GENODE_SOCKET_SNAPSHOT_H
#define GENODE_SOCKET_SNAPSHOT_H

#define SO_INTERNAL_STATE 0x00000011

struct socket_state {
    unsigned int proto;
    unsigned int state;
};

#endif //GENODE_SOCKET_SNAPSHOT_H
