#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include <nds3/nds.h>

class ADQAIChannelGroup
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode);

    nds::Port m_node;
    nds::StateMachine m_stateMachine;

    uint32_t m_numChannels;
    std::vector<std::shared_ptr<ADQAIChannel> > m_AIChannels;

protected:
    ADQInterface * m_adq_dev;
};

#endif /* ADQAICHANNELGROUP_H */
