#include <ptcp_client/serialized/serialized_socket_state.h>

void serialized_socket::save(std::ostream &out) {
    out << pfd << " ";
    out << (unsigned) state << " ";
    out << (const char *) boundAddress << " ";
    out << (const char *) remoteAddress << " ";
    out << syn_packet_len << " ";
    out.write(syn_packet, syn_packet_len);
    out << " ";
    out << ack_packet_len << " ";
    out.write(ack_packet, ack_packet_len);
    out << " ";
    out << "\n";
}

serialized_socket &serialized_socket::load(std::istream &in) {
    serialized_socket &ret = *new serialized_socket;
    in >> ret.pfd;
    in >> (unsigned &) ret.state;
    in.seekg(1, std::ios_base::cur);
    if (ret.state >= BOUND) {
        ret.boundAddress = new char[MAX_ADDR_LEN];
        in.get(ret.boundAddress, MAX_ADDR_LEN, ' ');
    }
    in.seekg(1, std::ios_base::cur);
    if (ret.state == ESTABLISHED) {
        ret.remoteAddress = new char[MAX_ADDR_LEN];
        in.get(ret.remoteAddress, MAX_ADDR_LEN, ' ');

        in >> ret.syn_packet_len;
        in.seekg(1, std::ios_base::cur);
        ret.syn_packet = new char[ret.syn_packet_len];
        in.read(ret.syn_packet, ret.syn_packet_len);

        in >> ret.ack_packet_len;
        in.seekg(1, std::ios_base::cur);
        ret.ack_packet = new char[ret.ack_packet_len];
        in.read(ret.ack_packet, ret.ack_packet_len);
    }
    return ret;
}

void serialized_socket::print(Genode::Output &out) const {
    Genode::print(out, pfd, " ");
    Genode::print(out, (unsigned) state, " ");
    Genode::print(out, (const char *) boundAddress, " ");
    Genode::print(out, (const char *) remoteAddress, " ");
}
