#ifndef ADQINIT_H
#define ADQINIT_H

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"

#include <ADQAPI.h>
#include <nds3/nds.h>
#include <mutex>

/*! @brief This class creates a device that communicates with a digitizer.
 *
 * ADQ Control Unit is handled by this class. The pointer to ADQAPI interface is also created here.
 *
 * @param factory contains an interface to the control system that requested a creation of the device.
 * @param deviceName a name with which the device should be presented to the control system (root node).
 * @param parameters here a serial number of the requested digitizer is passed to the device. 
 */
class ADQInit
{
public:
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
