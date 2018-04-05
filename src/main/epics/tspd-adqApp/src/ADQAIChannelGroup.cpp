#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface*& adqDevice) :
    m_node(nds::Port(name, nds::nodeType_t::generic))

{
    /*
    void ADQAIChannelGroup::getDeviceInfo()
    {
        char* adq_pn = m_adq_dev->GetBoardProductName();
        unsigned int adq_pid = m_adq_dev->GetProductID();
        int adq_type = m_adq_dev->GetADQType();
        const char* adq_opt = m_adq_dev->GetCardOption();
    }
    */
}
