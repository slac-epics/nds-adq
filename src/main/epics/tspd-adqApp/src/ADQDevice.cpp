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
        //// urojec L3: add const in front of any variable that is not expected to change
        ////            this way you cannt even change it by mistake
        // Check DLL revisions
        //// urojec L3: (DLL? :) we are on Linux ;P ), use something generic like library version
        //// also, be more careful with phrases such as linked loaded etc. they have a specific meaning and cannot be used interchangably,
        //// for example the header file is included not linked, library is linked (the -l remeber)
        ////
        int api_rev = ADQAPI_GetRevision();
        std::cout << "DEBUG: " << "API Revision: " << api_rev  << std::endl;
        if (!IS_VALID_DLL_REVISION(api_rev))
        {
            throw nds::NdsError("ERROR: The linked header file and the loaded DLL are of different revisions. This may cause corrupt behavior.");
        }

        // Find all connected devices
        success = ADQControlUnit_ListDevices(adq_cu, &adq_info_list, &adq_list);
        std::cout << "DEBUG: " << "Listed ADQs: " << adq_list  << std::endl;


        //// urojec L2: This is a very handy feature, but we need to provide an option for
        //// the serial number to be specified at startup - this is due to autostarting iocs etc.
        //// Let's discuss, could be as simple as leaving the parameter holding the serial number as QUERY or something
        ////
        // Before continuing it is needed to ask for a specified ADQ serial number of the device to connect to it!
        std::cout << "Enter device Serial Number (e.g. 06215):" << std::endl;
        std::cin >> input_raw;
        std::string prefix = "SPD-";
        prefix.insert(4, input_raw);
        input = prefix.c_str();
        specified_sn = (char*)input;


        //// urojec L3: In such long cases where success means a bunch of code to execute I
        ////            find it more readable if the nonsucessfull case is handled first (the !success),
        ////            since that one only has a few lines and you iommediately see what happens, instead
        ////            of having to scroll like crazy (jsut a preference though)
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
                                //// urojec L1: if you throw here the Delete function will not get executed!!
                                //// throw that is not caught breaks the flow of the code
                                throw nds::NdsError("ERROR: Device failure during setup.");
                                DeleteADQControlUnit(adq_cu);
                                throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                            } // ADQControlUnit_SetupDevice
                        }
                        else
                        {
                            //// urojec L1: same as above
                            throw nds::NdsError("ERROR: More than one device is added to CU lists.");
                            DeleteADQControlUnit(adq_cu);
                            throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                        }
                    }
                    else
                    {
                        //// urojec L1: same as above
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
                        //// urojec L1: same as above
                        throw nds::NdsError("ERROR: Device didn't start normally.");
                        DeleteADQControlUnit(adq_cu);
                        throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                    } // IsStartedOK
                }
                else
                {
                    //// urojec L1: same as above
                    throw nds::NdsError("ERROR: Device with specified serial number was not found.");
                    DeleteADQControlUnit(adq_cu);
                    throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
                } // end of the serial number block
            }
            else
            {
                //// urojec L1: samne as above
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
        //// urojec L1: same as above
        ////    maybe you should consider just throwing the error in all cases and wrapping them in one try/catch
        ////    statement. This way you will only need to have one place where you call Delete unit, the whole
        ////    thing would not have to be repeated so many times. You can still keep the message from the error
        ////    and attach it to the throw where you inform that the unit was deleted
        throw nds::NdsError("ERROR: Failed to create ADQ Control Unit.");
        DeleteADQControlUnit(adq_cu);
        throw nds::NdsError("DELETED: ADQ Control Unit was deleted due to error.");
    } // CreateADQControlUnit
}

ADQDevice::~ADQDevice()
{
    //// urojec L3: (check) everything is ok with this, such as are there any buffers that that
    //// thing holds pointers to that need to be freed etc. 
    DeleteADQControlUnit(adq_cu);
    ndsInfoStream(m_node) << "ADQ Control Unit is deleted." << std::endl;
}

// The following MACRO defines the function to be exported in order
//  to allow the dynamic loading of the shared module
NDS_DEFINE_DRIVER(adq, ADQDevice)
