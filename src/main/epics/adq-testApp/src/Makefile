TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = adq-test
# adq-test.dbd will be created and installed
DBD += adq-test.dbd

# adq-test.dbd will be made up from these files:
adq-test_DBD += base.dbd

# Include dbd files from all support applications:
adq-test_DBD += adq-testInclude.dbd

# Add all the support libraries needed by this IOC
PROD_LIBS += nds3 nds3epics asyn

# NDS3 can be found here
nds3_DIR = $(NDS3)

# Where to find ADQ driver and enforce linking
adq_DIR += $(ADQ)
adq-test_LDFLAGS += -Wl,--no-as-needed -ladq -lnds3 

# adq-test_registerRecordDeviceDriver.cpp derives from adq-test.dbd
adq-test_SRCS += adq-test_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
adq-test_SRCS_DEFAULT += adq-testMain.cpp
adq-test_SRCS_vxWorks += -nil-


# Finally link to the EPICS Base libraries
adq-test_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

