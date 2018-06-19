#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include <nds3/nds.h>
#include "ADQAIChannel.h"

//// urojec L3: goes for all the declarations that have anything to do with channels:
//// decide weather you will use ch, chn or channel everywhere and then be consistent,
//// exaples: unsigned char Channel; setChannels, setTriggerChan


typedef struct
{
    //// urojec L3: camelCase, don't use Capital start for anything else than class names, struct names
    //// struct fields or members should be lowerCase
    unsigned char recordStatus;
    unsigned char userID;
    unsigned char channel;
    unsigned char dataFormat;
    unsigned int serialNumber;
    unsigned int recordNumber;
    unsigned int samplePeriod;
    unsigned long long timestamp;
    unsigned long long recordStart;
    unsigned int recordLength;
    unsigned int reserved;
} streamingHeader_t;

class ADQAIChannelGroup
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev);
    virtual ~ADQAIChannelGroup();

    nds::StateMachine m_stateMachine;

    unsigned int m_chanCnt;
    std::vector<std::shared_ptr<ADQAIChannel> > m_AIChannelsPtr;

    //// urojec L2: Why dso you use pointers in one direction and references in the other?
    ////            why are references not good enough for getters?
    ////            Also, tell my what is the diffrerence between const ref and ref and
    ////            why are const references being used with setters instead of having the values
    ////            passed directly into the method
    //// ppipp: I cannot tell exactly why it is made so in NDS, but setValue methods use const references and getValue methods use pointers
    ////        setValue methods are used to set a PV to requested value and getValue methods are for readback PVs; 
    ////        PVs for setters have Passive scan field and PVs for getters have I/O Interrupt scan field 
    ////                                                        (that is why there is also push functions in commitChanges);
    ////        just in case here is a link to NDS doxygen: goo.gl/tgjCG7 (search for setValue and getValue methods)
    ////        but I must note I saw this usage in Niklas' example first and then checked NDS
    ////            P.S. I got an idea to try to have only one getValue method for all Rdbk PVs to make the code cleaner, 
    ////                 since most of getValue methods need an int32_t variable... Need to try

    void setDaqMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDaqMode(timespec* pTimestamp, std::int32_t* pValue);

    void setTrigMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTrigMode(timespec* pTimestamp, std::int32_t* pValue);
    void setTrigFreq(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTrigFreq(timespec* pTimestamp, std::int32_t* pValue);

    void setDcBias(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDcBias(timespec* pTimestamp, std::int32_t* pValue);

    void setDbsBypass(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDbsBypass(timespec* pTimestamp, std::int32_t* pValue);
    void setDbsDc(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDbsDc(timespec* pTimestamp, std::int32_t* pValue);
    void setDbsLowSat(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDbsLowSat(timespec* pTimestamp, std::int32_t* pValue);
    void setDbsUpSat(const timespec &pTimestamp, const std::int32_t &pValue);
    void getDbsUpSat(timespec* pTimestamp, std::int32_t* pValue);

    void setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue);
    void getPatternMode(timespec* pTimestamp, std::int32_t* pValue);

    void setRecordCnt(const timespec &pTimestamp, const std::int32_t &pValue);
    void getRecordCnt(timespec* pTimestamp, std::int32_t* pValue);

    void setRecordCntCollect(const timespec &pTimestamp, const std::int32_t &pValue);
    void getRecordCntCollect(timespec* pTimestamp, std::int32_t* pValue);

    void setSampleCnt(const timespec &pTimestamp, const std::int32_t &pValue);
    void getSampleCnt(timespec* pTimestamp, std::int32_t* pValue);
    void getSampleCntMax(timespec* pTimestamp, std::int32_t* pValue);
    void getSamplesTotal(timespec* pTimestamp, std::int32_t* pValue);

    void setFlushTime(const timespec &pTimestamp, const std::int32_t &pValue);
    void getFlushTime(timespec* pTimestamp, std::int32_t* pValue);

    void getLogMsg(timespec* pTimestamp, std::string* pValue);

    void commitChanges(bool calledFromDaqThread = false);

    void onSwitchOn();
    void onSwitchOff();
    void onStart();
    void onStop();
    void recover();
    bool allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal);

    void daqTrigStream();
    void daqTrigStreamProcessRecord(short* recordData, streamingHeader_t* recordHeader);
    void daqMultiRecord();
    void daqContinStream();

protected:
    int32_t m_daqMode;
    bool m_daqModeChanged;
    
    int32_t m_patternMode;
    bool m_patternModeChanged;

    int32_t m_dcBias;
    bool m_dcBiasChanged;

    int32_t m_dbsBypass;
    bool m_dbsBypassChanged;
    int32_t m_dbsDc;
    bool m_dbsDcChanged;
    int32_t m_dbsLowSat;
    bool m_dbsLowSatChanged;
    int32_t m_dbsUpSat;
    bool m_dbsUpSatChanged;
    
    int32_t m_recordCnt;
    bool m_recordCntChanged;
    int32_t m_recordCntCollect;
    bool m_recordCntCollectChanged;
    
    int32_t m_sampleCnt;
    bool m_sampleCntChanged;
    int32_t m_sampleCntMax;
    int32_t m_sampleCntTotal;

    int32_t m_chanActive;     // Device specific
    bool m_chanActiveChanged; //
    int32_t m_chanBits;       // Four bits: 0 - channel A; 1 - ch B; 2 - ch C; 3 - ch D 
    std::string m_chanMask;   // Variations: 0x1 (A), 0x2 (B), 0x4 (C), 0x8 (D), 0x3 (AB), 0xC (CD), 0xF (ABCD)
    bool m_chanMaskChanged;   //
    
    int32_t m_trigMode;
    bool m_trigModeChanged;
    int32_t m_trigLvl;
    bool m_trigLvlChanged;
    int32_t m_trigEdge;
    bool m_trigEdgeChanged;
    int32_t m_trigChan;
    int32_t m_trigChanInt;
    bool m_trigChanChanged;
    int32_t m_trigFreq; 
    bool m_trigFreqChanged;
    int32_t m_trigPeriod;

    bool m_flushTimeChanged;
    int32_t m_flushTime;

    nds::PVDelegateIn<std::string> m_logMsgPV;
    
private:
    ADQInterface * m_adqDevPtr;
    nds::Port m_node;

    nds::PVDelegateIn<std::int32_t> m_daqModePV; 
    nds::PVDelegateIn<std::int32_t> m_patternModePV;
    nds::PVDelegateIn<std::int32_t> m_dcBiasPV;
    nds::PVDelegateIn<std::int32_t> m_dbsBypassPV;
    nds::PVDelegateIn<std::int32_t> m_dbsDcPV;
    nds::PVDelegateIn<std::int32_t> m_dbsLowSatPV;
    nds::PVDelegateIn<std::int32_t> m_dbsUpSatPV;   
    nds::PVDelegateIn<std::int32_t> m_recordCntPV;
    nds::PVDelegateIn<std::int32_t> m_recordCntCollectPV;
    nds::PVDelegateIn<std::int32_t> m_sampleCntMaxPV;
    nds::PVDelegateIn<std::int32_t> m_sampleCntPV;
    nds::PVDelegateIn<std::int32_t> m_sampleCntTotalPV;
    nds::PVDelegateIn<std::int32_t> m_trigModePV;
    nds::PVDelegateIn<std::int32_t> m_trigFreqPV;
    nds::PVDelegateIn<std::int32_t> m_flushTimePV;

    nds::Thread m_daqThread;
    bool m_stopDaq;
};

#endif /* ADQAICHANNELGROUP_H */
