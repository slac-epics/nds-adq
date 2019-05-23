TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#=============================
# Build the IOC support library
# Name of the device library
LIBRARY_IOC += tspd-adq

# install tspd-adq.dbd into <top>/dbd
DBD += tspd-adq.dbd

# We enable the c++11 flag, plus we define the NDS3_DLL preprocessor macro.
USR_CXXFLAGS = -std=c++11 -Wall -Wextra -Werror -pedantic -DNDS3_DLL -pthread -DLINUX

# Dependency on NDS3 and ADQAPI
LIBS = -lnds3 -ladq

# tspd-adq_registerRecordDeviceDriver.cpp derives from tspd-adq.dbd
tspd-adq_SRCS += ADQAIChannel.cpp  ADQAIChannelGroup.cpp  ADQInit.cpp ADQDevice.cpp

#=============================
# Include makefile snippets to add support for additional modules

-include $(EPICS_ROOT)/mk/codac-common.mk

tspd-adq_DBD  += $(CODAC_DBD)
tspd-adq_LIBS += $(CODAC_LIBS)

#=============================
# Include SNL program makefile snippets

#-include ../*.snlprog

#=============================
# Finally link to the EPICS Base libraries

tspd-adq_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

