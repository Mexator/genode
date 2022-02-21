LIBS += vfs_lwip_dl
SRC_CC += sig_handlers.cc
SRC_CC += file_system_factory.cc

PLUGIN_DIR = $(REP_DIR)/src/lib/vfs_ptcp
vpath %.cc $(PLUGIN_DIR)
CC_CXX_WARN_STRICT =
SHARED_LIB = yes