#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

class ADQAIChannel
{
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum);

    std::int32_t m_channelNum;

protected:
    ADQInterface * m_adq_dev;

private:
    nds::Node m_node;
    nds::StateMachine m_stateMachine;
    
    int channel;
    unsigned char channelsmask;

    /*
    void switchOn();
    void switchOff();
    void start();
    void stop();
    void recover();
    bool allowChange(const nds::state_t, const nds::state_t, const nds::state_t);
    */
};

#endif /* ADQAICHANNEL_H */