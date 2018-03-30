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
        int api_rev = ADQAPI_GetRevision();
        if (!IS_VALID_DLL_REVISION(api_rev))
        {
            throw nds::NdsError("ERROR: The linked header file and the loaded DLL are of different revisions. This may cause corrupt behavior.");
        }

        // Check how many ADQ devices are connected to the system; returns a number of devices
        int nof_adq = ADQControlUnit_FindDevices(m_adq_cu);
        
        if (nof_adq > 0)
        {
            m_adq_dev = ADQControlUnit_GetADQ(m_adq_cu, m_adq_nr); // get pointer to interface of certain device
            // Check if device started normally
            unsigned int adq_ok = m_adq_dev->IsStartedOK();
            if (adq_ok == 1)
            {
                // Get device info
                char* adq_bpn = m_adq_dev->GetBoardProductName();
                char* adq_bsn = m_adq_dev->GetBoardSerialNumber();
                unsigned int adq_pid = m_adq_dev->GetProductID();
                const char* adq_opt = m_adq_dev->GetCardOption();

                //Get a pointer to channel group of device
                //std::shared_ptr<ADQAIChannelGroup> aichgrp = std::make_shared<ADQAIChannelGroup>("AI", m_node, m_adq_cu, m_adq_nr);
                //m_AIChannelGroup.push_back(aichgrp);

                // initialize certain device
                m_node.initialize(this, factory);

                ndsDebugStream(m_node) << "ADQ device started:\nProduct name: " << adq_bpn << "\nSerial number: " << adq_bsn << \
                    "\nProduct ID: " << adq_pid << "Card option: " << adq_opt << std::endl;
            }
            else
            {
                throw nds::NdsError("Device failure during configuration");
            }           
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
    DeleteADQControlUnit(m_adq_cu);
}

NDS_DEFINE_DRIVER(adq, ADQDevice);