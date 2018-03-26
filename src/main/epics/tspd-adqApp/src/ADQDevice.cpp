#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"

ADQDevice::ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters) :
    m_node(deviceName)
{
    void* adq_cu_ptr = CreateADQControlUnit(); // Creates an ADQControlUnit called adq_cu_ptr 
    
    if (adq_cu_ptr != NULL)
    {
        // Check how many ADQ devices are connected to the system; returns a number of devices
        int nof_ADQ = ADQControlUnit_NofADQ(void* adq_cu_ptr); // the function returns int not unint!
        
        if (nof_ADQ > 0)
        {
            unsigned int retLen = nof_ADQ;
        }
        else
        {
            // return a error
        }
        
        // Lists devices connected to the system. The list is then returned as an array 
        // which can be indexed from retList[0] to retList[retLen - 1], with each entry corresponding to an ADQ device.
        unsigned int ADQControlUnit_ListDevices(void* adq_cu_ptr, struct ADQInfoListEntry** retList, &retLen);

        unsigned int ADQControlUnit_OpenDeviceInterface(void* adq_cu_ptr, int ADQInfoListEntryNumber);
        unsigned int ADQControlUnit_SetupDevice(void* adq_cu_ptr, int ADQInfoListEntryNumber);
    }
    


    m_node.initialize(this, factory)
}
ADQDevice::~ADQDevice()
{
    void ADQControlUnit_DeleteADQ(void * adq_cu_ptr, int ADQ_num);
}

NDS_DEFINE_DRIVER(adq, ADQDevice);