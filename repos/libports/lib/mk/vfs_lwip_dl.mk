SRC_CC = vfs.cc

VFS_DIR  = $(REP_DIR)/src/lib/vfs/lwip
INC_DIR += $(VFS_DIR)
LD_OPT  += --version-script=$(VFS_DIR)/symbol.map

LWIP_PORT_DIR := $(call select_from_ports,lwip)

INC_DIR += $(REP_DIR)/include/lwip \
           $(LWIP_PORT_DIR)/include/lwip \

LIBS += lwip_dl

vpath %.cc $(VFS_DIR)

SHARED_LIB = yes

CC_CXX_WARN_STRICT_CONVERSION =