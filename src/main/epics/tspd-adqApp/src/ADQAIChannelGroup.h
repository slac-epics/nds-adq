#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include "ADQAIChannel.h"
#include "ADQDevice.h"
#include "ADQDefinition.h"

#include <nds3/nds.h>
#include <mutex>

/*! @brief This structure is used by Triggered streaming DAQ mode.
 */
typedef struct
{
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

/*! @brief This class handles majority of parameters for correct setup of each data acquisition mode.
 *
 * Data acquisition is handled in this class. The state machine of the device is defined here.
 * Each digitizer's channel gets a representation by calling ADQCHannel constructor for N amount of physical channels.
 * 
 * @param name a name with which this class will register its child node.
 * @param parentNode a name of a parent node to which this class' node is a child.
 * @param adqInterface a pointer to the ADQ API interface created in the ADQInit class.
 * @sa ADQInit(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters)
 */
class ADQAIChannelGroup : public ADQDevice
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface*& adqInterface);
    virtual ~ADQAIChannelGroup();

    nds::Port m_node;
    nds::StateMachine m_stateMachine;

    std::vector<std::shared_ptr<ADQAIChannel>> m_AIChannelsPtr;

    /*! @brief This function creates the most common type of PV and sets it readback PV to interrupt mode.
    */
    template <typename T>
    void createPv(const std::string& name, nds::PVDelegateIn<T>& pvRb,
                  std::function<void(ADQAIChannelGroup*, const timespec&, const T&)> setter,
                  std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter);

    /*! @brief This function creates the Enumeration type of PV and sets it readback PV to interrupt mode.
    */
    template <typename T>
    void createPvEnum(const std::string& name, nds::PVDelegateIn<T>& pvRb, nds::enumerationStrings_t enumList,
                      std::function<void(ADQAIChannelGroup*, const timespec&, const T&)> setter,
                      std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter);

    /*! @brief This function creates and returns the readback PV.
    */
    template <typename T>
    nds::PVDelegateIn<T> createPvRb(const std::string& name, std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter);

    void setDaqMode(const timespec& pTimestamp, const int32_t& pValue);
    void getDaqMode(timespec* pTimestamp, int32_t* pValue);

    void setTrigMode(const timespec& pTimestamp, const int32_t& pValue);
    void getTrigMode(timespec* pTimestamp, int32_t* pValue);

    void setDbsBypass(const timespec& pTimestamp, const int32_t& pValue);
    void getDbsBypass(timespec* pTimestamp, int32_t* pValue);
    void setDbsDc(const timespec& pTimestamp, const int32_t& pValue);
    void getDbsDc(timespec* pTimestamp, int32_t* pValue);
    void setDbsLowSat(const timespec& pTimestamp, const int32_t& pValue);
    void getDbsLowSat(timespec* pTimestamp, int32_t* pValue);
    void setDbsUpSat(const timespec& pTimestamp, const int32_t& pValue);
    void getDbsUpSat(timespec* pTimestamp, int32_t* pValue);

    void setPatternMode(const timespec& pTimestamp, const int32_t& pValue);
    void getPatternMode(timespec* pTimestamp, int32_t* pValue);

    void setChanActive(const timespec& pTimestamp, const int32_t& pValue);
    void getChanActive(timespec* pTimestamp, int32_t* pValue);
    void getChanMask(timespec* pTimestamp, int32_t* pValue);

    void setRecordCnt(const timespec& pTimestamp, const int32_t& pValue);
    void getRecordCnt(timespec* pTimestamp, int32_t* pValue);

    void setRecordCntCollect(const timespec& pTimestamp, const int32_t& pValue);
    void getRecordCntCollect(timespec* pTimestamp, int32_t* pValue);

    void setSampleCnt(const timespec& pTimestamp, const int32_t& pValue);
    void getSampleCnt(timespec* pTimestamp, int32_t* pValue);
    void getSampleCntMax(timespec* pTimestamp, int32_t* pValue);
    void getSamplesTotal(timespec* pTimestamp, int32_t* pValue);
    void setSampleSkip(const timespec& pTimestamp, const int32_t& pValue);
    void getSampleSkip(timespec* pTimestamp, int32_t* pValue);
    void setSampleDec(const timespec& pTimestamp, const int32_t& pValue);
    void getSampleDec(timespec* pTimestamp, int32_t* pValue);
    void setPreTrigSamp(const timespec& pTimestamp, const int32_t& pValue);
    void getPreTrigSamp(timespec* pTimestamp, int32_t* pValue);
    void setTrigHoldOffSamp(const timespec& pTimestamp, const int32_t& pValue);
    void getTrigHoldOffSamp(timespec* pTimestamp, int32_t* pValue);

    void setClockSrc(const timespec& pTimestamp, const int32_t& pValue);
    void getClockSrc(timespec* pTimestamp, int32_t* pValue);
    void setClockRefOut(const timespec& pTimestamp, const int32_t& pValue);
    void getClockRefOut(timespec* pTimestamp, int32_t* pValue);

    void setTimeout(const timespec& pTimestamp, const int32_t& pValue);
    void getTimeout(timespec* pTimestamp, int32_t* pValue);

    void setStreamTime(const timespec& pTimestamp, const double& pValue);
    void getStreamTime(timespec* pTimestamp, double* pValue);

    void setSWTrigEdge(const timespec& pTimestamp, const int32_t& pValue);
    void getSWTrigEdge(timespec* pTimestamp, int32_t* pValue);
    void setLevelTrigLvl(const timespec& pTimestamp, const int32_t& pValue);
    void getLevelTrigLvl(timespec* pTimestamp, int32_t* pValue);
    void setLevelTrigEdge(const timespec& pTimestamp, const int32_t& pValue);
    void getLevelTrigEdge(timespec* pTimestamp, int32_t* pValue);
    void setLevelTrigChan(const timespec& pTimestamp, const int32_t& pValue);
    void getLevelTrigChan(timespec* pTimestamp, int32_t* pValue);
    void getLevelTrigChanMask(timespec* pTimestamp, int32_t* pValue);
    void setExternTrigDelay(const timespec& pTimestamp, const int32_t& pValue);
    void getExternTrigDelay(timespec* pTimestamp, int32_t* pValue);
    void setExternTrigThreshold(const timespec& pTimestamp, const double& pValue);
    void getExternTrigThreshold(timespec* pTimestamp, double* pValue);
    void setExternTrigEdge(const timespec& pTimestamp, const int32_t& pValue);
    void getExternTrigEdge(timespec* pTimestamp, int32_t* pValue);
    void setInternTrigHighSamp(const timespec& pTimestamp, const int32_t& pValue);
    void getInternTrigHighSamp(timespec* pTimestamp, int32_t* pValue);
    void setInternTrigLowSamp(const timespec& pTimestamp, const int32_t& pValue);
    void getInternTrigLowSamp(timespec* pTimestamp, int32_t* pValue);
    void setInternTrigFreq(const timespec& pTimestamp, const int32_t& pValue);
    void getInternTrigFreq(timespec* pTimestamp, int32_t* pValue);
    void setInternTrigEdge(const timespec& pTimestamp, const int32_t& pValue);
    void getInternTrigEdge(timespec* pTimestamp, int32_t* pValue);

    void getLogMsg(timespec* pTimestamp, std::string* pValue);

    void onSwitchOn();
    void onSwitchOff();
    void onStart();
    void onStop();
    void recover();
    bool allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal);

    /*! @brief This method processes Triggered streaming data acquisition.
    */
    void daqTrigStream();

    /*! @brief This method processes Multi-Record data acquisition.
    */
    void daqMultiRecord();

    /*! @brief This method processes Continuous streaming data acquisition.
    */
    void daqContinStream();

    /*! @brief This method processes Raw streaming data acquisition.
    */
    void daqRawStream();

