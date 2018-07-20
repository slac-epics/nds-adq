#!../../bin/linux-x86_64/adq-test

#- You may have to change adq-test to something else
#- everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/adq-test.dbd"
adq_test_registerRecordDeviceDriver pdbbase

drvAsynIPPortConfigure("ADQ","06215")

## Load record instances
dbLoadTemplate("db/ADQDevice.substitutions", "DEV = 06215")

## Load library of ADQ module
ndsLoadDriver("${ADQ_LIB}/libtspd-adq.so")
ndsLoadNamingRules ("${ADQ_SRC}/ADQDeviceNamingRules.ini")
ndsCreateDevice(adq, "ADQ")

## Enable info logging in NDS
nds setLogLevelInfo ADQ

cd "${TOP}/iocBoot/${IOC}"
iocInit
