LIBS += vfs_lwip_dl

LWIP_PORT_DIR := $(call select_from_ports,lwip)
LWIP_LIBPORT_DIR := $(call select_from_repositories,include/lwip/) # should be libports
INC_DIR += $(LWIP_PORT_DIR)/include/lwip
INC_DIR += $(LWIP_LIBPORT_DIR)

SRC_CC += sig_handlers.cc
SRC_CC += file_system_factory.cc
SRC_CC += snapshot.cc

PLUGIN_DIR = $(REP_DIR)/src/lib/vfs_ptcp
vpath %.cc $(PLUGIN_DIR)
CC_CXX_WARN_STRICT =
SHARED_LIB = yes