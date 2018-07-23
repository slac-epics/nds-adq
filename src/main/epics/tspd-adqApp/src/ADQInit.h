#ifndef ADQINIT_H
#define ADQINIT_H

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"

#include <ADQAPI.h>
#include <nds3/nds.h>
#include <mutex>

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
