#include <ptcp_client/serialized/serialized_socket_state.h>

void serialized_socket::save(std::ostream &out) {
    out << pfd << " ";
    out << (unsigned) state << " ";
    out << bound << " ";
    out << (const char *) boundAddress << " ";
    out << "\n";
}

serialized_socket& serialized_socket::load(std::istream &in) {
    serialized_socket& ret = *new serialized_socket;
    in >> ret.pfd;
    in >> (unsigned &) ret.state;
    in >> ret.bound;
    if (ret.bound) {
        ret.boundAddress = new char[1024];
        in.seekg(1, std::ios_base::cur);
        in.getline(ret.boundAddress, 1024);
    }
    return ret;
}

void serialized_socket::print(Genode::Output &out) const {
    Genode::print(out, pfd);
    Genode::print(out, (unsigned) state);
    Genode::print(out, bound);
    Genode::print(out, (const char *) boundAddress);
}
