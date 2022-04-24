SRC_CC += fd_proxy.cc
SRC_CC += ptcp_lock.cc
SRC_CC += supervisor_helper.cc
SRC_CC += socket_state.cc
SRC_CC += socket_supervisor.cc
SRC_CC += serialized/serialized_socket_state.cc

LIBS += libc
LIBS += stdcxx

PLUGIN_DIR = $(REP_DIR)/src/lib/ptcp_client
vpath %.cc $(PLUGIN_DIR)
CC_CXX_WARN_STRICT =
SHARED_LIB = yes