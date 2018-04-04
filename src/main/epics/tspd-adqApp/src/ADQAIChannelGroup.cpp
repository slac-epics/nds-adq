#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode) :
    m_node(nds::Port(name, nds::nodeType_t::generic))

{

}
