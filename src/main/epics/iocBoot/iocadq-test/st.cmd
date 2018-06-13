#!../../bin/linux-x86_64/adq-test

#- You may have to change adq-test to something else
#- everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/adq-test.dbd"
adq_test_registerRecordDeviceDriver pdbbase

drvAsynIPPortConfigure("ADQ","localhost:8007")

## Load record instances
##dbLoadTemplate("db/ADQDevice.substitutions")

# Load library of ADQ module
ndsLoadDriver("${ADQ_LIB}/libtspd-adq.so")
ndsLoadNamingRules ("${ADQ_SRC}/ADQDeviceNamingRules.ini")
ndsCreateDevice(adq, "ADQ")

cd "${TOP}/iocBoot/${IOC}"
iocInit
