#include <ptcp_client/serialized/serialized_socket_state.h>

void serialized_socket::save(std::ostream &out) {
    out << pfd << " ";
    out << (unsigned) state << " ";
    out << (const char *) boundAddress << " ";
    out << (const char *) remoteAddress << " ";
    out << "\n";
}

serialized_socket &serialized_socket::load(std::istream &in) {
    serialized_socket &ret = *new serialized_socket;
    in >> ret.pfd;
    in >> (unsigned &) ret.state;
    in.seekg(1, std::ios_base::cur);
    if (ret.state >= BOUND) {
        ret.boundAddress = new char[MAX_ADDR_LEN];
        in.getline(ret.boundAddress, MAX_ADDR_LEN);
    }
    in.seekg(1, std::ios_base::cur);
    if (ret.state == ESTABLISHED) {
        ret.remoteAddress = new char[MAX_ADDR_LEN];
        in.getline(ret.remoteAddress, MAX_ADDR_LEN);
    }
    return ret;
}

void serialized_socket::print(Genode::Output &out) const {
    Genode::print(out, pfd);
    Genode::print(out, (unsigned) state);
    Genode::print(out, (const char *) boundAddress);
    Genode::print(out, (const char *) remoteAddress);
}
