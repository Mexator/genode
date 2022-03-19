# Collection of handy VFSs

LIBS += base
SRC_CC += buffer_fs.cc
SRC_CC += proxy_fs.cc

LIB_DIR = $(REP_DIR)/src/lib/vfs_collection
vpath %.cc $(LIB_DIR)