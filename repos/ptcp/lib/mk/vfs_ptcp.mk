SRC_CC += file_system_factory.cc
SRC_CC += proxy_fs.cc

LIBS += ptcp_client
LIBS += libc

# Compile time only
LIBS += stdcxx

PLUGIN_DIR = $(REP_DIR)/src/lib/vfs_ptcp
vpath %.cc $(PLUGIN_DIR)
CC_CXX_WARN_STRICT =
SHARED_LIB = yes