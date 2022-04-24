#include <ptcp_client/serialized/serialized_socket_state.h>

void serialized_socket::save(std::ostream &out) {
    out << pfd << " ";
    out << "\n";
}

serialized_socket serialized_socket::load(std::istream &in) {
    serialized_socket ret;
    in >> ret.pfd;
    return ret;
}