#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include <nds3/nds.h>
#include "ADQAIChannel.h"

#define sleep(interval) usleep(1000*interval) // usleep - microsecond interval
#define PRINT_RECORD_INFO

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

    nds::StateMachine m_stateMachine;

    uint32_t m_numChannels;
    unsigned int m_chanCnt; // Device specific
    std::vector<std::shared_ptr<ADQAIChannel> > m_AIChannelsPtr;
    //std::vector<std::shared_ptr<ADQFourteen> > m_adqFrtnPtr;

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

    //void setChanActive(const timespec &pTimestamp, const std::int32_t &pValue); // Device specific
    //void getChanActive(timespec* pTimestamp, std::int32_t* pValue);             //
    //void setChanMask(const timespec &pTimestamp, const std::string &pValue);    //
    //void getChanMask(timespec* pTimestamp, std::string* pValue);                //

    void setRecordCnt(const timespec &pTimestamp, const std::int32_t &pValue);
    void getRecordCnt(timespec* pTimestamp, std::int32_t* pValue);

    void setRecordCntCollect(const timespec &pTimestamp, const std::int32_t &pValue);
    void getRecordCntCollect(timespec* pTimestamp, std::int32_t* pValue);

    void setSampleCnt(const timespec &pTimestamp, const std::int32_t &pValue);
    void getSampleCnt(timespec* pTimestamp, std::int32_t* pValue);
    void getSampleCntMax(timespec* pTimestamp, std::int32_t* pValue);
    void getSamplesTotal(timespec* pTimestamp, std::int32_t* pValue);

    //void setTrigLvl(const timespec &pTimestamp, const std::int32_t &pValue);
    //void getTrigLvl(timespec* pTimestamp, std::int32_t* pValue);
    //void setTrigEdge(const timespec &pTimestamp, const std::int32_t &pValue);
    //void getTrigEdge(timespec* pTimestamp, std::int32_t* pValue);
    //void setTrigChan(const timespec &pTimestamp, const std::int32_t &pValue);
    //void getTrigChan(timespec* pTimestamp, std::int32_t* pValue);

    //void getAdqDevVal(timespec* pTimestamp, std::int32_t* pValue);
    //void getAdqDevStrVal(timespec* pTimestamp, std::string* pValue);

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

private:
    ADQInterface * m_adqDevPtr;
    nds::Port m_node;

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
    int32_t m_chanBits;       // Four bits: 0 - channel A; 1 - A and B; 2 - A, B and C; 3 - all channels (ABCD) -- make dropdown in GUI!
    std::string m_chanMask;   // Four variations: 0x01 (ch A), 0x02 (ch AB), 0x04 (ch ABC), 0x08 (all ch)
    bool m_chanMaskChanged;   //
    
    int32_t m_trigMode;
    bool m_trigModeChanged;
    int32_t m_trigLvl;
    bool m_trigLvlChanged;
    int32_t m_trigEdge;
    bool m_trigEdgeChanged;
    int32_t m_trigChan;
    bool m_trigChanChanged;
    int32_t m_trigFreq; // not used yet
    

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
    //nds::PVDelegateIn<std::int32_t> m_chanActivePV; // Device specific
    //nds::PVDelegateIn<std::string> m_chanMaskPV;    //
    nds::PVDelegateIn<std::int32_t> m_trigModePV;
    //nds::PVDelegateIn<std::int32_t> m_trigLvlPV;
    //nds::PVDelegateIn<std::int32_t> m_trigEdgePV;
    //nds::PVDelegateIn<std::int32_t> m_trigChanPV;

    nds::Thread m_daqThread;
    bool m_stopDaq;
};

#endif /* ADQAICHANNELGROUP_H */
