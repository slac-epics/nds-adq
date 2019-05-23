//
// Copyright (c) 2018 Cosylab d.d.
// This software is distributed under the terms found
// in file LICENSE.txt that is included with this distribution.
//

#ifndef ADQINIT_H
#define ADQINIT_H

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"

#include <ADQAPI.h>
#include <mutex>
#include <nds3/nds.h>

/** @file ADQInit.h
 * @brief This file defines ADQInit class that creates a device.
 */

/** @brief This class creates a device that communicates with a digitizer.
 * ADQ Control Unit is handled by this class. The pointer to ADQAPI interface is also created here.
 */
class ADQInit
{
public:
    /** @fn ADQInit(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& parameters);
     * @brief ADQInit class constructor.
     * @param factory contains an interface to the control system that requested a creation of the device.
     * @param deviceName a name with which the device should be presented to the control system (root node).
     * @param parameters here a serial number of the requested digitizer is passed to the device. 
     */
    ADQInit(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& parameters);
    ~ADQInit();

private:
    nds::Node m_node;

    // Pointer to ADQ device interface
    ADQInterface* m_adqInterface;

    // ADQ Control Unit
    void* m_adqCtrlUnit;

    // Vector of pointers to Group channel class
    std::vector<std::shared_ptr<ADQAIChannelGroup>> m_adqChanGrpPtr;
};

#endif /* ADQINIT_H */
