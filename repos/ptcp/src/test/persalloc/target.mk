TARGET = persalloc

SRC_CC += main.cc
SRC_CC += pers_heap.cc

LIBS += base
LIBS += libc
LIBS += stdcxx

CC_CXX_WARN_STRICT =
