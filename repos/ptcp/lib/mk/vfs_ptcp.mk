LIBS += vfs_lwip_dl
LIBS += vfs_collection
LIBS += ptcp_client
LIBS += libc
LIBS += stdcxx

LWIP_PORT_DIR := $(call select_from_ports,lwip)
LWIP_LIBPORT_DIR := $(call select_from_repositories,include/lwip/) # should be libports
INC_DIR += $(LWIP_PORT_DIR)/include/lwip
INC_DIR += $(LWIP_LIBPORT_DIR)

LIBC_SRC_DIR := $(call select_from_repositories,src/lib/libc/) # should be libports
INC_DIR += $(LIBC_SRC_DIR)

SRC_CC += file_system_factory.cc
SRC_CC += load.cc
SRC_CC += load_state/load_libc.cc
SRC_CC += load_state/load_lwip.cc
SRC_CC += save.cc
SRC_CC += save_state/save_libc.cc
SRC_CC += save_state/save_lwip.cc
SRC_CC += sig_handlers.cc
SRC_CC += snapshot.cc

PLUGIN_DIR = $(REP_DIR)/src/lib/vfs_ptcp
vpath %.cc $(PLUGIN_DIR)
CC_CXX_WARN_STRICT =
SHARED_LIB = yes