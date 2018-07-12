#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string.h>
#include <string>
#include <unistd.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"
#include "ADQInfo.h"

ADQDevice::ADQDevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& parameters) :
    m_node(deviceName)
{
    unsigned int status;

    try
    {
        m_adqCtrlUnit = CreateADQControlUnit();   // Creates an ADQControlUnit called adq_cu
        if (!m_adqCtrlUnit)
        {
            throw nds::NdsError("Failed to create ADQ Control Unit (CreateADQControlUnit).");
        }

        // Enable error logging for devices (saves files to the TOP directory '/m-epics-tspd-adq/')
        ADQControlUnit_EnableErrorTrace(m_adqCtrlUnit, LOG_LEVEL_INFO, ".");

        // Check revisions
        const int adqApiRev = ADQAPI_GetRevision();
        std::cout << "DEBUG: "
                  << "API Revision: " << adqApiRev << std::endl;
        if (!IS_VALID_DLL_REVISION(adqApiRev))
        {
            std::cout << "WARNING: The included header file and the linked library are of different revisions. This "
                         "may cause corrupt behavior."
                      << std::endl;
        }

        // Find all connected devices
        struct ADQInfoListEntry* adqInfoStruct;
        unsigned int adqDevList;
        status = ADQControlUnit_ListDevices(m_adqCtrlUnit, &adqInfoStruct, &adqDevList);
        if (!status)
        {
            throw nds::NdsError("Listing connected devices failed (ADQControlUnit_ListDevices).");
        }

        if (adqDevList <= 0)
        {
            throw nds::NdsError("No ADQ devices found.");
        }

        std::cout << "DEBUG: "
                  << "Number of ADQs: " << adqDevList << std::endl;

        // Before continuing it is needed to ask for a specified ADQ serial number of the device to connect to it
        char adqSnReqRaw[6];
        std::ostringstream adqSnTmp;

        std::cout << "Enter device Serial Number (e.g. 06215, 06302):" << std::endl;
        std::cin >> adqSnReqRaw;
        adqSnTmp << "SPD-" << adqSnReqRaw;
        std::string adqSnReq(adqSnTmp.str());

        /* This block searches a device with a requested serial number
        */
        bool adqReqFound;
        for (unsigned int adqDevListNum = 0; adqDevListNum < adqDevList; ++adqDevListNum)
        {
            // Opens communication channel to a certain ADQ device
            status = ADQControlUnit_OpenDeviceInterface(m_adqCtrlUnit, adqDevListNum);
            if (!status)
            {
                throw nds::NdsError("Device failure during interface opening (ADQControlUnit_OpenDeviceInterface).");
            }

            // Make this device ready to use
            status = ADQControlUnit_SetupDevice(m_adqCtrlUnit, adqDevListNum);
            if (!status)
            {
                throw nds::NdsError("Device failure during setup (ADQControlUnit_SetupDevice).");
            }

            // Get pointer to interface of the device
            m_adqInterface = ADQControlUnit_GetADQ(m_adqCtrlUnit, adqDevListNum + 1);

            // Check if this ADQ serial number is the one requested
            const char* adqSnRdbk = m_adqInterface->GetBoardSerialNumber();
            if (adqSnReq == adqSnRdbk)
            {
                adqReqFound = true;
                std::cout << "Requested ADQ device is found; Serial Number: " << adqSnRdbk << std::endl;
                break;
            }
        }   // Requested serial number

        if (!adqReqFound)
        {
            throw nds::NdsError("Requested ADQ device was not found.");
        }

        // Check if ADQ started normally
        status = m_adqInterface->IsStartedOK();
        if (!status)
        {
            throw nds::NdsError("Device didn't start normally (IsStartedOK).");
        }

        std::shared_ptr<ADQInfo> adqInfo = std::make_shared<ADQInfo>(adqSnReqRaw, m_node, m_adqInterface);
        m_adqInfoPtr.push_back(adqInfo);

        std::shared_ptr<ADQAIChannelGroup> adqChanGrp = std::make_shared<ADQAIChannelGroup>(adqSnReqRaw, m_node, m_adqInterface);
        m_adqChanGrpPtr.push_back(adqChanGrp);
        
        // Initialize requested device after declaration of all its PVs
        m_node.initialize(this, factory);
    }
    catch (const nds::NdsError& e)
    {
        if (m_adqCtrlUnit)
        {
            DeleteADQControlUnit(m_adqCtrlUnit);
            std::cout << "ADQ Control Unit was deleted." << std::endl;
        }
        std::cout << "Initializing stopped due to error: " << e.what() << std::endl;
    }
}

ADQDevice::~ADQDevice()
{
    if (m_adqCtrlUnit)
    {
        DeleteADQControlUnit(m_adqCtrlUnit);
        ndsInfoStream(m_node) << "ADQ Control Unit was deleted." << std::endl;
    }
    ndsInfoStream(m_node) << "ADQ Device class was destructed." << std::endl;
}

// The following MACRO defines the function to be exported in order
// to allow the dynamic loading of the shared module
NDS_DEFINE_DRIVER(adq, ADQDevice)
