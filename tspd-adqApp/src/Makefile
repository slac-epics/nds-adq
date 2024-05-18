TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#========================================

USR_CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -pthread -DLINUX

# Set a flag if we're using an EVR, so that the driver gets EVR timestamps
ifdef EVRCLIENT
USR_CXXFLAGS += -DEVR_SUPPORT
endif
ifdef EVENT
USR_CXXFLAGS += -DEVR_SUPPORT
endif

USR_INCLUDES += -I$(ADQ_INCLUDE)
USR_INCLUDES += -I$(NDS3_INCLUDE)

# ======================================================
# PATH TO "NON EPICS" EXTERNAL PACKAGES: USER LIBRARIES
# ======================================================
adq_DIR = $(ADQ_LIB)
nds3_DIR = $(NDS3_LIB)

# ======================================================
# LINK "NON EPICS" EXTERNAL PACKAGE LIBRARIES STATICALLY
#=======================================================
USR_LIBS_Linux += adq
USR_LIBS_Linux += nds3

#========================================
# Build an IOC support library
#========================================
PROD_NAME = tspd-adq
LIBRARY_IOC_Linux += $(PROD_NAME)

# Install dbd files
DBD += $(PROD_NAME).dbd

$(PROD_NAME)_DBD += base.dbd

ifeq ($(T_A),linuxRT-x86_64)
$(PROD_NAME)_DBD += evrSupport.dbd
$(PROD_NAME)_DBD += devMrfEr.dbd
endif

# The following are compiled and added to the Support library
$(PROD_NAME)_SRCS += ADQAIChannel.cpp  ADQAIChannelGroup.cpp  ADQInfo.cpp ADQDevice.cpp

ifeq ($(T_A),linuxRT-x86_64)
$(PROD_NAME)_LIBS += evrSupport
endif

$(PROD_NAME)_LIBS += $(EPICS_BASE_IOC_LIBS)

#=============================
include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# End of file

