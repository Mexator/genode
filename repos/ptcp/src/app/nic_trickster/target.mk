TARGET = nic_trickster

LIBS += base net

SRC_CC += component.cc
SRC_CC += main.cc
SRC_CC += packet_log.cc
SRC_CC += uplink.cc
SRC_CC += interface.cc
SRC_CC += control/stopper.cc
SRC_CC += control/tracker_delegate.cc
SRC_CC += threading/submit_thread.cc

INC_DIR += $(PRG_DIR)

CONFIG_XSD = config.xsd

CC_CXX_WARN_STRICT_CONVERSION =
CC_CXX_WARN_STRICT = -Weffc++
