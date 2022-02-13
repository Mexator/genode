TARGET = ptcp_test
SRC_CC = main.cc
LIBS += base
LIBS += libc
LIBS += lwip_dl

LWIP_PORT_DIR := $(call select_from_ports,lwip)
LWIP_LIBPORT_DIR := $(call select_from_repositories,include/lwip/) # should be libports

INC_DIR += $(LWIP_PORT_DIR)/include/lwip
INC_DIR += $(LWIP_LIBPORT_DIR)

CC_CXX_WARN_STRICT =
