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

 /*                       // Set PVs for device info
                        m_productNamePV.setScanType(nds::scanType_t::interrupt);
                        m_productNamePV.setMaxElements(32);
                        m_node.addChild(m_productNamePV);

                        m_serialNumberPV.setScanType(nds::scanType_t::interrupt);
                        m_serialNumberPV.setMaxElements(32);
                        m_node.addChild(m_serialNumberPV);

                        m_productIDPV.setScanType(nds::scanType_t::interrupt);
                        m_node.addChild(m_productIDPV);

                        m_adqTypePV.setScanType(nds::scanType_t::interrupt);
                        m_node.addChild(m_adqTypePV);

                        m_cardOptionPV.setScanType(nds::scanType_t::interrupt);
                        m_cardOptionPV.setMaxElements(32);
                        m_node.addChild(m_cardOptionPV);
*/
                        // Initialize certain device after declaration of all its PVs
                        m_node.initialize(this, factory);

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
/*
void ADQDevice::setDeviceInfo()
{
    std::int32_t tmp_int;
    std::string tmp_str;
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);

    m_productNamePV.processAtInit(1);
    m_productNamePV.read(&now, &tmp_str);
    m_productNamePV.push(now, tmp_str);

    m_serialNumberPV.processAtInit(1);
    m_serialNumberPV.read(&now, &tmp_str);
    m_serialNumberPV.push(now, tmp_str);

    m_productIDPV.processAtInit(1);
    m_productIDPV.read(&now, &tmp_int);
    m_productIDPV.push(now, tmp_int);

    m_adqTypePV.processAtInit(1);
    m_adqTypePV.read(&now, &tmp_int);
    m_adqTypePV.push(now, tmp_int);

    m_cardOptionPV.processAtInit(1);
    m_cardOptionPV.read(&now, &tmp_str);
    m_cardOptionPV.push(now, tmp_str);

    std::cout << "Device info is received." << std::endl;

}

void ADQDevice::getProductName(timespec* pTimestamp, std::string* pValue)
{
    char* adq_pn = m_adq_dev->GetBoardProductName();
    *pValue = std::string(adq_pn);
}

void ADQDevice::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    char* adq_sn = m_adq_dev->GetBoardSerialNumber();
    *pValue = std::string(adq_sn);
}

void ADQDevice::getProductID(timespec* pTimestamp, std::int32_t* pValue)
{
    unsigned int adq_pid = m_adq_dev->GetProductID();
    *pValue = std::int32_t(adq_pid);
}

void ADQDevice::getADQType(timespec* pTimestamp, std::int32_t* pValue)
{
    int adq_type = m_adq_dev->GetADQType();
    *pValue = std::int32_t(adq_type);
}

void ADQDevice::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    const char* adq_opt = m_adq_dev->GetCardOption();
    *pValue = std::string(adq_opt);
}
*/

ADQDevice::~ADQDevice()
{
    DeleteADQControlUnit(adq_cu);
    ndsInfoStream(m_node) << "ADQ Control Unit is deleted." << std::endl;
}

// The following MACRO defines the function to be exported in order
//  to allow the dynamic loading of the shared module
NDS_DEFINE_DRIVER(adq, ADQDevice)