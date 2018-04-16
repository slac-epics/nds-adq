#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include <nds3/nds.h>

class ADQAIChannelGroup
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& m_adq_dev);

    nds::Port m_node;
    nds::StateMachine m_stateMachine;

    uint32_t m_numChannels;
    std::vector<std::shared_ptr<ADQAIChannel> > m_AIChannels;


protected:
    // PVs connected to EPICS records
    nds::PVVariableIn<std::string> m_productName;
    nds::PVVariableIn<std::string> m_serialNumber;
    nds::PVVariableIn<std::string> m_productID;
    nds::PVVariableIn<std::string> m_adqType;
    nds::PVVariableIn<std::string> m_cardOption;
};

#endif /* ADQAICHANNELGROUP_H */
