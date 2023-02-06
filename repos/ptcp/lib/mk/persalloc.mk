LIBS += base

SRC_CC += pers_heap.cc

LIB_DIR = $(REP_DIR)/src/lib/persalloc
vpath %.cc $(LIB_DIR)
SHARED_LIB = yes
CC_CXX_WARN_STRICT =
