#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQInfo.h"

#include <ADQAPI.h>
#include <nds3/nds.h>

class ADQDevice
{
public:
    ADQDevice(nds::Factory& factory, const std::string& deviceName, const nds::namedParameters_t& parameters);
    ~ADQDevice();

private:
    // ADQ device interface
    ADQInterface* m_adqInterface;

    // ADQ Control Unit
    void* m_adqCtrlUnit;

    // Pointer to device information class
    std::vector<std::shared_ptr<ADQInfo>> m_adqInfoPtr;

    // Pointers to Group channel class
    std::vector<std::shared_ptr<ADQAIChannelGroup>> m_adqChanGrpPtr;

    nds::Node m_node;
};

#endif /* ADQDEVICE_H */
