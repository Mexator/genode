SRC_CC += ptcp_file_system_factory.cc
PLUGIN_DIR = $(REP_DIR)/src/lib/vfs_ptcp
INC_DIR += $(PLUGIN_DIR)
vpath %.cc $(PLUGIN_DIR)
CC_CXX_WARN_STRICT =
SHARED_LIB = yes