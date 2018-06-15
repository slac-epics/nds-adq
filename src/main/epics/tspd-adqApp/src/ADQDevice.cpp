#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <string.h>
#include <unistd.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQDefinition.h"
#include "ADQInfo.h"
#include "ADQFourteen.h"
#include "ADQSeven.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQDevice::ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters) :
    m_node(deviceName)
{
    unsigned int success;
    int adqApiRev;
    // Readback ADQ serial number
    char* adqSnRdbk;
    // Requested ADQ serial number
    char* adqSnReq;
    // Pointer to ADQ info structure
    struct ADQInfoListEntry* adqInfoStruct;
    // Number of ADQ devices connected to the system from ListDevices function
    unsigned int adqDevList;
    // Number of ADQ devices from NofADQ function
    int adqTotalDevCnt;
    // ADQ device number from adqDevList array; indexing starts from 0
    // Please note that the device number when using GetADQ/NofADQ/etc will not have anything to do with the index number used in this function.
    unsigned int adqDevListNum;
    // ADQ device number used for getting a pointer m_adqDevPtr
    int adqDevNum;
    // Var for for-loop
    bool adqReqFound;
    // User input
    char adqSnReqRaw[6];
    const char* adqSnReqTmp;

    try
    {
        m_adqCtrlUnitPtr = CreateADQControlUnit(); // Creates an ADQControlUnit called adq_cu
        if (!m_adqCtrlUnitPtr)
        {
            throw nds::NdsError("ERROR: Failed to create ADQ Control Unit.");
        }
        else
        {
            //// urojec L3: add const in front of any variable that is not expected to change                                        !!!!!!!!!!!!!!!!
            ////            this way you cannt even change it by mistake                        
            // Check revisions
            adqApiRev = ADQAPI_GetRevision();
            std::cout << "DEBUG: " << "API Revision: " << adqApiRev << std::endl;
            if (!IS_VALID_DLL_REVISION(adqApiRev))
            {
                std::cout << "WARNING: The included header file and the linked library are of different revisions. This may cause corrupt behavior." << std::endl;
            }

            // Find all connected devices
            success = ADQControlUnit_ListDevices(m_adqCtrlUnitPtr, &adqInfoStruct, &adqDevList);
            std::cout << "DEBUG: " << "Listed ADQs: " << adqDevList << std::endl;


            //// urojec L2: This is a very handy feature, but we need to provide an option for                                       !!!!!!!!!!!!!!!!
            //// the serial number to be specified at startup - this is due to autostarting iocs etc.
            //// Let's discuss, could be as simple as leaving the parameter holding the serial number as QUERY or something
            ////
            // Before continuing it is needed to ask for a specified ADQ serial number of the device to connect to it!
            std::cout << "Enter device Serial Number (e.g. 06215, 06302):" << std::endl;
            std::cin >> adqSnReqRaw;
            std::string prefix = "SPD-";
            prefix.insert(4, adqSnReqRaw);
            adqSnReqTmp = prefix.c_str();
            adqSnReq = (char*)adqSnReqTmp;


            //// urojec L3: In such long cases where success means a bunch of code to execute I                   
            ////            find it more readable if the nonsucessfull case is handled first (the !success),
            ////            since that one only has a few lines and you iommediately see what happens, instead
            ////            of having to scroll like crazy (jsut a preference though)

            if (!success)
            {
                throw nds::NdsError("ERROR: Listing all connected devices failed.");
            }
            else
            {
                if (adqDevList <= 0)
                {
                    throw nds::NdsError("ERROR: No ADQ devices found.");
                }
                else
                {
                    /* This block searches a device with a requested serial number
                    */
                    for (adqDevListNum = 0; adqDevListNum < adqDevList; ++adqDevListNum)
                    {
                        // Opens communication channel to a certain ADQ device;
                        // this ADQ will show up in lists of functions NofADQ and GetADQ
                        success = ADQControlUnit_OpenDeviceInterface(m_adqCtrlUnitPtr, adqDevListNum);
                        if (!success)
                        {
                            throw nds::NdsError("ERROR: Device failure during interface opening.");
                            std::cout << "ERROR: " << "Device failure during interface opening" << std::endl;
                        }
                        else
                        {
                            // Make this device ready to use
                            success = ADQControlUnit_SetupDevice(m_adqCtrlUnitPtr, adqDevListNum);
                            if (!success)
                            {
                                throw nds::NdsError("ERROR: Device failure during setup.");
                            }
                            else
                            {
                                // Get pointer to interface of certain device
                                m_adqDevPtr = ADQControlUnit_GetADQ(m_adqCtrlUnitPtr, adqDevListNum + 1);
                                // Check if this ADQ serial number is the one specified
                                adqSnRdbk = m_adqDevPtr->GetBoardSerialNumber();
                                if (!strcmp(adqSnReq, adqSnRdbk))
                                {
                                    adqReqFound = true;
                                    std::cout << "DEBUG: " << "Requested ADQ device is found; Serial Number: " << adqSnRdbk << std::endl;
                                    break;
                                }
                            } // ADQControlUnit_SetupDevice
                        } // ADQControlUnit_OpenDeviceInterface
                    } // Requested device search for-loop 
                    if (!adqReqFound)
                    {
                        throw nds::NdsError("ERROR: Device was not found.");
                        std::cout << "ERROR: " << "Device was not found" << std::endl;
                    }
                    else
                    {
                        // Check if ADQ started normally
                        success = m_adqDevPtr->IsStartedOK();
                        if (!success)
                        {
                            throw nds::NdsError("ERROR: Device didn't start normally.");
                            std::cout << "ERROR: " << "Device didn't start normally" << adqSnRdbk << std::endl;
                        }
                        else
                        {
                            // Get a pointer to device specific class
                            int adqType = m_adqDevPtr->GetADQType();
                            const char* adqSn = adqSnReqRaw;
                            if (adqType == 714 || adqType == 14)
                            {
                                std::shared_ptr<ADQFourteen> adqDevSpecific = std::make_shared<ADQFourteen>(adqSn, m_node, m_adqDevPtr);
                                m_adqFrtnPtr.push_back(adqDevSpecific);
                            }
                            if (adqType == 7)
                            {
                                std::shared_ptr<ADQSeven> adqDevSpecific = std::make_shared<ADQSeven>(adqSn, m_node, m_adqDevPtr);
                                m_adqSvnPtr.push_back(adqDevSpecific);
                            } 

                            std::shared_ptr<ADQInfo> adqInfo = std::make_shared<ADQInfo>(adqSn, m_node, m_adqDevPtr);
                            m_adqInfoPtr.push_back(adqInfo);

                            // Set information log level
                            m_node.setLogLevel(nds::logLevel_t::info);

                            // Initialize certain device after declaration of all its PVs
                            m_node.initialize(this, factory);

                            //Test streams
                            ndsInfoStream(m_node) << "test INFO" << std::endl;
                            ndsWarningStream(m_node) << "test WARNING" << std::endl;
                            ndsErrorStream(m_node) << "test ERROR" << std::endl;

                            unsigned int chanCnt = m_adqDevPtr->GetNofChannels();
                            ndsInfoStream(m_node) << "DEBUG: Device has " << chanCnt << " channels." << std::endl;

                        } // IsStartedOK
                    }
                } // adqDevList > 0
            } // ADQControlUnit_ListDevices 
        } // CreateADQControlUnit
    }
    catch (nds::NdsError)
    {
        DeleteADQControlUnit(m_adqCtrlUnitPtr);
        std::cout << "DELETED: ADQ Control Unit was deleted due to error." << std::endl;
    }
}

ADQDevice::~ADQDevice()
{
    //// urojec L3: (check) everything is ok with this, such as are there any buffers that that
    //// thing holds pointers to that need to be freed etc. 
    DeleteADQControlUnit(m_adqCtrlUnitPtr);
    ndsInfoStream(m_node) << "ADQ Control Unit was destructed." << std::endl;
}

// The following MACRO defines the function to be exported in order
//  to allow the dynamic loading of the shared module
NDS_DEFINE_DRIVER(adq, ADQDevice)