private:
    ADQInterface* m_adqInterface;

    unsigned int m_chanCnt;
    int m_adqType;

    int32_t m_daqMode;
    bool m_daqModeChanged;

    int32_t m_patternMode;
    bool m_patternModeChanged;

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
    bool m_sampleDecChanged;
    int32_t m_sampleDec;
    bool m_preTrigSampChanged;
    int32_t m_preTrigSamp;
    bool m_trigHoldOffSampChanged;
    int32_t m_trigHoldOffSamp;

    bool m_clockSrcChanged;
    int32_t m_clockSrc;
    bool m_clockRefOutChanged;
    int32_t m_clockRefOut;

    int32_t m_chanActive;
    bool m_chanActiveChanged;
    unsigned int m_chanInt;
    int32_t m_chanMask;   // Variations: 0x1 (A), 0x2 (B), 0x4 (C), 0x8 (D), 0x3 (AB), 0xC (CD), 0xF (ABCD)

    int32_t m_trigMode;
    bool m_trigModeChanged;
    int32_t m_swTrigEdge;
    bool m_swTrigEdgeChanged;
    int32_t m_levelTrigLvl;
    bool m_levelTrigLvlChanged;
    int32_t m_levelTrigEdge;
    bool m_levelTrigEdgeChanged;
    int32_t m_levelTrigChan;
    int32_t m_levelTrigChanMask;
    bool m_levelTrigChanChanged;
    int32_t m_externTrigDelay;
    bool m_externTrigDelayChanged;
    double m_externTrigThreshold;
    bool m_externTrigThresholdChanged;
    int32_t m_externTrigEdge;
    bool m_externTrigEdgeChanged;
    int32_t m_internTrigHighSamp;
    bool m_internTrigHighSampChanged;
    int32_t m_internTrigLowSamp;
    bool m_internTrigLowSampChanged;
    int32_t m_internTrigFreq;
    bool m_internTrigFreqChanged;
    int32_t m_internTrigPeriod;
    int32_t m_internTrigEdge;
    bool m_internTrigEdgeChanged;

    int32_t m_overVoltProtect;
    bool m_overVoltProtectChanged;

    bool m_timeoutChanged;
    int32_t m_timeout;

    bool m_streamTimeChanged;
    double m_streamTime;

    /*! @brief This method processes changes applied to channel specific parameters.
    */
    void commitChanges(bool calledFromDaqThread = false);

    nds::PVDelegateIn<std::string> m_logMsgPV;
    nds::PVDelegateIn<int32_t> m_daqModePV;
    nds::PVDelegateIn<int32_t> m_patternModePV;
    nds::PVDelegateIn<int32_t> m_chanActivePV;
    nds::PVDelegateIn<int32_t> m_chanMaskPV;
    nds::PVDelegateIn<int32_t> m_dbsBypassPV;
    nds::PVDelegateIn<int32_t> m_dbsDcPV;
    nds::PVDelegateIn<int32_t> m_dbsLowSatPV;
    nds::PVDelegateIn<int32_t> m_dbsUpSatPV;
    nds::PVDelegateIn<int32_t> m_recordCntPV;
    nds::PVDelegateIn<int32_t> m_recordCntCollectPV;
    nds::PVDelegateIn<int32_t> m_sampleCntPV;
    nds::PVDelegateIn<int32_t> m_sampleCntMaxPV;
    nds::PVDelegateIn<int32_t> m_sampleCntTotalPV;
    nds::PVDelegateIn<int32_t> m_sampleSkipPV;
    nds::PVDelegateIn<int32_t> m_sampleDecPV;
    nds::PVDelegateIn<int32_t> m_preTrigSampPV;
    nds::PVDelegateIn<int32_t> m_trigHoldOffSampPV;
    nds::PVDelegateIn<int32_t> m_clockSrcPV;
    nds::PVDelegateIn<int32_t> m_clockRefOutPV;
    nds::PVDelegateIn<int32_t> m_trigModePV;
    nds::PVDelegateIn<int32_t> m_swTrigEdgePV;
    nds::PVDelegateIn<int32_t> m_levelTrigLvlPV;
    nds::PVDelegateIn<int32_t> m_levelTrigEdgePV;
    nds::PVDelegateIn<int32_t> m_levelTrigChanPV;
    nds::PVDelegateIn<int32_t> m_levelTrigChanMaskPV;
    nds::PVDelegateIn<int32_t> m_externTrigDelayPV;
    nds::PVDelegateIn<double> m_externTrigThresholdPV;
    nds::PVDelegateIn<int32_t> m_externTrigEdgePV;
    nds::PVDelegateIn<int32_t> m_internTrigHighSampPV;
    nds::PVDelegateIn<int32_t> m_internTrigLowSampPV;
    nds::PVDelegateIn<int32_t> m_internTrigFreqPV;
    nds::PVDelegateIn<int32_t> m_internTrigEdgePV;
    nds::PVDelegateIn<int32_t> m_timeoutPV;
    nds::PVDelegateIn<double> m_streamTimePV;

    nds::Thread m_daqThread;
    bool m_stopDaq;

    short* m_daqRawDataBuffer;
    short* m_daqDataBuffer[CHANNEL_COUNT_MAX];
    unsigned char* m_daqRecordHeaders[CHANNEL_COUNT_MAX];
    streamingHeader_t* m_daqStreamHeaders[CHANNEL_COUNT_MAX];
    short* m_daqLeftoverSamples[CHANNEL_COUNT_MAX];
};

#endif /* ADQAICHANNELGROUP_H */
