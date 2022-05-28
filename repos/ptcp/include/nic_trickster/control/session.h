#ifndef _PTCP_SESSION_H_
#define _PTCP_SESSION_H_

#include <session/session.h>
#include <base/connection.h>
#include <nic_trickster/control/tracker_delegate.h>

namespace Nic_control {
    using Genode::Dataspace_capability;
    using Genode::size_t;

    class Session;

    class Session_client;

    class Connection;
}

struct Nic_control::Session : Genode::Session {
    static const char *service_name() { return "Nic_control"; }

    // No caps needed
    static const int CAP_QUOTA = 0;

    // Suspend transferring packets from and to clients until resume call
    virtual void suspend() = 0;

    // Resume transferring packets from and to clients
    virtual void resume() = 0;

    /**
     * Starts a restore mode. In this mode each SYN/ACK packet sent from client is filtered out.
     * These packets will not be handed to Uplink clients
     * @param enabled
     */
    virtual void set_restore_mode(bool enabled) = 0;

    // If the ethernet frame located in the provided dataspace is TCP segment,
    // shift its SEQ to [expected_seq] and then sends the packet to to client of Nic session
    virtual void send_offset_packet(size_t len, Dataspace_capability cap, uint32_t expected_seq) = 0;

    // Calculates and sets SEQ offset for the last established socket. All packets from host to remote will
    // have SEQ shifted to this offset
    virtual void calculate_offsets(uint32_t old_ack) = 0;

    class Id_not_found : Genode::Exception {
    };

    virtual Nic_socket_metadata
    get_md_value(Nic_socket_id id, Dataspace_capability cap, Dataspace_capability ackCap) = 0;

    GENODE_RPC(Rpc_suspend, void, suspend);

    GENODE_RPC(Rpc_resume, void, resume);

    GENODE_RPC(Rpc_set_restore_mode, void, set_restore_mode, bool);

    GENODE_RPC(Rpc_send_offset_packet, void, send_offset_packet, size_t, Dataspace_capability, uint32_t);

    GENODE_RPC(Rpc_calculate_offsets, void, calculate_offsets, uint32_t);

    GENODE_RPC(Rpc_get_md_value, Nic_socket_metadata, get_md_value, Nic_socket_id, Dataspace_capability,
               Dataspace_capability);

    GENODE_RPC_INTERFACE(Rpc_suspend,
                         Rpc_resume,
                         Rpc_set_restore_mode,
                         Rpc_send_offset_packet,
                         Rpc_calculate_offsets,
                         Rpc_get_md_value);
};

/** Client implementation for [Nic_control] */
struct Nic_control::Session_client : Genode::Rpc_client<Session> {
    Session_client(Genode::Capability<Session> cap)
            : Genode::Rpc_client<Session>(cap) {}

    void suspend() override { call<Rpc_suspend>(); }

    void resume() override { call<Rpc_resume>(); }

    void set_restore_mode(bool enabled) override { call<Rpc_set_restore_mode>(enabled); };

    void send_offset_packet(size_t len, Dataspace_capability cap, uint32_t expected_seq) override {
        call<Rpc_send_offset_packet>(len, cap, expected_seq);
    }

    void calculate_offsets(uint32_t old_ack) override { call<Rpc_calculate_offsets>(old_ack); }

    Nic_socket_metadata get_md_value(
            Nic_socket_id id,
            Dataspace_capability cap,
            Dataspace_capability ackCap) override {

        return call<Rpc_get_md_value>(id, cap, ackCap);
    }
};

struct Nic_control::Connection : Genode::Connection<Session>, Nic_control::Session_client {
    Connection(Genode::Env &env) :
            Genode::Connection<Session>(
                    env,
                    session(env.parent(), "ram_quota=6K, cap_quota=0")
            ),
            Session_client(cap()) {}
};

#endif //_PTCP_SESSION_H_
