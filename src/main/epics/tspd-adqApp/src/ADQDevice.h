#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <ADQAPI.h>
#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include <nds3/nds.h>


class ADQDevice 
{
public: 
    ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters);
    ~ADQDevice();
    
protected:
    // pointer to certain ADQ device
    ADQInterface * m_adq_dev;

private:
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
    char input;
    // ADQ device number from adq_list array; indexing starts from 0
    // Please note that the device number when using GetADQ/NofADQ/etc will not have anything to do with the index number used in this function.
    int adq_list_nr;
    // ADQ device number used for getting a pointer m_adq_dev
    int adq_nr;
    // ADQ serial number
    char* adq_sn;
    // Serial number of needed ADQ
    char* specified_sn;
    // Var for for loop
    unsigned int adq_found;


    // Pointer to channel group of device
    std::vector<std::shared_ptr<ADQAIChannelGroup> > m_AIChannelGroup;

    nds::Node m_node;
};


#endif /* ADQDEVICE_H */