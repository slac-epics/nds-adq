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

    void getAdjustBias(timespec* pTimestamp, std::int32_t* pValue);
    void setAdjustBias(const timespec &pTimestamp, const std::int32_t &pValue);

    void setDBS(const timespec &pTimestamp, const std::int32_t &pValue);

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

    // Channel number
    unsigned int ch;

    int32_t m_trigmode;
    bool m_trigmodeChanged;
    
    // Bias ADC code (ADC offset)
    int m_adjustBias; 
    bool m_biasChanged;

    // DBS settings
    unsigned int m_dbs_bypass;
    int m_dbs_dctarget;
    int m_dbs_lowsat;
    int m_dbs_upsat;
    unsigned int dbs_n_of_inst;
    unsigned char dbs_inst;

    unsigned int n_of_chan;


    nds::PVDelegateIn<std::int32_t> m_trigmodePV;
    nds::PVDelegateIn<std::int32_t> m_adjustBiasPV;
    nds::PVDelegateIn<std::int32_t> m_dbsPV;

};

#endif /* ADQAICHANNELGROUP_H */
