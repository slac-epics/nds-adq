#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include <nds3/nds.h>
#include <time.h>


class ADQAIChannelGroup
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev);

    nds::Port m_node;
    nds::StateMachine m_stateMachine;

    uint32_t m_numChannels;
    std::vector<std::shared_ptr<ADQAIChannel> > m_AIChannels;

    void getTriggerMode(timespec* pTimestamp, std::int32_t* pValue);
    void setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue);

    void onSwitchOn();
    void onSwitchOff();
    void onStart();
    void onStop();
    void recover();
    bool allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal);
    
private:
    // pointer to certain ADQ device
    ADQInterface * m_adq_dev;

    // Success status
    unsigned int success;

    int32_t m_trigmode;
    bool m_trigmodeChanged;

    // Success status
    unsigned int n_of_chan;

    nds::PVDelegateIn<std::int32_t> m_trigmodePV;

};

#endif /* ADQAICHANNELGROUP_H */
