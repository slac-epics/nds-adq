#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include <nds3/nds.h>
#include <time.h>

typedef struct
{
    unsigned char RecordStatus;
    unsigned char UserID;
    unsigned char Channel;
    unsigned char DataFormat;
    unsigned int SerialNumber;
    unsigned int RecordNumber;
    unsigned int SamplePeriod;
    unsigned long long Timestamp;
    unsigned long long RecordStart;
    unsigned int RecordLength;
    unsigned int Reserved;
} StreamingHeader_t;

class ADQAIChannelGroup
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev);

    nds::Port m_node;
    nds::StateMachine m_stateMachine;

    uint32_t m_numChannels;
    std::vector<std::shared_ptr<ADQAIChannel> > m_AIChannels;

    void setDAQMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDAQMode(timespec* pTimestamp, std::int32_t* pValue);

    void setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTriggerMode(timespec* pTimestamp, std::int32_t* pValue);

    void setAdjustBias(const timespec &pTimestamp, const std::int32_t &pValue);
    void getAdjustBias(timespec* pTimestamp, std::int32_t* pValue);

    void setDBSbypass(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDBSbypass(timespec* pTimestamp, std::int32_t* pValue);
    void setDBSdc(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDBSdc(timespec* pTimestamp, std::int32_t* pValue);
    void setDBSlowsat(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDBSlowsat(timespec* pTimestamp, std::int32_t* pValue);
    void setDBSupsat(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDBSupsat(timespec* pTimestamp, std::int32_t* pValue);

    void setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getPatternMode(timespec* pTimestamp, std::int32_t* pValue);

    void setChannels(const timespec &pTimestamp, const std::int32_t &pValue);
    void getChannels(timespec* pTimestamp, std::int32_t* pValue);
    void setChannelMask(const timespec &pTimestamp, const std::string &pValue);
    void getChannelMask(timespec* pTimestamp, std::string* pValue);

    void setNofRecords(const timespec &pTimestamp, const std::int32_t &pValue);
    void getNofRecords(timespec* pTimestamp, std::int32_t* pValue);

    void setCollectRecords(const timespec &pTimestamp, const std::int32_t &pValue);
    void getCollectRecords(timespec* pTimestamp, std::int32_t* pValue);

    void getMaxSamples(timespec* pTimestamp, std::int32_t* pValue);

    void setNofSamples(const timespec &pTimestamp, const std::int32_t &pValue);
    void getNofSamples(timespec* pTimestamp, std::int32_t* pValue);
    void getTotalSamples(timespec* pTimestamp, std::int32_t* pValue);

    void setTriggerLvl(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTriggerLvl(timespec* pTimestamp, std::int32_t* pValue);
    void setTriggerEdge(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTriggerEdge(timespec* pTimestamp, std::int32_t* pValue);
    void setTriggerChan(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTriggerChan(timespec* pTimestamp, std::int32_t* pValue);
    
    void commitChanges(bool calledFromAcquisitionThread = false);

    void onSwitchOn();
    void onSwitchOff();
    void onStart();
    void onStop();
    void recover();
    bool allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal);

    void acquisition_trigstream();
    void acquisition_multirec();
    void acquisition_contstream();

    void adq14_triggered_streaming_process_record(short* record_data, StreamingHeader_t* record_header);
    
private:
    ADQInterface * m_adq_dev;

    unsigned int success;
    unsigned int nofchan;
    int ch;

    int32_t m_daqmode;
    int32_t m_trigmode;    
    int32_t m_pattmode;
    int32_t m_adjustBias;
    
    unsigned int dbs_n_of_inst;
    unsigned char dbs_inst;
    int32_t m_dbs_bypass;
    int32_t m_dbs_dctarget;
    int32_t m_dbs_lowsat;
    int32_t m_dbs_upsat;
 
    unsigned int nofrecords_sum;
    unsigned int max_nofsamples;
    int32_t m_nofrecords;
    int32_t m_collect_records;
    int32_t m_maxsamples;
    int32_t m_nofsamples;
    int32_t m_totalsamples;
    
    int32_t m_channels;
    int32_t m_channelbits; // Four bits: 0 - channel A; 1 - A and B; 2 - A, B and C; 3 - all channels (ABCD) -- make dropdown in GUI!
    std::string m_channelmask; // Four variations: 0x01 (ch A), 0x02 (ch AB), 0x04 (ch ABC), 0x08 (all ch)
    std::string m_channels_str;
    unsigned char channelmask_char;

    int32_t m_triglvl;
    int32_t m_trigedge;
    int32_t m_trigchan;
    int32_t m_trigfreq;
    int trigchan_int;
    
    bool m_trigmodeChanged;
    bool m_biasChanged;
    bool m_dbsbypassChanged;
    bool m_dbsdcChanged;
    bool m_dbslowsatChanged;
    bool m_dbsupsatChanged;
    bool m_pattmodeChanged;
    bool m_nofrecordsChanged;
    bool m_collect_recordsChanged;
    bool m_nofsamplesChanged;
    bool m_daqmodeChanged;
    bool m_channelsChanged;
    bool m_channelmaskChanged;
    bool m_triglvlChanged;
    bool m_trigedgeChanged;
    bool m_trigchanChanged;

    nds::PVDelegateIn<std::int32_t> m_daqmodePV;
    nds::PVDelegateIn<std::int32_t> m_trigmodePV;
    nds::PVDelegateIn<std::int32_t> m_adjustBiasPV;
    nds::PVDelegateIn<std::int32_t> m_dbs_bypassPV;
    nds::PVDelegateIn<std::int32_t> m_dbs_dctargetPV;
    nds::PVDelegateIn<std::int32_t> m_dbs_lowsatPV;
    nds::PVDelegateIn<std::int32_t> m_dbs_upsatPV;
    nds::PVDelegateIn<std::int32_t> m_pattmodePV;
    nds::PVDelegateIn<std::int32_t> m_channelsPV;
    nds::PVDelegateIn<std::string> m_channelmaskPV;
    nds::PVDelegateIn<std::int32_t> m_nofrecordsPV;
    nds::PVDelegateIn<std::int32_t> m_collect_recordsPV;
    nds::PVDelegateIn<std::int32_t> m_maxsamplesPV;
    nds::PVDelegateIn<std::int32_t> m_nofsamplesPV;
    nds::PVDelegateIn<std::int32_t> m_totalsamplesPV;

    nds::PVDelegateIn<std::int32_t> m_triglvlPV;
    nds::PVDelegateIn<std::int32_t> m_trigedgePV;
    nds::PVDelegateIn<std::int32_t> m_trigchanPV;

    nds::Thread m_acquisitionThread;

    // void acquisitionLoop(int32_t repeat);
    bool m_stop;

};

#endif /* ADQAICHANNELGROUP_H */
