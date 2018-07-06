#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include "ADQAIChannel.h"
#include "ADQDefinition.h"

#include <nds3/nds.h>

typedef struct
{
    //// urojec L3: camelCase, don't use Capital start for anything else than class names, struct names
    //// struct fields or members should be lowerCase
    unsigned char recordStatus;
    unsigned char userID;
    unsigned char chan;
    unsigned char dataFormat;
    unsigned int serialNumber;
    unsigned int recordNumber;
    unsigned int samplePeriod;
    unsigned long long timeStamp;
    unsigned long long recordStart;
    unsigned int recordLength;
    unsigned int reserved;
} streamingHeader_t;

class ADQAIChannelGroup
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface*& adqInterface);
    virtual ~ADQAIChannelGroup();

    nds::Port m_node;
    nds::StateMachine m_stateMachine;

    std::vector<std::shared_ptr<ADQAIChannel>> m_AIChannelsPtr;

    // This function creates the most common type of PV and sets it readback PV to interrupt mode
    template <typename T>
    void createPv(const std::string& name, nds::PVDelegateIn<T>& pvRb,
                  std::function<void(ADQAIChannelGroup*, const timespec&, const T&)> setter,
                  std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter);

    // This function creates the Enumeration type of PV and sets it readback PV to interrupt mode
    template <typename T>
    void createPvEnum(const std::string& name, nds::PVDelegateIn<T>& pvRb, nds::enumerationStrings_t enumList,
                      std::function<void(ADQAIChannelGroup*, const timespec&, const T&)> setter,
                      std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter);

    // This function creates and returns the readback PV
    template <typename T>
    nds::PVDelegateIn<T> createPvRb(const std::string& name, std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter);

    void setDaqMode(const timespec& pTimestamp, const std::int32_t& pValue);
    void getDaqMode(timespec* pTimestamp, std::int32_t* pValue);

    void setTrigMode(const timespec& pTimestamp, const std::int32_t& pValue);
    void getTrigMode(timespec* pTimestamp, std::int32_t* pValue);
    void setTrigFreq(const timespec& pTimestamp, const std::int32_t& pValue);
    void getTrigFreq(timespec* pTimestamp, std::int32_t* pValue);

    void setDcBias(const timespec& pTimestamp, const std::int32_t& pValue);
    void getDcBias(timespec* pTimestamp, std::int32_t* pValue);

    void setDbsBypass(const timespec& pTimestamp, const std::int32_t& pValue);
    void getDbsBypass(timespec* pTimestamp, std::int32_t* pValue);
    void setDbsDc(const timespec& pTimestamp, const std::int32_t& pValue);
    void getDbsDc(timespec* pTimestamp, std::int32_t* pValue);
    void setDbsLowSat(const timespec& pTimestamp, const std::int32_t& pValue);
    void getDbsLowSat(timespec* pTimestamp, std::int32_t* pValue);
    void setDbsUpSat(const timespec& pTimestamp, const std::int32_t& pValue);
    void getDbsUpSat(timespec* pTimestamp, std::int32_t* pValue);

    void setPatternMode(const timespec& pTimestamp, const std::int32_t& pValue);
    void getPatternMode(timespec* pTimestamp, std::int32_t* pValue);

    void setChanActive(const timespec& pTimestamp, const std::int32_t& pValue);
    void getChanActive(timespec* pTimestamp, std::int32_t* pValue);
    void getChanMask(timespec* pTimestamp, std::string* pValue);

    void setRecordCnt(const timespec& pTimestamp, const std::int32_t& pValue);
    void getRecordCnt(timespec* pTimestamp, std::int32_t* pValue);

    void setRecordCntCollect(const timespec& pTimestamp, const std::int32_t& pValue);
    void getRecordCntCollect(timespec* pTimestamp, std::int32_t* pValue);

    void setSampleCnt(const timespec& pTimestamp, const std::int32_t& pValue);
    void getSampleCnt(timespec* pTimestamp, std::int32_t* pValue);
    void getSampleCntMax(timespec* pTimestamp, std::int32_t* pValue);
    void getSamplesTotal(timespec* pTimestamp, std::int32_t* pValue);
    void setSampleSkip(const timespec& pTimestamp, const std::int32_t& pValue);
    void getSampleSkip(timespec* pTimestamp, std::int32_t* pValue);

    void setFlushTime(const timespec& pTimestamp, const std::int32_t& pValue);
    void getFlushTime(timespec* pTimestamp, std::int32_t* pValue);

    void setStreamTime(const timespec& pTimestamp, const double& pValue);
    void getStreamTime(timespec* pTimestamp, double* pValue);

    void setTrigLvl(const timespec& pTimestamp, const std::int32_t& pValue);
    void getTrigLvl(timespec* pTimestamp, std::int32_t* pValue);
    void setTrigEdge(const timespec& pTimestamp, const std::int32_t& pValue);
    void getTrigEdge(timespec* pTimestamp, std::int32_t* pValue);
    void setTrigChan(const timespec& pTimestamp, const std::int32_t& pValue);
    void getTrigChan(timespec* pTimestamp, std::int32_t* pValue);

    void getLogMsg(timespec* pTimestamp, std::string* pValue);

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
    void daqRawStream();

