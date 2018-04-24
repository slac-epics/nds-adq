#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <ADQAPI.h>
#include "ADQInfo.h"
#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include <nds3/nds.h>

class ADQDevice 
{
public: 
    ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters);
    ~ADQDevice();
/*
    void setDeviceInfo();
    void getProductName(timespec* pTimestamp, std::string* pValue);
    void getSerialNumber(timespec* pTimestamp, std::string* pValue);
    void getProductID(timespec* pTimestamp, std::int32_t* pValue);
    void getADQType(timespec* pTimestamp, std::int32_t* pValue);
    void getCardOption(timespec* pTimestamp, std::string* pValue);
*/    
protected:


private:
    // pointer to certain ADQ device
    ADQInterface * m_adq_dev;

    // Success status
    unsigned int success;
    // Pointer to ADQ Control Unit
    void* adq_cu;
    // Pointer to ADQ info structure
    struct ADQInfoListEntry* adq_info_list;
    // Number of ADQ devices connected to the system from ListDevices function
    unsigned int adq_list;
    // Number of ADQ devices from NofADQ function
    int n_of_adq;
    // User input
    char input[100];
    // ADQ device number from adq_list array; indexing starts from 0
    // Please note that the device number when using GetADQ/NofADQ/etc will not have anything to do with the index number used in this function.
    int adq_list_nr;
    // ADQ device number used for getting a pointer m_adq_dev
    int adq_nr;
    // Readback ADQ serial number
    char* adq_sn;
    // Serial number of needed ADQ
    char* specified_sn;
    // Var for for loop
    unsigned int adq_found;

    // PVs connected to EPICS records
    nds::PVVariableIn<std::string> m_productNamePV;
    nds::PVVariableIn<std::string> m_serialNumberPV;
    nds::PVVariableIn<std::int32_t> m_productIDPV;
    nds::PVVariableIn<std::int32_t> m_adqTypePV;
    nds::PVVariableIn<std::string> m_cardOptionPV;

    // Pointer to channel group of device
    std::vector<std::shared_ptr<ADQAIChannelGroup> > m_AIChannelGroup;

    // Pointer to info part of device
    std::vector<std::shared_ptr<ADQInfo> > m_Info;

    nds::Node m_node;
};


#endif /* ADQDEVICE_H */