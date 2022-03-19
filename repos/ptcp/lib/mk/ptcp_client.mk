LIBS += base

SRC_CC += fd_proxy.cc

PLUGIN_DIR = $(REP_DIR)/src/lib/ptcp_client
vpath %.cc $(PLUGIN_DIR)
CC_CXX_WARN_STRICT =
SHARED_LIB = yes