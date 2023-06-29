TOP=..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

INC += ADQAPI.h

ifeq (rhel7-x86_64, $(findstring rhel7-x86_64, $(T_A)))
LIB_INSTALLS_Linux += ../os/rhel7-x86_64/libadq.so.8.0
LIB_INSTALLS_Linux += ../os/rhel7-x86_64/libadq.so
LIB_INSTALLS_Linux += ../os/rhel7-x86_64/libadq.a
endif


ifeq (linuxRT-x86_64, $(findstring linuxRT-x86_64, $(T_A)))
LIB_INSTALLS_Linux += ../os/linuxRT-x86_64/libadq.so.8.0
LIB_INSTALLS_Linux += ../os/linuxRT-x86_64/libadq.so
LIB_INSTALLS_Linux += ../os/linuxRT-x86_64/libadq.a
#install:
#	ln -srf $(TOP)/lib/$(T_A)/libadq.so $(TOP)/lib/$(T_A)/libadq.so.0
endif

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE
