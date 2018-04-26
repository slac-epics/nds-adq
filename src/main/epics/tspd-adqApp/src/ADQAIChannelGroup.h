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

    void setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTriggerMode(timespec* pTimestamp, std::int32_t* pValue);

    void setAdjustBias(const timespec &pTimestamp, const std::int32_t &pValue);
    void getAdjustBias(timespec* pTimestamp, std::int32_t* pValue);

    void setDBS(const timespec &pTimestamp, const std::vector<std::int32_t> &pValue);
    void getDBS(timespec* pTimestamp, std::vector<std::int32_t>* pValue);

    void setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getPatternMode(timespec* pTimestamp, std::int32_t* pValue);

    void setChanMask(const timespec &pTimestamp, const std::string &pValue);
    void getChanMask(timespec* pTimestamp, std::string* pValue);

    void commitChanges();

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

    // Number of channels
    unsigned int n_of_chan;

    // Channel number
    unsigned int ch;

    int32_t m_trigmode;
    bool m_trigmodeChanged;
    
    // Bias ADC code (ADC offset)
    int m_adjustBias; 
    bool m_biasChanged;

    // DBS setup
    unsigned int dbs_n_of_inst;
    unsigned char dbs_inst;
    std::vector<std::int32_t> m_dbs_settings; // 0 - bypass, 1 - DC target, 2 - lower saturation lvl, 3 - upper saturation lvl
    bool m_dbsChanged;
    
    // Pattern mode
    int32_t m_pattmode;
    bool m_pattmodeChanged;

    // Channel enabling
    std::string m_channels; // Four bits: 0 - channel A; 1 - A and B; 2 - A, B and C; 3 - all channels (ABCD) -- make dropdown in GUI!
    unsigned char m_channelmask; // Four variations: 0x01 (ch A), 0x02 (ch AB), 0x04 (ch ABC), 0x08 (all ch)
    bool m_channelmaskChanged;


    nds::PVDelegateIn<std::int32_t> m_trigmodePV;
    nds::PVDelegateIn<std::int32_t> m_adjustBiasPV;
    nds::PVDelegateIn<std::vector<std::int32_t>> m_dbs_settingsPV;
    nds::PVDelegateIn<std::int32_t> m_pattmodePV;
    nds::PVDelegateIn<std::string> m_channelmaskPV;
};

#endif /* ADQAICHANNELGROUP_H */
