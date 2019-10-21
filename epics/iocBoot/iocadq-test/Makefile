TOP = ../..
include $(TOP)/configure/CONFIG

ifeq (${EPICS_REVISION},14)
ARCH = ${EPICS_HOST_ARCH}
endif

TARGETS = envPaths
include $(TOP)/configure/RULES.ioc
