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

    short* buf_a;
    short* buf_b;
    short* buf_c;
    short* buf_d;

    void* target_buf[8];

    void setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTriggerMode(timespec* pTimestamp, std::int32_t* pValue);

    void setAdjustBias(const timespec &pTimestamp, const std::int32_t &pValue);
    void getAdjustBias(timespec* pTimestamp, std::int32_t* pValue);

    void setDBS(const timespec &pTimestamp, const std::vector<std::int32_t> &pValue);
    void getDBS(timespec* pTimestamp, std::vector<std::int32_t>* pValue);

    void setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getPatternMode(timespec* pTimestamp, std::int32_t* pValue);

    void setChannels(const timespec &pTimestamp, const std::int32_t &pValue);
    void getChannels(timespec* pTimestamp, std::int32_t* pValue);
    void getChannelMask(timespec* pTimestamp, std::string* pValue);

    void setNofRecords(const timespec &pTimestamp, const std::int32_t &pValue);
    void getNofRecords(timespec* pTimestamp, std::int32_t* pValue);

    void getMaxSamples(timespec* pTimestamp, std::int32_t* pValue);

    void setNofSamples(const timespec &pTimestamp, const std::int32_t &pValue);
    void getNofSamples(timespec* pTimestamp, std::int32_t* pValue);

    void setDAQMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDAQMode(timespec* pTimestamp, std::int32_t* pValue);

    void setRecords(const timespec &pTimestamp, const std::int32_t &pValue);
    void getRecords(timespec* pTimestamp, std::int32_t* pValue);

    void commitChanges(bool calledFromAcquisitionThread = false);

    void onSwitchOn();
    void onSwitchOff();
    void onStart();
    void onStop();
    void recover();
    bool allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal);

    void acquisition();
    
private:
    ADQInterface * m_adq_dev;

    unsigned int success;
    unsigned int nofchan;
    unsigned int ch;

    int32_t m_trigmode;    
    int32_t m_adjustBias;
    
    unsigned int dbs_n_of_inst;
    unsigned char dbs_inst;
    std::vector<std::int32_t> m_dbs_settings; // 0 - bypass, 1 - DC target, 2 - lower saturation lvl, 3 - upper saturation lvl
    
    int32_t m_pattmode;
    
    int32_t m_nofrecords;
    unsigned int max_nofsamples;
    int32_t m_maxsamples;
    int32_t m_nofsamples;
    
    int32_t m_daqmode;
    
    int32_t m_channelbits; // Four bits: 0 - channel A; 1 - A and B; 2 - A, B and C; 3 - all channels (ABCD) -- make dropdown in GUI!
    unsigned char m_channelmask; // Four variations: 0x01 (ch A), 0x02 (ch AB), 0x04 (ch ABC), 0x08 (all ch)
    
    bool m_trigmodeChanged;
    bool m_biasChanged;
    bool m_dbsChanged;
    bool m_pattmodeChanged;
    bool m_nofrecordsChanged;
    bool m_nofsamplesChanged;
    bool m_daqmodeChanged;
    bool m_channelmaskChanged;

    nds::PVDelegateIn<std::int32_t> m_trigmodePV;
    nds::PVDelegateIn<std::int32_t> m_adjustBiasPV;
    nds::PVDelegateIn<std::vector<std::int32_t>> m_dbs_settingsPV;
    nds::PVDelegateIn<std::int32_t> m_pattmodePV;
    nds::PVDelegateIn<std::int32_t> m_channelbitsPV;
    nds::PVDelegateIn<std::string> m_channelmaskPV;
    nds::PVDelegateIn<std::int32_t> m_nofrecordsPV;
    nds::PVDelegateIn<std::int32_t> m_maxsamplesPV;
    nds::PVDelegateIn<std::int32_t> m_nofsamplesPV;
    nds::PVDelegateIn<std::int32_t> m_daqmodePV;
    nds::PVDelegateIn<std::int32_t> m_recordsPV;

    nds::Thread m_acquisitionThread;

    // void acquisitionLoop(int32_t repeat);
    bool m_stop;
};

#endif /* ADQAICHANNELGROUP_H */
