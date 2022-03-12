#include <vfs_ptcp/save.h>
//#include <lwip/ip_addr.h>

void save_ip(const ip_addr &addr, std::ostream &out) {
    out << addr.type << " ";
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

void save_lwip_state(std::ostream &out, const Ptcp::Snapshot::Lwip_state &state) {
    tcp_pcb *pcb = state.bound_pcbs;
    while (pcb != nullptr) {
        save_ip(pcb->local_ip, out);
        save_ip(pcb->remote_ip, out);
        out << pcb->netif_idx << " ";
        out << pcb->so_options << " ";
        out << pcb->tos << " ";
        out << pcb->ttl << " ";
        // pcb->next;
        // pcb->callback_arg;
        // #if LWIP_TCP_PCB_NUM_EXT_ARGS
        // struct tcp_pcb_ext_args ext_args
        // #endif
        // #if LWIP_TCP_PCB_NUM_EXT_ARGS
        // struct tcp_pcb_ext_args ext_args
        // #endif
        out << pcb->state << " ";
        out << pcb->prio << " ";
        out << pcb->local_port << " ";
        out << pcb->remote_port << " ";
        out << pcb->flags << " ";
        out << pcb->polltmr << " ";
        out << pcb->pollinterval << " ";
        out << pcb->last_timer << " ";
        out << pcb->tmr << " ";
        out << pcb->rcv_nxt << " ";
        out << pcb->rcv_wnd << " ";
        out << pcb->rcv_ann_wnd << " ";
        out << pcb->rcv_ann_right_edge << " ";
        //#if LWIP_TCP_SACK_OUT
        //        /* SACK ranges to include in ACK packets (entry is invalid if left==right) */
        //  struct tcp_sack_range rcv_sacks[LWIP_TCP_MAX_SACK_NUM];
        //#define LWIP_TCP_SACK_VALID(pcb, idx) ((pcb)->rcv_sacks[idx].left != (pcb)->rcv_sacks[idx].right)
        //#endif /* LWIP_TCP_SACK_OUT */
        out << pcb->rtime << " ";
        out << pcb->mss << " ";
        out << pcb->rttest << " ";
        out << pcb->rtseq << " ";
        out << pcb->sa << " ";
        out << pcb->sv << " ";
        out << pcb->rto << " ";
        out << pcb->nrtx << " ";
        out << pcb->dupacks << " ";
        out << pcb->lastack << " ";
        out << pcb->cwnd << " ";
        out << pcb->ssthresh << " ";
        out << pcb->rto_end << " ";
        out << pcb->snd_nxt << " ";
        out << pcb->snd_wl1 << " ";
        out << pcb->snd_wl2 << " ";
        out << pcb->snd_lbb << " ";
        out << pcb->snd_wnd << " ";
        out << pcb->snd_wnd_max << " ";
        out << pcb->snd_buf << " ";
        out << pcb->snd_queuelen << " ";
#if TCP_OVERSIZE
        out << pcb->unsent_oversize << " ";
#endif /* TCP_OVERSIZE */
        out << pcb->bytes_acked << " ";
        out << pcb->unsent << " ";
        out << pcb->unacked << " ";
#if TCP_QUEUE_OOSEQ
        out << pcb->ooseq << " ";
#endif /* TCP_QUEUE_OOSEQ */
        out << pcb->refused_data << " ";
#if LWIP_CALLBACK_API || TCP_LISTEN_BACKLOG
        // out << pcb->listener << " ";
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
        out << pcb -> ts_lastacksent << " ";
        out << pcb -> ts_recent << " ";
#endif /* LWIP_TCP_TIMESTAMPS */
        out << pcb->keep_idle << " ";
#if LWIP_TCP_KEEPALIVE
        out << pcb->keep_intvl << " ";
        out << pcb->keep_cnt << " ";
#endif /* LWIP_TCP_KEEPALIVE */

        out << pcb->persist_cnt << " ";
        out << pcb->persist_backoff << " ";
        out << pcb->persist_probe << " ";
        out << pcb->keep_cnt_sent << " ";

#if LWIP_WND_SCALE
        out << pcb->snd_scale << " ";
        out << pcb->rcv_scale << " ";
#endif

        out << std::endl;
        pcb = pcb->next;
    }
}