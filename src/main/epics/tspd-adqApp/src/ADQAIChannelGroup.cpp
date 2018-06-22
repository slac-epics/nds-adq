#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <time.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQFourteen.h"
#include "ADQSeven.h"

static struct timespec tsref;
void timerStart(void)
{
    if (clock_gettime(CLOCK_REALTIME, &tsref) < 0) {
        printf("\nFailed to start timer.");
        return;
    }
}
unsigned int timerTimeMs(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        printf("\nFailed to get system time.");
        return -1;
    }
    return (unsigned int)((int)(ts.tv_sec - tsref.tv_sec) * 1000 +
        (int)(ts.tv_nsec - tsref.tv_nsec) / 1000000);
}

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adqDevPtr(adqDev), 
    m_logMsgPV(nds::PVDelegateIn<std::string>("LogMsg", std::bind(&ADQAIChannelGroup::getLogMsg,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_daqModePV(nds::PVDelegateIn<std::int32_t>("DAQMode-RB", std::bind(&ADQAIChannelGroup::getDaqMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    
    m_patternModePV(nds::PVDelegateIn<std::int32_t>("PatternMode-RB", std::bind(&ADQAIChannelGroup::getPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dcBiasPV(nds::PVDelegateIn<std::int32_t>("DCBias-RB", std::bind(&ADQAIChannelGroup::getDcBias,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbsBypassPV(nds::PVDelegateIn<std::int32_t>("DBSBypass-RB", std::bind(&ADQAIChannelGroup::getDbsBypass,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbsDcPV(nds::PVDelegateIn<std::int32_t>("DBSDC-RB", std::bind(&ADQAIChannelGroup::getDbsDc,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbsLowSatPV(nds::PVDelegateIn<std::int32_t>("DBSLowSat-RB", std::bind(&ADQAIChannelGroup::getDbsLowSat,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbsUpSatPV(nds::PVDelegateIn<std::int32_t>("DBSUpSat-RB", std::bind(&ADQAIChannelGroup::getDbsUpSat,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_recordCntPV(nds::PVDelegateIn<std::int32_t>("RecordCnt-RB", std::bind(&ADQAIChannelGroup::getRecordCnt,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_recordCntCollectPV(nds::PVDelegateIn<std::int32_t>("RecordCntCollect-RB", std::bind(&ADQAIChannelGroup::getRecordCntCollect,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleCntPV(nds::PVDelegateIn<std::int32_t>("SampCnt-RB", std::bind(&ADQAIChannelGroup::getSampleCnt,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleCntMaxPV(nds::PVDelegateIn<std::int32_t>("SampCntMax-RB", std::bind(&ADQAIChannelGroup::getSampleCntMax,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleCntTotalPV(nds::PVDelegateIn<std::int32_t>("SampCntTotal-RB", std::bind(&ADQAIChannelGroup::getSamplesTotal,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleSkipPV(nds::PVDelegateIn<std::int32_t>("SampSkip-RB", std::bind(&ADQAIChannelGroup::getSampleSkip,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigModePV(nds::PVDelegateIn<std::int32_t>("TrigMode-RB", std::bind(&ADQAIChannelGroup::getTrigMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigFreqPV(nds::PVDelegateIn<std::int32_t>("TrigFreq-RB", std::bind(&ADQAIChannelGroup::getTrigFreq,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_flushTimePV(nds::PVDelegateIn<std::int32_t>("Timeout-RB", std::bind(&ADQAIChannelGroup::getFlushTime,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_streamTimePV(nds::PVDelegateIn<double>("StreamTime-RB", std::bind(&ADQAIChannelGroup::getStreamTime,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);
    //m_node = parentNode.addChild(nds::Node(name));

    m_chanCnt = m_adqDevPtr->GetNofChannels();

    // Create vector of pointers to each chanel
    for (size_t channelNum(0); channelNum != m_chanCnt; ++channelNum)
    {
        std::ostringstream channelName;
        channelName << "CH" << channelNum;
        m_AIChannelsPtr.push_back(std::make_shared<ADQAIChannel>(channelName.str(), m_node, channelNum));
    }

    // PV for error/warning/info messages
    m_logMsgPV.setScanType(nds::scanType_t::interrupt);
    m_logMsgPV.setMaxElements(512);
    m_node.addChild(m_logMsgPV);

    // PVs for data acquisition modes
    nds::enumerationStrings_t daqModeList = { "Multi-Record", "Continuous streaming", "Triggered streaming", "Raw streaming" };
    nds::PVDelegateOut<std::int32_t> node(nds::PVDelegateOut<std::int32_t>("DAQMode", std::bind(&ADQAIChannelGroup::setDaqMode,
                                                                                                  this,
                                                                                                  std::placeholders::_1,
                                                                                                  std::placeholders::_2),
                                                                                      std::bind(&ADQAIChannelGroup::getDaqMode,
                                                                                                  this,
                                                                                                  std::placeholders::_1,
                                                                                                  std::placeholders::_2)));
    node.setEnumeration(daqModeList);
    m_node.addChild(node);

    m_daqModePV.setScanType(nds::scanType_t::interrupt);
    m_daqModePV.setEnumeration(daqModeList);
    m_node.addChild(m_daqModePV);

    // PVs for Trigger Mode
    nds::enumerationStrings_t trigModeList = { "SW trigger", "External trigger", "Level trigger", "Internal trigger" };

    //// urojec L1: is this reusage of nodes safe? Are you sure that a copy is made in the addChild function,
    //// or are there references?
    //// ppipp: in the NDS examples they create new names for each node
    ////        this reusage was in Niklas' work: I think he made it this way because he has more than 10 PVs to connect to these nodes in ChannelsGroup
    ////        I didn't have problems with this yet but I always can change for sake of safety, it will risen the amount of variables 
    node = nds::PVDelegateOut<std::int32_t>("TrigMode", std::bind(&ADQAIChannelGroup::setTrigMode,
                                                                       this,
                                                                       std::placeholders::_1,
                                                                       std::placeholders::_2),
                                                           std::bind(&ADQAIChannelGroup::getTrigMode,
                                                                       this,
                                                                       std::placeholders::_1,
                                                                       std::placeholders::_2));
    node.setEnumeration(trigModeList);
    m_node.addChild(node);

    m_trigModePV.setScanType(nds::scanType_t::interrupt);
    m_trigModePV.setEnumeration(trigModeList);
    m_node.addChild(m_trigModePV);

    // PVs for Adjustable Bias
    node = nds::PVDelegateOut<std::int32_t>("DCBias",
                                                   std::bind(&ADQAIChannelGroup::setDcBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDcBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2));
    m_node.addChild(node);
    m_dcBiasPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dcBiasPV);

    // PV for DBS setup
    node = nds::PVDelegateOut<std::int32_t>("DBSBypass",
                                                   std::bind(&ADQAIChannelGroup::setDbsBypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsBypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2));
    m_node.addChild(node);
    m_dbsBypassPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbsBypassPV);

    node = nds::PVDelegateOut<std::int32_t>("DBSDC",
                                                   std::bind(&ADQAIChannelGroup::setDbsDc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsDc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2));
    m_node.addChild(node);
    m_dbsDcPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbsDcPV);

    node = nds::PVDelegateOut<std::int32_t>("DBSLowSat",
                                                   std::bind(&ADQAIChannelGroup::setDbsLowSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsLowSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2));
    m_node.addChild(node);
    m_dbsLowSatPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbsLowSatPV);

    node = nds::PVDelegateOut<std::int32_t>("DBSUpSat",
                                                   std::bind(&ADQAIChannelGroup::setDbsUpSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsUpSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2));
    m_node.addChild(node);
    m_dbsUpSatPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbsUpSatPV);

    // PV for Pattern Mode
    nds::enumerationStrings_t patternModeList = { "Normal", "Test (x)", "Count upwards", "Count downwards", "Alternating ups and downs" };
    node = nds::PVDelegateOut<std::int32_t>("PatternMode", std::bind(&ADQAIChannelGroup::setPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                           std::bind(&ADQAIChannelGroup::getPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));
    node.setEnumeration(patternModeList);
    m_node.addChild(node);

    m_patternModePV.setScanType(nds::scanType_t::interrupt);
    m_patternModePV.setEnumeration(patternModeList);
    m_node.addChild(m_patternModePV);

    // PVs for records
    node = nds::PVDelegateOut<std::int32_t>("RecordCnt", std::bind(&ADQAIChannelGroup::setRecordCnt,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getRecordCnt,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_recordCntPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_recordCntPV);

    node = nds::PVDelegateOut<std::int32_t>("RecordCntCollect", std::bind(&ADQAIChannelGroup::setRecordCntCollect,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                              std::bind(&ADQAIChannelGroup::getRecordCntCollect,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_recordCntCollectPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_recordCntCollectPV);

    // PVs for samples
    node = nds::PVDelegateOut<std::int32_t>("SampCnt", std::bind(&ADQAIChannelGroup::setSampleCnt,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getSampleCnt,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_sampleCntPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleCntPV);

    m_sampleCntMaxPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleCntMaxPV);

    m_sampleCntTotalPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleCntTotalPV);

    node = nds::PVDelegateOut<std::int32_t>("SampSkip", std::bind(&ADQAIChannelGroup::setSampleSkip,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getSampleSkip,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_sampleSkipPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleSkipPV);

    // PV for trigger frequence
    node = nds::PVDelegateOut<std::int32_t>("TrigFreq", std::bind(&ADQAIChannelGroup::setTrigFreq,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getTrigFreq,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_trigFreqPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_trigFreqPV);

    // PV for flush timeout
    node = nds::PVDelegateOut<std::int32_t>("Timeout", std::bind(&ADQAIChannelGroup::setFlushTime,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getFlushTime,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_flushTimePV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_flushTimePV);

    // PV for flush timeout
    nds::PVDelegateOut<double> nodeDbl = nds::PVDelegateOut<double>("StreamTime", std::bind(&ADQAIChannelGroup::setStreamTime,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getStreamTime,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(nodeDbl);
    m_streamTimePV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_streamTimePV);

    // PV for state machine
    m_stateMachine = m_node.addChild(nds::StateMachine(true, std::bind(&ADQAIChannelGroup::onSwitchOn, this),
                                                             std::bind(&ADQAIChannelGroup::onSwitchOff, this),
                                                             std::bind(&ADQAIChannelGroup::onStart, this),
                                                             std::bind(&ADQAIChannelGroup::onStop, this),
                                                             std::bind(&ADQAIChannelGroup::recover, this),
                                                             std::bind(&ADQAIChannelGroup::allowChange,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2,
                                                                     std::placeholders::_3)));

    commitChanges();
}

void ADQAIChannelGroup::getLogMsg(timespec* pTimestamp, std::string* pValue)
{
    std::string text;
    *pValue = text;
}

void ADQAIChannelGroup::setDaqMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_daqMode = pValue;
    m_daqModeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDaqMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_daqMode;
}

void ADQAIChannelGroup::setTrigMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigMode = pValue;
    m_trigModeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigMode;
}

void ADQAIChannelGroup::setTrigFreq(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigFreq = pValue;
    m_trigFreqChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigFreq(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigFreq;
}

void ADQAIChannelGroup::setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_patternMode = pValue;
    m_patternModeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getPatternMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_patternMode;
}

void ADQAIChannelGroup::setDcBias(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dcBias = pValue;
    m_dcBiasChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDcBias(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dcBias;
}

void ADQAIChannelGroup::setDbsBypass(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbsBypass = pValue;
    m_dbsBypassChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsBypass(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbsBypass;
}

void ADQAIChannelGroup::setDbsDc(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbsDc = pValue;
    m_dbsDcChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsDc(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbsDc;
}

void ADQAIChannelGroup::setDbsLowSat(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbsLowSat = pValue;
    m_dbsLowSatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsLowSat(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbsLowSat;
}

void ADQAIChannelGroup::setDbsUpSat(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbsUpSat = pValue;
    m_dbsUpSatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsUpSat(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbsUpSat;
}

void ADQAIChannelGroup::setRecordCnt(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_recordCnt = pValue;
    m_recordCntChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getRecordCnt(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_recordCnt;
}

void ADQAIChannelGroup::setRecordCntCollect(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_recordCntCollect = pValue;
    m_recordCntCollectChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getRecordCntCollect(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_recordCntCollect;
}

void ADQAIChannelGroup::setSampleCnt(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_sampleCnt = pValue;
    m_sampleCntChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getSampleCnt(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_sampleCnt;
}

void ADQAIChannelGroup::getSampleCntMax(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_sampleCntMax;
}

void ADQAIChannelGroup::getSamplesTotal(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_sampleCntTotal;
}

void ADQAIChannelGroup::setSampleSkip(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_sampleSkip = pValue;
    m_sampleSkipChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getSampleSkip(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_sampleSkip;
}

void ADQAIChannelGroup::setFlushTime(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_flushTime = pValue;
    m_flushTimeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getFlushTime(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_flushTime;
}

void ADQAIChannelGroup::setStreamTime(const timespec &pTimestamp, const double &pValue)
{
    m_streamTime = pValue;
    m_streamTimeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getStreamTime(timespec* pTimestamp, double* pValue)
{
    *pValue = m_streamTime;
}

void ADQAIChannelGroup::commitChanges(bool calledFromDaqThread)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    unsigned int success;
    std::string adqOption = m_adqDevPtr->GetCardOption();

    /* Allow changes to parameters when device is ON/STOPPING/INITIALISING states.
     * Do not apply changes when device is on acquisition state.
     */
    if (!calledFromDaqThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) 
    {
        return;
    }

    if (m_daqModeChanged)
    {
        m_daqModeChanged = false;
        m_daqModePV.push(now, m_daqMode);
    }

    if (m_trigModeChanged)
    {
        m_trigModeChanged = false;

        success = m_adqDevPtr->SetTriggerMode(m_trigMode + 1);
        ADQNDS_MSG_WARNLOG(success, "WARNING: SetTriggerMode failed.");
        if (success)
        {
            m_trigModePV.push(now, m_trigMode);
            ADQNDS_MSG_INFOLOG("SUCCESS: SetTriggerMode");
        }
    }

    if (m_patternModeChanged)
    {
        m_patternModeChanged = false;
        success = m_adqDevPtr->SetTestPatternMode(m_patternMode);
        ADQNDS_MSG_WARNLOG(success, "WARNING: SetTestPatternMode failed.");
        if (success)
        {
            m_patternModePV.push(now, m_patternMode);
            ADQNDS_MSG_INFOLOG("SUCCESS: SetTestPatternMode");
        }
    }

    if (m_dcBiasChanged)
    {
        m_dcBiasChanged = false;
        success = m_adqDevPtr->HasAdjustableBias();

        if (success)
        {
            unsigned int i = 0;
            for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
            {
                success = m_adqDevPtr->SetAdjustableBias(chan + 1, m_dcBias);
                if (success)
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == m_chanCnt)
            {
                ADQNDS_MSG_INFOLOG("SUCCESS: SetAdjustableBias is set for all channels.");
                m_dcBiasPV.push(now, m_dcBias);
            }
            else
            {
                success = 0;
                ADQNDS_MSG_WARNLOG(success, "WARNING: SetAdjustableBias failed on one or more channels.");
            }
        }
    }

    if (m_dbsBypassChanged || m_dbsDcChanged || m_dbsLowSatChanged || m_dbsUpSatChanged)
    {
        unsigned int dbsInstCnt;

        m_dbsBypassChanged = false;
        m_dbsDcChanged = false;
        m_dbsLowSatChanged = false;
        m_dbsUpSatChanged = false;

        success = m_adqDevPtr->GetNofDBSInstances(&dbsInstCnt);
        if (success)
        {
            unsigned int i = 0;
            for (unsigned char dbsInst = 0; dbsInst < dbsInstCnt; ++dbsInst)
            {
                success = m_adqDevPtr->SetupDBS(dbsInst, m_dbsBypass, m_dbsDc, m_dbsLowSat, m_dbsUpSat);
                if (success)
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == dbsInstCnt)
            {
                ADQNDS_MSG_INFOLOG("SUCCESS: SetupDBS is set for all channels.");
                m_dbsBypassPV.push(now, m_dbsBypass);
                m_dbsDcPV.push(now, m_dbsDc);
                m_dbsLowSatPV.push(now, m_dbsLowSat);
                m_dbsUpSatPV.push(now, m_dbsUpSat);
            }
            else
            {
                success = 0;
                ADQNDS_MSG_WARNLOG(success, "WARNING: SetupDBS failed on one or more channels.");
            }
        }
    }

    if (m_recordCntChanged || m_sampleCntChanged)
    {
        unsigned int sampleCntMax;

        m_recordCntChanged = false;
        m_sampleCntChanged = false;

        if ((m_recordCnt <= -1) && ((m_trigMode == 0) || (m_daqMode == 0)))
        {
            m_recordCnt = 0;
            success = 0;
            ADQNDS_MSG_WARNLOG(success, "WARNING: Inifinite mode is not allowed with this Trigger Mode or DAQ Mode.");
        }

        if (m_recordCnt > 0)
        {
            m_recordCntPV.push(now, m_recordCnt);

            if (m_daqMode == 0) // Multi-Record
            {
                success = m_adqDevPtr->GetMaxNofSamplesFromNofRecords(m_recordCnt, &sampleCntMax);
                ADQNDS_MSG_WARNLOG(success, "WARNING: Couldn't get the MAX number of samples: set number of records.");
                if (success)
                {
                    m_sampleCntMax = sampleCntMax;
                    m_sampleCntMaxPV.push(now, m_sampleCntMax);

                    if (m_sampleCnt > m_sampleCntMax)
                    {
                        success = 0;
                        ADQNDS_MSG_WARNLOG(success, "WARNING: Chosen number of samples was higher than the maximum number of samples.");
                        m_sampleCnt = m_sampleCntMax;
                    }
                }
            }

            m_sampleCntPV.push(now, m_sampleCnt);

            m_sampleCntTotal = m_sampleCnt * m_recordCnt;
            m_sampleCntTotalPV.push(now, m_sampleCntTotal);
        }
        else 
        {
            m_recordCntPV.push(now, m_recordCnt);
            m_sampleCntPV.push(now, m_sampleCnt);
        }
    }

    if (m_recordCntCollectChanged)
    {
        m_recordCntCollectChanged = false;

        if (m_recordCntCollect > m_recordCnt)
        {
            m_recordCntCollect = m_recordCnt;
        }
          
        m_recordCntCollectPV.push(now, m_recordCntCollect);

        m_sampleCntTotal = m_sampleCnt * m_recordCntCollect;
        m_sampleCntTotalPV.push(now, m_sampleCntTotal);
    }
    
    if (m_trigFreqChanged && (m_trigFreq != 0))
    {
        m_trigFreqChanged = false;
        m_trigFreqPV.push(now, m_trigFreq);
        m_trigPeriod = 1000000000 / m_trigFreq;
    }

    if (m_flushTimeChanged)
    {
        m_flushTimeChanged = false;

        if (m_flushTime <= 0)
        {
            m_flushTime = 1000;
        }

        m_flushTimePV.push(now, m_flushTime);
    }

    if (m_sampleSkipChanged && (m_daqMode == 1)) // Continuous streaming
    {
        m_sampleSkipChanged = false;

        if (m_sampleSkip <= 1)
        {
            m_sampleSkip = 2;
            ADQNDS_MSG_INFOLOG("INFO: Sample skip can't be less than 2 -> changed to 2.");
        }

        if (m_sampleSkip > 65536)
        {
            m_sampleSkip = 65536;
            ADQNDS_MSG_INFOLOG("INFO: Sample skip can't be more than 65536 -> changed to 65536.");
        }

        if ((adqOption.find("-2X") != std::string::npos) || (adqOption.find("-1X") != std::string::npos))
        {
            if (m_sampleSkip == 3)
            {
                m_sampleSkip = 4;
                ADQNDS_MSG_INFOLOG("INFO: Sample skip can't be 3 -> changed to 4.");
            }

            if ( m_sampleSkip == 5 || m_sampleSkip == 6 || m_sampleSkip == 7)
            {
                m_sampleSkip = 8;
                ADQNDS_MSG_INFOLOG("INFO: Sample skip can't be 5, 6 or 7 -> changed to 8.");
            }
        }

        if ((adqOption.find("-4C") != std::string::npos) || (adqOption.find("-2C") != std::string::npos))
        {
            if (m_sampleSkip == 3)
            {
                m_sampleSkip = 4;
                ADQNDS_MSG_INFOLOG("INFO: Sample skip can't be 3 -> changed to 4.");
            }
        }

        m_sampleSkipPV.push(now, m_sampleSkip);
    }

    if (m_streamTimeChanged && (m_daqMode == 1))
    {
        m_streamTimeChanged = false;

        if (m_streamTime <= 0)
        {
            m_streamTime = 1.0;
            ADQNDS_MSG_INFOLOG("INFO: Streaming time can't be 0 seconds -> changed to 1.0 seconds.");
        }

        m_streamTimePV.push(now, m_streamTime);
    }
}

void ADQAIChannelGroup::onSwitchOn()
{
    // Enable all channels
    for (auto const& channel : m_AIChannelsPtr)
    {
        channel->setState(nds::state_t::on);
    }
    
    commitChanges(true);
}

void ADQAIChannelGroup::onSwitchOff()
{
    // Disable all channels
    for (auto const& channel : m_AIChannelsPtr)
    {
        channel->setState(nds::state_t::off);
    }
    
    commitChanges();
}

void ADQAIChannelGroup::onStart()
{
    // Start all Channels
    for (auto const& channel : m_AIChannelsPtr)
    {
        channel->setState(nds::state_t::running);
    }

    // Start data acquisition
    m_stopDaq = false;

    if (m_daqMode == 0)
        m_daqThread = m_node.runInThread("DAQ-MultiRecord", std::bind(&ADQAIChannelGroup::daqMultiRecord, this));
    if (m_daqMode == 1)
        m_daqThread = m_node.runInThread("DAQ-ContinStream", std::bind(&ADQAIChannelGroup::daqContinStream, this));
    if (m_daqMode == 2)
        m_daqThread = m_node.runInThread("DAQ-TrigStream", std::bind(&ADQAIChannelGroup::daqTrigStream, this));
    //if (m_daqMode == 3)
        
}

void ADQAIChannelGroup::onStop()
{
    m_stopDaq = true;

    m_daqThread.join();

    // Stop channels
    for (auto const& channel : m_AIChannelsPtr)
    {
        channel->setState(nds::state_t::on);
    }

    commitChanges();
}

void ADQAIChannelGroup::recover()
{
    throw nds::StateMachineRollBack("INFO: Cannot recover");
}

bool ADQAIChannelGroup::allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal)
{
    return true;
}

void ADQAIChannelGroup::daqTrigStream()
{
    int success;
    unsigned int bufferSize;
    streamingHeader_t* tr_headers[CHANNEL_NUMBER_MAX];
    short* tr_buffers[CHANNEL_NUMBER_MAX];
    short* tr_extra[CHANNEL_NUMBER_MAX];
    short* recordData;
    unsigned int preTrigSampleCnt = 0;
    unsigned int holdOffSampleCnt = 0;
    unsigned int buffersFilled;
    unsigned int recordsDone[CHANNEL_NUMBER_MAX];
    unsigned int samplesAdded[CHANNEL_NUMBER_MAX];
    unsigned int headersAdded[CHANNEL_NUMBER_MAX];
    unsigned int samplesExtraData[CHANNEL_NUMBER_MAX];
    unsigned int headerStatus[CHANNEL_NUMBER_MAX];
    unsigned int headersDone = 0;
    unsigned int gotRecordsTotal = 0;
    unsigned int samplesRemain;
    unsigned int gotRecordsTotalSum = 0;
    unsigned int recordsSum = 0;

    int adqType = m_adqDevPtr->GetADQType();
    if (adqType == 714 || adqType == 14)
    {
        bufferSize = 512 * 1024;
    }
    if (adqType == 7)
    {
        bufferSize = 256 * 1024;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        tr_headers[chan] = NULL;
        tr_buffers[chan] = NULL;
        tr_extra[chan] = NULL;

        recordsDone[chan] = 0;
        samplesAdded[chan] = 0;
        headersAdded[chan] = 0;
        samplesExtraData[chan] = 0;
        headerStatus[chan] = 0;
    }

    // Convert chanMask from std::string to unsigned char for certain ADQAPI functions
    unsigned char chanMaskChar = *m_chanMask.c_str();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (!((1 << chan) & chanMaskChar))
            continue;

        tr_buffers[chan] = (short int*)malloc((size_t)bufferSize);
        if (!tr_buffers[chan])
        {
            success = 0;
            ADQNDS_MSG_ERRLOG(success, "ERROR: Failed to allocate memory for target buffers.");
        }

        tr_headers[chan] = (streamingHeader_t*)malloc((size_t)bufferSize);
        if (!tr_headers[chan])
        {
            success = 0;
            ADQNDS_MSG_ERRLOG(success, "ERROR: Failed to allocate memory for target headers.");
        }

        tr_extra[chan] = (short int*)malloc((size_t)(sizeof(short)*m_sampleCnt));
        if (!tr_extra[chan])
        {
            success = 0;
            ADQNDS_MSG_ERRLOG(success, "ERROR: Failed to allocate memory for target extradata.");
        }
    }

    if (adqType == 714 || adqType == 14) // Only ADQ14 function
    {
        // Allocate memory for record data (used for ProcessRecord function template)
        recordData = (short int*)malloc((size_t)(sizeof(short)*m_sampleCnt));

        if (!recordData)
        {
            success = 0;
            ADQNDS_MSG_ERRLOG(success, "ERROR: Failed to allocate memory for ProcessRecord method.");
        }
    }

    switch (m_trigMode)
    {
    case 0: // SW trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing Software trigger... ");
        break;
    case 1: // External trigger  ----- need to investigate the API doc to develop this method
        ADQNDS_MSG_INFOLOG("INFO: Issuing External trigger... ");
        break;
    case 2: // Level trigger ----- need to investigate the API doc to develop this method
        ADQNDS_MSG_INFOLOG("INFO: Issuing Level trigger... ");
       
        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigEdge failed.");
        
        success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigLevel failed.");

        success = m_adqDevPtr->SetLvlTrigChannel(m_trigChanInt);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigChannel failed.");
        break;
    case 3: // Internal trigger ----- need to investigate the API doc to develop this method
        ADQNDS_MSG_INFOLOG("INFO: Issuing Internal trigger... ");
        success = m_adqDevPtr->SetInternalTriggerPeriod(m_trigFreq); 
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetInternalTriggerPeriod failed.");
        break;
    }

    success = m_adqDevPtr->TriggeredStreamingSetup(m_recordCnt, m_sampleCnt, preTrigSampleCnt, holdOffSampleCnt, chanMaskChar);
    ADQNDS_MSG_ERRLOG(success, "ERROR: TriggeredStreamingSetup failed.");

    success = m_adqDevPtr->SetTransferBuffers(CHANNEL_NUMBER_MAX, bufferSize);
    ADQNDS_MSG_ERRLOG(success, "ERROR: SetTransferBuffers failed.");

    success = m_adqDevPtr->StopStreaming();
    ADQNDS_MSG_ERRLOG(success, "ERROR: StopStreaming failed.");

    success = m_adqDevPtr->StartStreaming();
    ADQNDS_MSG_ERRLOG(success, "ERROR: StartStreaming failed.");

    if (m_trigMode == 0)
    {
        success = m_adqDevPtr->DisarmTrigger();
        ADQNDS_MSG_ERRLOG(success, "ERROR: DisarmTrigger failed.");

        success = m_adqDevPtr->ArmTrigger();
        ADQNDS_MSG_ERRLOG(success, "ERROR: ArmTrigger failed.");

        for (int i = 0; i < m_recordCnt; ++i)
        {
            success = m_adqDevPtr->SWTrig();
            ADQNDS_MSG_ERRLOG(success, "ERROR: SWTrig failed.");
        }
    }
   
    do
    {
        buffersFilled = 0;

        success = m_adqDevPtr->GetStreamOverflow();
        if (success)
        {
            ndsErrorStream(m_node) << "ERROR: Streaming overflow detected." << std::endl;
            goto finish;
        }

        success = m_adqDevPtr->GetTransferBufferStatus(&buffersFilled);
        ADQNDS_MSG_ERRLOG(success, "ERROR: GetTransferBufferStatus failed.");

        // Poll for the transfer buffer status as long as the timeout has not been
        // reached and no buffers have been filled.
        while (!buffersFilled)
        {
            // Mark the loop start
            timerStart();
            while (!buffersFilled && (timerTimeMs() < m_flushTime))
            {
                success = m_adqDevPtr->GetTransferBufferStatus(&buffersFilled);
                ADQNDS_MSG_ERRLOG(success, "ERROR: GetTransferBufferStatus failed.");
                // Sleep to avoid loading the processor too much
                sleep(10);
            }

            // Timeout reached, flush the transfer buffer to receive data
            if (!buffersFilled)
            {
                ADQNDS_MSG_INFOLOG("INFO: Timeout, flushing DMA...");
                success = m_adqDevPtr->FlushDMA();
                ADQNDS_MSG_ERRLOG(success, "ERROR: FlushDMA failed.");
            }
        }

        ADQNDS_MSG_INFOLOG("INFO: Receiving data...");
        success = m_adqDevPtr->GetDataStreaming((void**)tr_buffers, (void**)tr_headers, chanMaskChar, samplesAdded, headersAdded, headerStatus);
        ADQNDS_MSG_ERRLOG(success, "ERROR: GetDataStreaming failed.");

        // Parse data
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            if (!((1 << chan) & chanMaskChar))
                continue;

            if (headersAdded[chan] > 0)
            {
                if (headerStatus[chan])
                {
                    headersDone = headersAdded[chan];
                }
                else
                {
                    // One incomplete record in the buffer (header is copied to the front
                    // of the buffer later)
                    headersDone = headersAdded[chan] - 1;
                }

                // If there is at least one complete header
                recordsDone[chan] += headersDone;
            }

            // Parse  added samples
            if (samplesAdded[chan] > 0)
            {
                samplesRemain = samplesAdded[chan];

                // Handle incomplete record at the start of the buffer
                if (samplesExtraData[chan] > 0)
                {
                    if (headersDone == 0)
                    {
                        // There is not enough data in the transfer buffer to complete
                        // the record. Add all the samples to the extradata buffer.
                        std::memcpy(&tr_extra[chan][samplesExtraData[chan]], tr_buffers[chan], sizeof(short)*samplesAdded[chan]);
                        samplesRemain -= samplesAdded[chan];
                        samplesExtraData[chan] += samplesAdded[chan];
                    }
                    else
                    {
                        if (adqType == 714 || adqType == 14)
                        {
                            // Move data to record_data
                            std::memcpy((void*)recordData, tr_extra[chan], sizeof(short)*samplesExtraData[chan]);
                            std::memcpy((void*)(recordData + samplesExtraData[chan]), tr_buffers[chan], sizeof(short)*(tr_headers[chan][0].recordLength - samplesExtraData[chan]));
                            daqTrigStreamProcessRecord(recordData, &tr_headers[chan][0]);
                        }

                        samplesRemain -= tr_headers[chan][0].recordLength - samplesExtraData[chan];
                        samplesExtraData[chan] = 0;
                    }

                    m_AIChannelsPtr[chan]->readDAQ(tr_buffers[chan], m_sampleCntTotal);
                }
                else
                {
                    if (headersDone == 0)
                    {
                        // The samples in the transfer buffer begin a new record, this
                        // record is incomplete.
                        std::memcpy(tr_extra[chan], tr_buffers[chan], sizeof(short)*samplesAdded[chan]);
                        samplesRemain -= samplesAdded[chan];
                        samplesExtraData[chan] = samplesAdded[chan];
                    }
                    else
                    {

                        // Copy data to record buffer
                        if (adqType == 714 || adqType == 14)
                        {
                            std::memcpy((void*)recordData, tr_buffers[chan], sizeof(short)*tr_headers[chan][0].recordLength);
                            daqTrigStreamProcessRecord(recordData, &tr_headers[chan][0]);
                        }
                        
                        samplesRemain -= tr_headers[chan][0].recordLength;
                    }

                    //m_AIChannelsPtr[chan]->readTrigStream(tr_buffers[chan], m_sampleCntTotal);
                }
                // At this point: the first record in the transfer buffer or the entire
                // transfer buffer has been parsed.

                // Loop through complete records fully inside the buffer
                for (unsigned int i = 1; i < headersDone; ++i)
                {
                    // Copy data to record buffer
                    if (adqType == 714 || adqType == 14)
                    {
                        std::memcpy((void*)recordData, (&tr_buffers[chan][samplesAdded[chan] - samplesRemain]), sizeof(short)*tr_headers[chan][i].recordLength);
                        daqTrigStreamProcessRecord(recordData, &tr_headers[chan][i]);
                    }
                    
                    samplesRemain -= tr_headers[chan][i].recordLength;

                    //m_AIChannelsPtr[chan]->readTrigStream(tr_buffers[chan], m_sampleCntTotal);
                }

                if (samplesRemain > 0)
                {
                    // There is an incomplete record at the end of the transfer buffer
                    // Copy the incomplete header to the start of the target_headers buffer
                    std::memcpy(tr_headers[chan], &tr_headers[chan][headersDone], sizeof(streamingHeader_t));

                    // Copy any remaining samples to the target_buffers_extradata buffer,
                    // they belong to the incomplete record
                    std::memcpy(tr_extra[chan], &tr_buffers[chan][samplesAdded[chan] - samplesRemain], sizeof(short)*samplesRemain);
                    // printf("Incomplete at end of transfer buffer. %u samples.\n", samples_remaining);
                    // printf("Copying %u samples to the extradata buffer\n", samples_remaining);
                    samplesExtraData[chan] = samplesRemain;
                    samplesRemain = 0;
                }
            }

            // Read buffers from each channel and send them to DATA PVs
            m_AIChannelsPtr[chan]->readDAQ(tr_buffers[chan], m_sampleCntTotal);
        }

        // Update received_all_records
        gotRecordsTotalSum = 0;
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            gotRecordsTotalSum += recordsDone[chan];
        }

        // Determine if collection is completed
        gotRecordsTotal = (gotRecordsTotalSum >= recordsSum);
    } while (!gotRecordsTotal);

finish:
    ADQNDS_MSG_INFOLOG("INFO: Acquisition finished.");
    commitChanges(true);
    m_adqDevPtr->StopStreaming();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan) 
    {
        if (tr_buffers[chan])
            free(tr_buffers[chan]);
        if (tr_headers[chan])
            free(tr_headers[chan]);
        if (tr_extra[chan])
            free(tr_extra[chan]);
    }

    try 
    {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) 
    {
        /* We are probably already in "stopping", no need to panic... */
    }
}

void ADQAIChannelGroup::daqTrigStreamProcessRecord(short* recordData, streamingHeader_t* recordHeader)  // Need to create PV for average samples, probably it is important, since they have this method
{
#ifdef PRINT_RECORD_INFO
    // You may process a full record here (will be called once with every completed record)
    // Calculate average over all samples (as an example of processing a record)
    int64_t recordSampleSum;
    double  recordSampleAverage;

    recordSampleSum = 0;
    for (unsigned int k = 0; k < recordHeader->recordLength; k++)
    {
        recordSampleSum += recordData[k];
    } 
 
    recordSampleAverage = (double)recordSampleSum / (double)recordHeader->recordLength;

    // Print record info
    ndsInfoStream(m_node) << "--------------------------------------" << std::endl;
    ndsInfoStream(m_node) << " Device (SPD-" << (int)recordHeader->serialNumber  << "), Channel " << (int)recordHeader->chan << ", Record " << recordHeader->recordNumber << std::endl;
    ndsInfoStream(m_node) << " Samples         = " << recordHeader->recordLength << std::endl;
    ndsInfoStream(m_node) << " Status          = " << (int)recordHeader->recordStatus << std::endl;
    ndsInfoStream(m_node) << " Timestamp       = " << recordHeader->timeStamp << std::endl;
    ndsInfoStream(m_node) << " Average samples = " << recordSampleAverage << std::endl;
    ndsInfoStream(m_node) << "--------------------------------------" << std::endl;
#endif
}

void ADQAIChannelGroup::daqMultiRecord()  // Need to mention on GUI about triggering the device when ot SW trigger is used
{
    int trigged, success;
    short* buffers[CHANNEL_NUMBER_MAX];
    void* tr_buffers[CHANNEL_NUMBER_MAX];

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        buffers[chan] = NULL;
    }

    // Convert chanMask from std::string to unsigned char for GetData() function
    unsigned char chanMaskChar = *m_chanMask.c_str();

    switch (m_trigMode)
    {
    case 0: // SW trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing Software trigger...");
        break;
    case 1: // External trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing External trigger...");
        break;
    case 2: // Level trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing Level trigger...");

        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigEdge failed.");

        success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigLevel failed.");

        success = m_adqDevPtr->SetLvlTrigChannel(m_trigChanInt);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigChannel failed.");
        break;
    case 3: // Internal trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing Internal trigger...");
        success = m_adqDevPtr->SetInternalTriggerPeriod(m_trigPeriod);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetInternalTriggerPeriod failed.");
        break;
    }

    success = m_adqDevPtr->MultiRecordSetup(m_recordCnt, m_sampleCnt);
    ADQNDS_MSG_ERRLOG(success, "ERROR: MultiRecordSetup failed.");

    success = m_adqDevPtr->DisarmTrigger();
    ADQNDS_MSG_ERRLOG(success, "ERROR: DisarmTrigger failed.");

    success = m_adqDevPtr->ArmTrigger();
    ADQNDS_MSG_ERRLOG(success, "ERROR: ArmTrigger failed.");

    if (m_trigMode == 0) // SW trigger
    {
        do
        {
            trigged = m_adqDevPtr->GetAcquiredAll();
            for (int i = 0; i < m_recordCnt; ++i)
            {
                success = m_adqDevPtr->SWTrig();
                ADQNDS_MSG_ERRLOG(success, "ERROR: SWTrig failed.");
            }
        } while (trigged == 0);

        unsigned int acqRecTotal = m_adqDevPtr->GetAcquiredRecords();
        ndsInfoStream(m_node) << "INFO: GetAcquiredRecords: " << acqRecTotal << std::endl;
    }
    else
    {
        ADQNDS_MSG_INFOLOG("INFO: Trigger the device to collect data.");
        do
        {
            trigged = m_adqDevPtr->GetAcquiredAll();
        } while (trigged == 0);

        unsigned int acqRecTotal = m_adqDevPtr->GetAcquiredRecords();
        ndsInfoStream(m_node) << "INFO: GetAcquiredRecords: " << acqRecTotal << std::endl;
    }

    success = m_adqDevPtr->GetStreamOverflow();
    if (success)
    {
        success = 0;
        ADQNDS_MSG_ERRLOG(success, "ERROR: GetStreamOverflow detected.");
    }

    // Here sampleCntTotal should be calculated as (recordCntCollect * sampleCnt), what is taken care of in commitChanges method (recordCntCollectchanged)
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        buffers[chan] = (short*)calloc(m_sampleCntTotal, sizeof(short));
    }

    // Create a pointer array containing the data buffer pointers
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        tr_buffers[chan] = (void*)buffers[chan];
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (buffers[chan] == NULL)
        {
            success = 0;
            ADQNDS_MSG_ERRLOG(success, "ERROR: Failed to allocate memory for target buffers.");
        }
    }

    success = m_adqDevPtr->GetData(tr_buffers, m_sampleCntTotal, sizeof(short), 0, m_recordCntCollect, chanMaskChar, 0, m_sampleCnt, ADQ_TRANSFER_MODE_NORMAL);
    ADQNDS_MSG_ERRLOG(success, "ERROR: GetData failed.");

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        // Read buffers by each channel and send them to DATA PVs
        m_AIChannelsPtr[chan]->readDAQ((short*)tr_buffers[chan], m_sampleCntTotal);
    }

    success = m_adqDevPtr->DisarmTrigger();
    ADQNDS_MSG_ERRLOG(success, "ERROR: DisarmTrigger failed.");


finish:
    ADQNDS_MSG_INFOLOG("INFO: Acquisition finished.");
    commitChanges(true);
    m_adqDevPtr->MultiRecordClose();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (buffers[chan] != NULL)
            free(buffers[chan]);
    }

    try {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) {
        /* We are probably already in "stopping", no need to panic... */
    }
}

void ADQAIChannelGroup::daqContinStream()
{
    int success;
    unsigned int bufferSize, buffersFilled;
    time_t start_time, curr_time;
    double elapsedSeconds;
    short* tr_buffers[CHANNEL_NUMBER_MAX];
    unsigned char* tr_headers[CHANNEL_NUMBER_MAX];
    int bufferStatusLoops;
    unsigned int samplesAdded[CHANNEL_NUMBER_MAX];
    unsigned int headersAdded[CHANNEL_NUMBER_MAX];
    unsigned int headerStatus[CHANNEL_NUMBER_MAX];
    unsigned int samplesAddedTotal;
    uint64_t bytesParsedTotal;
    unsigned int streamCompleted = 0;

    int adqType = m_adqDevPtr->GetADQType();
    if (adqType == 714 || adqType == 14)
    {
        bufferSize = 512 * 1024;
    }
    if (adqType == 7)
    {
        bufferSize = 256 * 1024;
    }

    // Convert chanMask from std::string to unsigned char for SetupContinuousStreaming() function
    unsigned char chanMaskChar = *m_chanMask.c_str();

    for (unsigned int chan = 0; chan < m_chanCnt; chan++)
    {
        tr_buffers[chan] = NULL;
        tr_headers[chan] = NULL;
        if (chanMaskChar & (1 << chan)) 
        {
            tr_buffers[chan] = (short*)malloc(bufferSize * sizeof(char));
            tr_headers[chan] = (unsigned char*)malloc(10 * sizeof(uint32_t));
        }
    }

    success = m_adqDevPtr->StopStreaming();
    ADQNDS_MSG_ERRLOG(success, "ERROR: StopStreaming failed.");

    success = m_adqDevPtr->ContinuousStreamingSetup(chanMaskChar);
    ADQNDS_MSG_WARNLOG(success, "WARNING: ContinuousStreamingSetup failed.");

    success = m_adqDevPtr->SetSampleSkip(m_sampleSkip);
    ADQNDS_MSG_WARNLOG(success, "WARNING: ContinuousStreamingSetup failed.");

    // Start streaming
    samplesAddedTotal = 0;
    bytesParsedTotal = 0;

    success = m_adqDevPtr->StartStreaming();
    ADQNDS_MSG_ERRLOG(success, "ERROR: StartStreaming failed.");

    time(&start_time);

    switch (m_trigMode)
    {
    case 0: // SW trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing Software trigger...");
        break;
    case 1: // External trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing External trigger...");
        break;
    case 2: // Level trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing Level trigger...");

        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigEdge failed.");

        success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigLevel failed.");

        success = m_adqDevPtr->SetLvlTrigChannel(m_trigChanInt);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigChannel failed.");
        break;
    case 3: // Internal trigger
        ADQNDS_MSG_INFOLOG("INFO: Issuing Internal trigger...");
        success = m_adqDevPtr->SetInternalTriggerPeriod(m_trigPeriod);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetInternalTriggerPeriod failed.");
        break;
    }

    if (m_trigMode == 0)
    {
        success = m_adqDevPtr->DisarmTrigger();
        ADQNDS_MSG_ERRLOG(success, "ERROR: DisarmTrigger failed.");

        success = m_adqDevPtr->ArmTrigger();
        ADQNDS_MSG_ERRLOG(success, "ERROR: ArmTrigger failed.");

        for (unsigned int i = 0; i < m_recordCnt; ++i)
        {
            success = m_adqDevPtr->SWTrig();
            ADQNDS_MSG_ERRLOG(success, "ERROR: SWTrig failed.");
        }
    }
    else
    {
        success = m_adqDevPtr->DisarmTrigger();
        ADQNDS_MSG_ERRLOG(success, "ERROR: DisarmTrigger failed.");

        success = m_adqDevPtr->ArmTrigger();
        ADQNDS_MSG_ERRLOG(success, "ERROR: ArmTrigger failed.");
    }

    time(&curr_time);
    while (!streamCompleted)
    {
        bufferStatusLoops = 0;
        buffersFilled = 0;

        while (buffersFilled == 0 && success)
        {
            success = m_adqDevPtr->GetTransferBufferStatus(&buffersFilled);
            ADQNDS_MSG_ERRLOG(success, "ERROR: GetTransferBufferStatus failed.");

            if (buffersFilled == 0)
            {
                sleep(10);
                bufferStatusLoops++;

                if (bufferStatusLoops > 2000)
                {
                    // If the DMA transfer is taking too long, we should flush the DMA buffer before it times out. The timeout defaults to 60 seconds.
                    ADQNDS_MSG_INFOLOG("INFO: Timeout, flushing DMA...");
                    success = m_adqDevPtr->FlushDMA();
                    ADQNDS_MSG_ERRLOG(success, "ERROR: FlushDMA failed.");
                }
            }
        }

        for (unsigned int buf = 0; buf < buffersFilled; buf++)
        {
            ADQNDS_MSG_INFOLOG("INFO: Receiving data...");
            success = m_adqDevPtr->GetDataStreaming((void**)tr_buffers, (void**)tr_headers, chanMaskChar, samplesAdded, headersAdded, headerStatus);
            ADQNDS_MSG_ERRLOG(success, "ERROR: GetDataStreaming failed.");

            for (unsigned int chan = 0; chan < m_chanCnt; ++chan) {
                samplesAddedTotal += samplesAdded[chan];
            }
            
            bytesParsedTotal += (uint64_t)bufferSize;

            // Here, process data. (perform calculations, write to file, let another process operate on the buffers, etc). 
            // This will likely be the bottleneck for the transfer rate.
            for (unsigned int chan = 0; chan < m_chanCnt; ++chan) {
                if (!(chanMaskChar & (1 << chan)))
                    continue;
                m_AIChannelsPtr[chan]->readDAQ(tr_buffers[chan], samplesAddedTotal);
            }
        }

        time(&curr_time);
        elapsedSeconds = difftime(curr_time, start_time);

        if (elapsedSeconds > m_streamTime)
        {
            streamCompleted = 1;
            ADQNDS_MSG_INFOLOG("INFO: Acquisition finished due to achieved target stream time.");
        }

        success = m_adqDevPtr->GetStreamOverflow();
        if (success)
        {
            streamCompleted = 1;
            ADQNDS_MSG_INFOLOG("ERROR: GetStreamOverflow detected.");
        }
    }

finish:
    ADQNDS_MSG_INFOLOG("INFO: Acquisition finished.");
    commitChanges(true);
    m_adqDevPtr->StopStreaming();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan) 
    {
        if (tr_buffers[chan])
            free(tr_buffers[chan]);
        if (tr_headers[chan])
            free(tr_headers[chan]);
    }

    try 
    {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) 
    {
        /* We are probably already in "stopping", no need to panic... */
    }
}

ADQAIChannelGroup::~ADQAIChannelGroup()
{
    /* Destructor
    */
}