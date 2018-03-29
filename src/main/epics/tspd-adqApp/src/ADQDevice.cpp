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
    m_adq_cu = CreateADQControlUnit(); // Creates an ADQControlUnit called adq_cu_ptr 
    
    if (m_adq_cu != NULL)
    {
        // Check DLL revisions 
        int apirev = ADQAPI_GetRevision();
        if (!IS_VALID_DLL_REVISION(apirev))
        {
            throw nds::NdsError("ERROR: The linked header file and the loaded DLL are of different revisions. This may cause corrupt behavior.");
        }

        // Check how many ADQ devices are connected to the system; returns a number of devices
        int nof_adq = ADQControlUnit_FindDevices(adq_cu); 
        
        if (nof_adq > 0)
        {
            m_adq_ptr = ADQControlUnit_GetADQ(m_adq_cu, m_adq_nr); // get pointer to interface of certain device
          //std::shared_ptr<ADQAIChannelGroup> aichgrp = std::make_shared<ADQAIChannelGroup>("AI", m_node, m_adq_cu, m_adq_nr);
          //m_AIChannelGroup.push_back(aichgrp);

            // initialize certain device
            m_node.initialize(this, factory);
             
        }
        else
        {
            throw nds::NdsError("No ADQ devices found."); // return a error
        }
      
    }
    else
    {
        throw nds::NdsError("Failed to create ADQ Control Unit.");
    }

}

ADQDevice::~ADQDevice()
{
    ADQControlUnit_DeleteADQ(m_adq_cu, m_adq_nr);
}

NDS_DEFINE_DRIVER(adq, ADQDevice);