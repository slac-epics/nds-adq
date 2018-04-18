#!../../bin/linux-x86_64/adq-test

#- You may have to change adq-test to something else
#- everywhere it appears in this file

< envPaths

cd "${TOP}"

## Register all support components
dbLoadDatabase "dbd/adq-test.dbd"
adq_test_registerRecordDeviceDriver pdbbase

drvAsynIPPortConfigure("adq_test","localhost:8007")


## Load record instances
dbLoadRecords("db/ADQDevice.template","PREFIX=adq_test,CH_GRP_ID=AI")

# Load library of ADQ module
ndsLoadDriver("lib/linux-x86_64/libtspd-adq.so")
ndsCreateDevice(adq, "adq_test")

cd "${TOP}/iocBoot/${IOC}"
iocInit

## Start any sequence programs
#seq sncxxx,"user=codac-dev"
