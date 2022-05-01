#include <ptcp_client/serialized/serialized_socket_state.h>

void serialized_socket::save(std::ostream &out) {
    out << pfd << " ";
    out << (unsigned) state << " ";
    out << boundAddress << " ";
    out << "\n";
}

serialized_socket serialized_socket::load(std::istream &in) {
    serialized_socket ret;
    in >> ret.pfd;
    in >> (unsigned &) ret.state;
    ret.boundAddress = new char[1024];
    in >> ret.boundAddress;
    return ret;
}

void serialized_socket::print(Genode::Output &out) const {
    Genode::print(out, pfd);
    Genode::print(out, (unsigned) state);
    Genode::print(out, (const char *) boundAddress);
}
