// Genode includes
#include <base/log.h>

#include <vfs_ptcp/persist.h>
#include <vfs_ptcp/snapshot.h>

// Libcxx includes
#include <ostream>
#include <fstream>
#include <cerrno>

// Lwip includes
#include <lwip/ip_addr.h>

using namespace std;

void save_ip(const ip_addr &addr, ostream &out);

void persist_saved_state(Ptcp::Snapshot::Composed_state state) {
    ofstream output;
    output.open("/snapshot/saved.txt");

    output << "libc" << endl;

    Ptcp::Snapshot::Libc_plugin_state &libc_state = state.libc_state;
    output << libc_state.sockets_number << endl;

    for (size_t i = 0; i < libc_state.sockets_number; ++i) {
        socket_state sock_state = libc_state.socket_states[i];
        output << sock_state.proto << " " << sock_state.state << endl;
    }

    output << "lwip" << endl;

    // lwip
    tcp_pcb *pcb = state.lwip_state.bound_pcbs;
    while (pcb != nullptr) {
        save_ip(pcb->local_ip, output);
        save_ip(pcb->remote_ip, output);
        output << pcb->netif_idx << " ";
        output << pcb->so_options << " ";
        output << pcb->tos << " ";
        output << pcb->ttl << " ";
        // pcb->next;
        // pcb->callback_arg;
        // #if LWIP_TCP_PCB_NUM_EXT_ARGS
        // struct tcp_pcb_ext_args ext_args
        // #endif
        // #if LWIP_TCP_PCB_NUM_EXT_ARGS
        // struct tcp_pcb_ext_args ext_args
        // #endif
        output << pcb->state << " ";
        output << pcb->prio << " ";
        output << pcb->local_port << " ";
        output << pcb->remote_port << " ";
        output << pcb->flags << " ";
        output << pcb->polltmr << " ";
        output << pcb->pollinterval << " ";
        output << pcb->last_timer << " ";
        output << pcb->tmr << " ";
        output << pcb->rcv_nxt << " ";
        output << pcb->rcv_wnd << " ";
        output << pcb->rcv_ann_wnd << " ";
        output << pcb->rcv_ann_right_edge << " ";
        //#if LWIP_TCP_SACK_OUT
        //        /* SACK ranges to include in ACK packets (entry is invalid if left==right) */
        //  struct tcp_sack_range rcv_sacks[LWIP_TCP_MAX_SACK_NUM];
        //#define LWIP_TCP_SACK_VALID(pcb, idx) ((pcb)->rcv_sacks[idx].left != (pcb)->rcv_sacks[idx].right)
        //#endif /* LWIP_TCP_SACK_OUT */
        output << pcb->rtime << " ";
        output << pcb->mss << " ";
        output << pcb->rttest << " ";
        output << pcb->rtseq << " ";
        output << pcb->sa << " ";
        output << pcb->sv << " ";
        output << pcb->rto << " ";
        output << pcb->nrtx << " ";
        output << pcb->dupacks << " ";
        output << pcb->lastack << " ";
        output << pcb->cwnd << " ";
        output << pcb->ssthresh << " ";
        output << pcb->rto_end << " ";
        output << pcb->snd_nxt << " ";
        output << pcb->snd_wl1 << " ";
        output << pcb->snd_wl2 << " ";
        output << pcb->snd_lbb << " ";
        output << pcb->snd_wnd << " ";
        output << pcb->snd_wnd_max << " ";
        output << pcb->snd_buf << " ";
        output << pcb->snd_queuelen << " ";
        #if TCP_OVERSIZE
            output << pcb->unsent_oversize << " ";
        #endif /* TCP_OVERSIZE */
        output << pcb->bytes_acked << " ";
        output << pcb->unsent << " ";
        output << pcb->unacked << " ";
        #if TCP_QUEUE_OOSEQ
            output << pcb->ooseq << " ";
        #endif /* TCP_QUEUE_OOSEQ */
        output << pcb->refused_data << " ";
        #if LWIP_CALLBACK_API || TCP_LISTEN_BACKLOG
         // output << pcb->listener << " ";
        #endif /* LWIP_CALLBACK_API || TCP_LISTEN_BACKLOG */
        #if LWIP_CALLBACK_API
        //        /* Function to be called when more send buffer space is available. */
        //        tcp_sent_fn sent;
        //        /* Function to be called when (in-sequence) data has arrived. */
        //        tcp_recv_fn recv;
        //        /* Function to be called when a connection has been set up. */
        //        tcp_connected_fn connected;
        //        /* Function which is called periodically. */
        //        tcp_poll_fn poll;
        //        /* Function to be called whenever a fatal error occurs. */
        //        tcp_err_fn errf;
        #endif /* LWIP_CALLBACK_API */

        #if LWIP_TCP_TIMESTAMPS
        output << pcb -> ts_lastacksent << " ";
        output << pcb -> ts_recent << " ";
        #endif /* LWIP_TCP_TIMESTAMPS */
        output << pcb->keep_idle << " ";
        #if LWIP_TCP_KEEPALIVE
        output << pcb->keep_intvl << " ";
        output << pcb->keep_cnt << " ";
        #endif /* LWIP_TCP_KEEPALIVE */

        output << pcb->persist_cnt << " ";
        output << pcb->persist_backoff << " ";
        output << pcb->persist_probe << " ";
        output << pcb->keep_cnt_sent << " ";

        #if LWIP_WND_SCALE
        output << pcb->snd_scale << " ";
        output << pcb->rcv_scale << " ";
        #endif

        output << endl;
        pcb = pcb->next;
    }

    output.close();
}

void save_ip(const ip_addr &addr, ostream &out) {
    switch (addr.type) {
        case IPADDR_TYPE_V6:
            out << addr.u_addr.ip6.addr;
            out << " ";
            out << addr.u_addr.ip6.zone;
            break;
        case IPADDR_TYPE_V4:
        default:
            out << addr.u_addr.ip4.addr;
    }
    out << " ";
}