private:
    ADQInterface* m_adqInterface;

    unsigned int m_chanCnt;
    int m_adqType;

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
    bool m_sampleSkipChanged;
    int32_t m_sampleSkip;

    int32_t m_chanActive;   // Device specific
    bool m_chanActiveChanged;
    unsigned int m_chanInt;
    std::string m_chanMask;   // Variations: 0x1 (A), 0x2 (B), 0x4 (C), 0x8 (D), 0x3 (AB), 0xC (CD), 0xF (ABCD)

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

    int32_t m_overVoltProtect;
    bool m_overVoltProtectChanged;

    bool m_flushTimeChanged;
    int32_t m_flushTime;

    bool m_streamTimeChanged;
    double m_streamTime;

    void commitChanges(bool calledFromDaqThread = false);

    nds::PVDelegateIn<std::string> m_logMsgPV;
    nds::PVDelegateIn<std::int32_t> m_daqModePV;
    nds::PVDelegateIn<std::int32_t> m_patternModePV;
    nds::PVDelegateIn<std::int32_t> m_chanActivePV;
    nds::PVDelegateIn<std::string> m_chanMaskPV;
    nds::PVDelegateIn<std::int32_t> m_dcBiasPV;
    nds::PVDelegateIn<std::int32_t> m_dbsBypassPV;
    nds::PVDelegateIn<std::int32_t> m_dbsDcPV;
    nds::PVDelegateIn<std::int32_t> m_dbsLowSatPV;
    nds::PVDelegateIn<std::int32_t> m_dbsUpSatPV;
    nds::PVDelegateIn<std::int32_t> m_recordCntPV;
    nds::PVDelegateIn<std::int32_t> m_recordCntCollectPV;
    nds::PVDelegateIn<std::int32_t> m_sampleCntPV;
    nds::PVDelegateIn<std::int32_t> m_sampleCntMaxPV;
    nds::PVDelegateIn<std::int32_t> m_sampleCntTotalPV;
    nds::PVDelegateIn<std::int32_t> m_sampleSkipPV;
    nds::PVDelegateIn<std::int32_t> m_trigModePV;
    nds::PVDelegateIn<std::int32_t> m_trigFreqPV;
    nds::PVDelegateIn<std::int32_t> m_flushTimePV;
    nds::PVDelegateIn<double> m_streamTimePV;
    nds::PVDelegateIn<std::int32_t> m_trigLvlPV;
    nds::PVDelegateIn<std::int32_t> m_trigEdgePV;
    nds::PVDelegateIn<std::int32_t> m_trigChanPV;

    nds::Thread m_daqThread;
    bool m_stopDaq;

    short* m_daqBuffers[CHANNEL_NUMBER_MAX];
    void* m_daqVoidBuffers[CHANNEL_NUMBER_MAX];
    unsigned char* m_daqHeaders[CHANNEL_NUMBER_MAX];
    streamingHeader_t* m_daqStreamHeaders[CHANNEL_NUMBER_MAX];
    short* m_daqExtra[CHANNEL_NUMBER_MAX];
};

#endif /* ADQAICHANNELGROUP_H */
