#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <string.h> 

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"

ADQDevice::ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters) :
    m_node(deviceName)
    //m_productName(nds::PVVariableIn<std::string>("ProductName"))
{
    adq_cu = CreateADQControlUnit(); // Creates an ADQControlUnit called adq_cu 
    
    if (adq_cu != NULL)
    {
        // Check DLL revisions 
        int api_rev = ADQAPI_GetRevision();
        if (!IS_VALID_DLL_REVISION(api_rev))
        {
            throw nds::NdsError("ERROR: The linked header file and the loaded DLL are of different revisions. This may cause corrupt behavior.");
        }

        // Find all connected devices
        success = ADQControlUnit_ListDevices(adq_cu, &adq_info_list, &adq_list);

        // Before continuing it is needed to ask for a specified ADQ serial number of the device to connect to it!
        
        if (success)
        {
            if (adq_list > 0)
            {
                // This block searches a device with a specified serial number
                for (adq_list_nr = 0; adq_list_nr < adq_list; adq_list_nr++)
                {
                    // Opens communication channel to a certain ADQ device; 
                    // this ADQ will show up when using functions NofADQ and GetADQ
                    success = ADQControlUnit_OpenDeviceInterface(adq_cu, adq_list_nr);

                    if (success)
                    {
                        // Make this device ready to use
                        success = ADQControlUnit_SetupDevice(adq_cu, adq_list_nr); // NEEDED TO BE CHECKED if this can be put AFTER serial number check!
                                                                                   // because function GetBoardSerialNumber is used
                                                                                                        //    |
                        if (success)                                                                    //    |
                        {                                                                               //    |                                                         
                            adq_nr = 1;                                                                 //    |
                            // Get pointer to interface of certain device                                     |
                            m_adq_dev = ADQControlUnit_GetADQ(adq_cu, adq_nr);                          //    |
                            // Check if this ADQ serial number is the one specified                           |
                            adq_sn = m_adq_dev->GetBoardSerialNumber();          //  <-------------------------
                            if (!strcmp(adq_sn, specified_sn))
                            {
                                break;
                            }
                            else
                            {
                                ADQControlUnit_DeleteADQ(adq_cu, adq_nr);
                            }
                        }
                        else
                        {
                            throw nds::NdsError("ERROR: Device failure during setup.");
                            goto error;
                        } // ADQControlUnit_SetupDevice
                    }
                    else
                    {
                        throw nds::NdsError("ERROR: Device failure during interface opening.");
                        goto error;
                    } // ADQControlUnit_OpenDeviceInterface

                } // end of the serial number block
                
                // Check if ADQ started normally
                unsigned int adq_ok = m_adq_dev->IsStartedOK(); // need a msg about OK start!
                if (adq_ok)
                {
                    // Get device info
                    char* adq_pn = m_adq_dev->GetBoardProductName();
                    unsigned int adq_pid = m_adq_dev->GetProductID();
                    int adq_type = m_adq_dev->GetADQType();
                    const char* adq_opt = m_adq_dev->GetCardOption();
               
                    // Get a pointer to channel group of device
                    std::shared_ptr<ADQAIChannelGroup> ai_chgrp = std::make_shared<ADQAIChannelGroup>("AI", m_node, m_adq_dev);
                    m_AIChannelGroup.push_back(ai_chgrp);

                    // Initialize certain device after declaration of all its PVs
                    m_node.initialize(this, factory);

                    // Device information is returned
                    ndsInfoStream(m_node) << "Device started:\nADQ" << adq_type << adq_opt << "\nProduct name: " << adq_pn << \
                        "\nSerial number: " << adq_sn << "\nProduct ID: " << adq_pid << std::endl;
                }
                else
                {
                    throw nds::NdsError("ERROR: Device didn't start normally.");
                    goto error;
                } // IsStartedOK
            }
            else
            {
                throw nds::NdsError("ERROR: No ADQ devices found.");
                goto error;
            } // adq_list

        }
        else
        {
            throw nds::NdsError("ERROR: Listing all connected devices failed.");
            goto error;
        } // ADQControlUnit_ListDevices
    }
    else
    {
        throw nds::NdsError("ERROR: Failed to create ADQ Control Unit.");
        goto error;
    } // CreateADQControlUnit

error:
    DeleteADQControlUnit(adq_cu);
    throw nds::NdsError("ERROR: ADQ Control Unit was deleted due to error.");
} 

ADQDevice::~ADQDevice()
{
    DeleteADQControlUnit(adq_cu);
    ndsInfoStream(m_node) << "ADQ Control Unit is deleted." << std::endl;
}

NDS_DEFINE_DRIVER(adq, ADQDevice);