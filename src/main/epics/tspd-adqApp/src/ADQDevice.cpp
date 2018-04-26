#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <string.h> 

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQDevice::ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters) :
    m_node(deviceName)
{
    adq_cu = CreateADQControlUnit(); // Creates an ADQControlUnit called adq_cu
    if (adq_cu != NULL)
    {
        // Check DLL revisions 
        int api_rev = ADQAPI_GetRevision();
        std::cout << "DEBUG: " << "API Revision: " << api_rev  << std::endl;
        if (!IS_VALID_DLL_REVISION(api_rev))
        {
            throw nds::NdsError("ERROR: The linked header file and the loaded DLL are of different revisions. This may cause corrupt behavior.");
        }

        // Find all connected devices
        success = ADQControlUnit_ListDevices(adq_cu, &adq_info_list, &adq_list);
        std::cout << "DEBUG: " << "Listed ADQs: " << adq_list  << std::endl;
        
        // Before continuing it is needed to ask for a specified ADQ serial number of the device to connect to it!
        std::cout << "Enter device Serial Number (e.g. SPD-06215):" << std::endl;
        std::cin >> input;
        specified_sn = input;
        if (success)
        {
            if (adq_list > 0)
            {
                // This block searches a device with a specified serial number
                for (adq_list_nr = 0; adq_list_nr < adq_list; ++adq_list_nr)
                {
                    // Opens communication channel to a certain ADQ device; 
                    // this ADQ will show up in lists of functions NofADQ and GetADQ
                    success = ADQControlUnit_OpenDeviceInterface(adq_cu, adq_list_nr);
                    if (success)                                             
                    {
                        n_of_adq = ADQControlUnit_NofADQ(adq_cu);
                        std::cout << "DEBUG: " << "Readback NofADQ: " << n_of_adq << std::endl;
                        if (n_of_adq == 1)
                        {
                            // Make this device ready to use
                            success = ADQControlUnit_SetupDevice(adq_cu, adq_list_nr);
                            if (success)
                            {
                                adq_nr = 1;
                                // Get pointer to interface of certain device                                      
                                m_adq_dev = ADQControlUnit_GetADQ(adq_cu, adq_nr);
                                // Check if this ADQ serial number is the one specified                            
                                adq_sn = m_adq_dev->GetBoardSerialNumber();
                                std::cout << "DEBUG: " << "Readback board serial num ADQ: " << adq_sn << std::endl;
                                if (!strcmp(specified_sn, adq_sn))
                                {
                                    adq_found = 1;
                                    std::cout << "DEBUG: " << "ADQ is found: " << adq_found << std::endl;
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
                                DeleteADQControlUnit(adq_cu);
                                throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                            } // ADQControlUnit_SetupDevice
                        }
                        else
                        {
                            throw nds::NdsError("ERROR: More than one device is added to CU lists.");
                            DeleteADQControlUnit(adq_cu);
                            throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                        }
                    }
                    else
                    {
                        throw nds::NdsError("ERROR: Device failure during interface opening.");
                        DeleteADQControlUnit(adq_cu);
                        throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                    } // ADQControlUnit_OpenDeviceInterface

                } 
                if (adq_found)
                {
                    // Check if ADQ started normally
                    success = m_adq_dev->IsStartedOK();
                    if (success)
                    {
                        // Get a pointer to channel group of device
                        std::shared_ptr<ADQAIChannelGroup> ai_chgrp = std::make_shared<ADQAIChannelGroup>("AI", m_node, m_adq_dev);
                        m_AIChannelGroup.push_back(ai_chgrp);

                        // Get a pointer to common part of device
                        std::shared_ptr<ADQInfo> info_adq = std::make_shared<ADQInfo>("INFO", m_node, m_adq_dev);
                        m_Info.push_back(info_adq);

                        m_node.setLogLevel(nds::logLevel_t::info);

                        // Initialize certain device after declaration of all its PVs
                        m_node.initialize(this, factory);

                        //Test streams
                            ndsInfoStream(m_node) << "test INFO" << std::endl;
                            ndsWarningStream(m_node) << "test WARNING" << std::endl;
                            ndsErrorStream(m_node) << "test ERROR" << std::endl;

                    }
                    else
                    {
                        throw nds::NdsError("ERROR: Device didn't start normally.");
                        DeleteADQControlUnit(adq_cu);
                        throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                    } // IsStartedOK
                }
                else
                {
                    throw nds::NdsError("ERROR: Device with specified serial number was not found.");
                    DeleteADQControlUnit(adq_cu);
                    throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                } // end of the serial number block
            }
            else
            {
                throw nds::NdsError("ERROR: No ADQ devices found.");
                DeleteADQControlUnit(adq_cu);
                throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
            } // adq_list

        }
        else
        {
            throw nds::NdsError("ERROR: Listing all connected devices failed.");
            DeleteADQControlUnit(adq_cu);
            throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
        } // ADQControlUnit_ListDevices
    }
    else
    {
        throw nds::NdsError("ERROR: Failed to create ADQ Control Unit.");
        DeleteADQControlUnit(adq_cu);
        throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
    } // CreateADQControlUnit
}

ADQDevice::~ADQDevice()
{
    DeleteADQControlUnit(adq_cu);
    ndsInfoStream(m_node) << "ADQ Control Unit is deleted." << std::endl;
}

// The following MACRO defines the function to be exported in order
//  to allow the dynamic loading of the shared module
NDS_DEFINE_DRIVER(adq, ADQDevice)