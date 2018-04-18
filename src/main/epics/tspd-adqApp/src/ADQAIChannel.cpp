#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQAIChannel::ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum, ADQInterface *& adq_dev) :
    m_channelNum(channelNum),
    m_adq_dev(adq_dev)

{

    m_node = parentNode.addChild(nds::Node(name));
/*
    m_stateMachine = nds::StateMachine(true,
                                     std::bind(&ADQAIChannel::switchOn, this),
                                     std::bind(&ADQAIChannel::switchOff, this),
                                     std::bind(&ADQAIChannel::start, this),
                                     std::bind(&ADQAIChannel::stop, this),
                                     std::bind(&ADQAIChannel::recover, this),
                                     std::bind(&ADQAIChannel::allowChange, this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2,
                                                 std::placeholders::_3));

    m_node.addChild(m_stateMachine);

    int SetLvlTrigChannel(int channel);



    void ADQAIChannel::switchOn()
    {

    }

    void ADQAIChannel::switchOff();
    void ADQAIChannel::start();
    void ADQAIChannel::stop();
    void ADQAIChannel::recover();
    bool ADQAIChannel::allowChange(const nds::state_t, const nds::state_t, const nds::state_t); 
    
    */
}
