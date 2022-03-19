# Dynamically linked version of lwip
include $(REP_DIR)/lib/mk/lwip.mk

SRC_CC += sio.cc
vpath %.cc $(REP_DIR)/src/lib/lwip_dl

SHARED_LIB=yes
