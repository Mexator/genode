
#ifndef _PTCP_SNAPSHOT_H_
#define _PTCP_SNAPSHOT_H_

#include <lwip/tcp.h>

struct Lwip_snapshot {
    tcp_pcb* tcp_bound_pcbs;
};

struct Lwip_snapshot form_snapshot();

#endif //_PTCP_SNAPSHOT_H_
