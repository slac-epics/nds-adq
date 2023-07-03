TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#========================================

USR_CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -DNDS3_DLL -pthread -DLINUX

USR_INCLUDES += -I$(NDS3)/include

#========================================
# Build an IOC support library

LIBRARY_IOC_Linux += tspd-adq

# Install dbd files
DBD += tspd-adq.dbd

tspd-adq_DBD += base.dbd

# The following are compiled and added to the Support library
tspd-adq_SRCS += ADQAIChannel.cpp  ADQAIChannelGroup.cpp  ADQInfo.cpp ADQDevice.cpp

tspd-adq_LIBS += adq
tspd-adq_LIBS += nds3

tspd-adq_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# End of file

