#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <time.h>
#include <typeinfo>
#include <unistd.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"
#include "ADQInfo.h"

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface*& adqInterface) :
    m_node(nds::Port(name + GROUP_CHAN_DEVICE, nds::nodeType_t::generic)), 
    m_adqInterface(adqInterface),
    m_logMsgPV(createPvRb<std::string>("LogMsg", &ADQAIChannelGroup::getLogMsg)),
    m_daqModePV(createPvRb<int32_t>("DAQMode-RB", &ADQAIChannelGroup::getDaqMode)),
    m_patternModePV(createPvRb<int32_t>("PatternMode-RB", &ADQAIChannelGroup::getPatternMode)),
    m_chanActivePV(createPvRb<int32_t>("ChanActive-RB", &ADQAIChannelGroup::getChanActive)),
    m_chanMaskPV(createPvRb<int32_t>("ChanMask-RB", &ADQAIChannelGroup::getChanMask)),
    m_dcBiasPV(createPvRb<int32_t>("DCBias-RB", &ADQAIChannelGroup::getDcBias)),
    m_dbsBypassPV(createPvRb<int32_t>("DBSBypass-RB", &ADQAIChannelGroup::getDbsBypass)),
    m_dbsDcPV(createPvRb<int32_t>("DBSDC-RB", &ADQAIChannelGroup::getDbsDc)),
    m_dbsLowSatPV(createPvRb<int32_t>("DBSLowSat-RB", &ADQAIChannelGroup::getDbsLowSat)),
    m_dbsUpSatPV(createPvRb<int32_t>("DBSUpSat-RB", &ADQAIChannelGroup::getDbsUpSat)),
    m_recordCntPV(createPvRb<int32_t>("RecordCnt-RB", &ADQAIChannelGroup::getRecordCnt)),
    m_recordCntCollectPV(createPvRb<int32_t>("RecordCntCollect-RB", &ADQAIChannelGroup::getRecordCntCollect)),
    m_sampleCntPV(createPvRb<int32_t>("SampCnt-RB", &ADQAIChannelGroup::getSampleCnt)),
    m_sampleCntMaxPV(createPvRb<int32_t>("SampCntMax-RB", &ADQAIChannelGroup::getSampleCntMax)),
    m_sampleCntTotalPV(createPvRb<int32_t>("SampCntTotal-RB", &ADQAIChannelGroup::getSamplesTotal)),
    m_sampleSkipPV(createPvRb<int32_t>("SampSkip-RB", &ADQAIChannelGroup::getSampleSkip)),
    m_trigModePV(createPvRb<int32_t>("TrigMode-RB", &ADQAIChannelGroup::getTrigMode)),
    m_trigFreqPV(createPvRb<int32_t>("TrigFreq-RB", &ADQAIChannelGroup::getTrigFreq)),
    m_trigLvlPV(createPvRb<int32_t>("TrigLevel-RB", &ADQAIChannelGroup::getTrigLvl)),
    m_trigEdgePV(createPvRb<int32_t>("TrigEdge-RB", &ADQAIChannelGroup::getTrigEdge)),
    m_trigChanPV(createPvRb<int32_t>("TrigChan-RB", &ADQAIChannelGroup::getTrigChan)),
    m_timeoutPV(createPvRb<int32_t>("Timeout-RB", &ADQAIChannelGroup::getTimeout)),
    m_streamTimePV(createPvRb<double>("StreamTime-RB", &ADQAIChannelGroup::getStreamTime))
{
    parentNode.addChild(m_node);

    m_chanCnt = m_adqInterface->GetNofChannels();
    m_adqType = m_adqInterface->GetADQType();

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
    nds::enumerationStrings_t daqModeList = { "Multi-Record", "Continuous stream", "Triggered stream", "Raw stream" };
    createPvEnum<int32_t>("DAQMode", m_daqModePV, daqModeList, &ADQAIChannelGroup::setDaqMode, &ADQAIChannelGroup::getDaqMode);

    // PV for Pattern Mode
    nds::enumerationStrings_t patternModeList = { "Normal", "Test (x)", "Count upwards", "Count downwards", "Alter ups & downs" };
    createPvEnum<int32_t>("PatternMode", m_patternModePV, patternModeList, &ADQAIChannelGroup::setPatternMode, &ADQAIChannelGroup::getPatternMode);

    // PVs for setting active channels and channel mask
    nds::enumerationStrings_t chanMaskList;
    if (m_chanCnt == 4)
    {
        chanMaskList = { "A", "B", "C", "D", "A+B", "C+D", "A+B+C+D" };
    }
    if (m_chanCnt == 2)
    {
        chanMaskList = { "A", "B", "A+B" };
    }
    if (m_chanCnt == 1)
    {
        chanMaskList = { "A" };
    }
    createPvEnum<int32_t>("ChanActive", m_chanActivePV, chanMaskList, &ADQAIChannelGroup::setChanActive, &ADQAIChannelGroup::getChanActive);

    m_chanMaskPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_chanMaskPV);

    // PVs for Adjustable Bias
    createPv<int32_t>("DCBias", m_dcBiasPV, &ADQAIChannelGroup::setDcBias, &ADQAIChannelGroup::getDcBias);

    // PV for DBS setup
    createPv<int32_t>("DBSBypass", m_dbsBypassPV, &ADQAIChannelGroup::setDbsBypass, &ADQAIChannelGroup::getDbsBypass);
    createPv<int32_t>("DBSDC", m_dbsDcPV, &ADQAIChannelGroup::setDbsDc, &ADQAIChannelGroup::getDbsDc);
    createPv<int32_t>("DBSLowSat", m_dbsLowSatPV, &ADQAIChannelGroup::setDbsLowSat, &ADQAIChannelGroup::getDbsLowSat);
    createPv<int32_t>("DBSUpSat", m_dbsUpSatPV, &ADQAIChannelGroup::setDbsUpSat, &ADQAIChannelGroup::getDbsUpSat);

    // PVs for records
    createPv<int32_t>("RecordCnt", m_recordCntPV, &ADQAIChannelGroup::setRecordCnt, &ADQAIChannelGroup::getRecordCnt);
    createPv<int32_t>("RecordCntCollect", m_recordCntCollectPV, &ADQAIChannelGroup::setRecordCntCollect, &ADQAIChannelGroup::getRecordCntCollect);

    // PVs for samples
    createPv<int32_t>("SampCnt", m_sampleCntPV, &ADQAIChannelGroup::setSampleCnt, &ADQAIChannelGroup::getSampleCnt);

    m_sampleCntMaxPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleCntMaxPV);

    m_sampleCntTotalPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleCntTotalPV);

    createPv<int32_t>("SampSkip", m_sampleSkipPV, &ADQAIChannelGroup::setSampleSkip, &ADQAIChannelGroup::getSampleSkip);

    // PVs for Trigger Mode
    nds::enumerationStrings_t trigModeList = { "SW trigger", "External trigger", "Level trigger", "Internal trigger" };
    createPvEnum<int32_t>("TrigMode", m_trigModePV, trigModeList, &ADQAIChannelGroup::setTrigMode, &ADQAIChannelGroup::getTrigMode);

    // PV for trigger frequence
    createPv<int32_t>("TrigFreq", m_trigFreqPV, &ADQAIChannelGroup::setTrigFreq, &ADQAIChannelGroup::getTrigFreq);

    //PVs for trigger level
    createPv<int32_t>("TrigLevel", m_trigLvlPV, &ADQAIChannelGroup::setTrigLvl, &ADQAIChannelGroup::getTrigLvl);

    // PVs for trigger edge
    nds::enumerationStrings_t triggerEdgeList = { "Falling edge", "Rising edge" };
    createPvEnum<int32_t>("TrigEdge", m_trigEdgePV, triggerEdgeList, &ADQAIChannelGroup::setTrigEdge, &ADQAIChannelGroup::getTrigEdge);

    // PVs for trigger channel
    nds::enumerationStrings_t trigChanList = { "None", "A", "B", "C", "D", "A+B", "C+D", "A+B+C+D" };
    createPvEnum<int32_t>("TrigChan", m_trigChanPV, trigChanList, &ADQAIChannelGroup::setTrigChan, &ADQAIChannelGroup::getTrigChan);

    // PV for flush timeout
    createPv<int32_t>("Timeout", m_timeoutPV, &ADQAIChannelGroup::setTimeout, &ADQAIChannelGroup::getTimeout);

    // PV for streaming time
    createPv<double>("StreamTime", m_streamTimePV, &ADQAIChannelGroup::setStreamTime, &ADQAIChannelGroup::getStreamTime);

    // PV for state machine
    m_stateMachine = m_node.addChild(nds::StateMachine(true,
                                                       std::bind(&ADQAIChannelGroup::onSwitchOn, this),
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

/* This function creates the most common type of PV and sets it readback PV to interrupt mode
 */
template <typename T>
void ADQAIChannelGroup::createPv(const std::string& name, nds::PVDelegateIn<T>& pvRb,
                                 std::function<void(ADQAIChannelGroup*, const timespec&, const T&)> setter,
                                 std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter)
{
    nds::PVDelegateOut<T> node(name,
                               std::bind(setter, this, std::placeholders::_1, std::placeholders::_2),
                               std::bind(getter, this, std::placeholders::_1, std::placeholders::_2));
    m_node.addChild(node);
    pvRb.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(pvRb);
}

/* This function creates the Enumeration type of PV and sets it readback PV to interrupt mode
 */
template <typename T>
void ADQAIChannelGroup::createPvEnum(const std::string& name, nds::PVDelegateIn<T>& pvRb, nds::enumerationStrings_t enumList,
                                     std::function<void(ADQAIChannelGroup*, const timespec&, const T&)> setter,
                                     std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter)
{
    nds::PVDelegateOut<T> node(name,
                               std::bind(setter, this, std::placeholders::_1, std::placeholders::_2),
                               std::bind(getter, this, std::placeholders::_1, std::placeholders::_2));
    node.setEnumeration(enumList);
    m_node.addChild(node);
    pvRb.setScanType(nds::scanType_t::interrupt);
    pvRb.setEnumeration(enumList);
    m_node.addChild(pvRb);
}

/* This function creates and returns the readback PV
 */
template <typename T>
nds::PVDelegateIn<T> ADQAIChannelGroup::createPvRb(const std::string& name,
                                                   std::function<void(ADQAIChannelGroup*, timespec*, T*)> getter)
{
    return nds::PVDelegateIn<T>(name, std::bind(getter, this, std::placeholders::_1, std::placeholders::_2));
}

void ADQAIChannelGroup::getLogMsg(timespec* pTimestamp, std::string* pValue)
{
    std::string text;
    *pValue = text;
    *pTimestamp = m_logMsgPV.getTimestamp();
}

void ADQAIChannelGroup::setDaqMode(const timespec& pTimestamp, const int32_t& pValue)
{
    m_daqMode = pValue;
    m_daqModePV.getTimestamp() = pTimestamp;
    m_daqModeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDaqMode(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_daqMode;
    *pTimestamp = m_daqModePV.getTimestamp();
}

void ADQAIChannelGroup::setPatternMode(const timespec& pTimestamp, const int32_t& pValue)
{
    m_patternMode = pValue;
    m_patternModePV.getTimestamp() = pTimestamp;
    m_patternModeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getPatternMode(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_patternMode;
    *pTimestamp = m_patternModePV.getTimestamp();
}

void ADQAIChannelGroup::setChanActive(const timespec& pTimestamp, const int32_t& pValue)
{
    m_chanActive = pValue;
    m_chanActivePV.getTimestamp() = pTimestamp;
    m_chanActiveChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getChanActive(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_chanActive;
    *pTimestamp = m_chanActivePV.getTimestamp();
}

void ADQAIChannelGroup::getChanMask(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_chanMask;
    *pTimestamp = m_chanMaskPV.getTimestamp();
}

void ADQAIChannelGroup::setDcBias(const timespec& pTimestamp, const int32_t& pValue)
{
    m_dcBias = pValue;
    m_dcBiasPV.getTimestamp() = pTimestamp;
    m_dcBiasChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDcBias(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_dcBias;
    *pTimestamp = m_dcBiasPV.getTimestamp();
}

void ADQAIChannelGroup::setDbsBypass(const timespec& pTimestamp, const int32_t& pValue)
{
    m_dbsBypass = pValue;
    m_dbsBypassPV.getTimestamp() = pTimestamp;
    m_dbsBypassChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsBypass(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_dbsBypass;
    *pTimestamp = m_dbsBypassPV.getTimestamp();
}

void ADQAIChannelGroup::setDbsDc(const timespec& pTimestamp, const int32_t& pValue)
{
    m_dbsDc = pValue;
    m_dbsDcPV.getTimestamp() = pTimestamp;
    m_dbsDcChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsDc(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_dbsDc;
    *pTimestamp = m_dbsDcPV.getTimestamp();
}

void ADQAIChannelGroup::setDbsLowSat(const timespec& pTimestamp, const int32_t& pValue)
{
    m_dbsLowSat = pValue;
    m_dbsLowSatPV.getTimestamp() = pTimestamp;
    m_dbsLowSatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsLowSat(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_dbsLowSat;
    *pTimestamp = m_dbsLowSatPV.getTimestamp();
}

void ADQAIChannelGroup::setDbsUpSat(const timespec& pTimestamp, const int32_t& pValue)
{
    m_dbsUpSat = pValue;
    m_dbsUpSatPV.getTimestamp() = pTimestamp;
    m_dbsUpSatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDbsUpSat(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_dbsUpSat;
    *pTimestamp = m_dbsUpSatPV.getTimestamp();
}

void ADQAIChannelGroup::setRecordCnt(const timespec& pTimestamp, const int32_t& pValue)
{
    m_recordCnt = pValue;
    m_recordCntPV.getTimestamp() = pTimestamp;
    m_recordCntChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getRecordCnt(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_recordCnt;
    *pTimestamp = m_recordCntPV.getTimestamp();
}

void ADQAIChannelGroup::setRecordCntCollect(const timespec& pTimestamp, const int32_t& pValue)
{
    m_recordCntCollect = pValue;
    m_recordCntCollectPV.getTimestamp() = pTimestamp;
    m_recordCntCollectChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getRecordCntCollect(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_recordCntCollect;
    *pTimestamp = m_recordCntCollectPV.getTimestamp();
}

void ADQAIChannelGroup::setSampleCnt(const timespec& pTimestamp, const int32_t& pValue)
{
    m_sampleCnt = pValue;
    m_sampleCntPV.getTimestamp() = pTimestamp;
    m_sampleCntChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getSampleCnt(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_sampleCnt;
    *pTimestamp = m_sampleCntPV.getTimestamp();
}

void ADQAIChannelGroup::getSampleCntMax(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_sampleCntMax;
    *pTimestamp = m_sampleCntMaxPV.getTimestamp();
}

void ADQAIChannelGroup::getSamplesTotal(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_sampleCntTotal;
    *pTimestamp = m_sampleCntTotalPV.getTimestamp();
}

void ADQAIChannelGroup::setSampleSkip(const timespec& pTimestamp, const int32_t& pValue)
{
    m_sampleSkip = pValue;
    m_sampleSkipPV.getTimestamp() = pTimestamp;
    m_sampleSkipChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getSampleSkip(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_sampleSkip;
    *pTimestamp = m_sampleSkipPV.getTimestamp();
}

void ADQAIChannelGroup::setTrigMode(const timespec& pTimestamp, const int32_t& pValue)
{
    m_trigMode = pValue;
    m_trigModePV.getTimestamp() = pTimestamp;
    m_trigModeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigMode(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_trigMode;
    *pTimestamp = m_trigModePV.getTimestamp();
}

void ADQAIChannelGroup::setTrigFreq(const timespec& pTimestamp, const int32_t& pValue)
{
    m_trigFreq = pValue;
    m_trigFreqPV.getTimestamp() = pTimestamp;
    m_trigFreqChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigFreq(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_trigFreq;
    *pTimestamp = m_trigFreqPV.getTimestamp();
}

void ADQAIChannelGroup::setTrigLvl(const timespec& pTimestamp, const int32_t& pValue)
{
    m_trigLvl = pValue;
    m_trigLvlPV.getTimestamp() = pTimestamp;
    m_trigLvlChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigLvl(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_trigLvl;
    *pTimestamp = m_trigLvlPV.getTimestamp();
}

void ADQAIChannelGroup::setTrigEdge(const timespec& pTimestamp, const int32_t& pValue)
{
    m_trigEdge = pValue;
    m_trigFreqPV.getTimestamp() = pTimestamp;
    m_trigEdgeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigEdge(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_trigEdge;
    *pTimestamp = m_trigEdgePV.getTimestamp();
}

void ADQAIChannelGroup::setTrigChan(const timespec& pTimestamp, const int32_t& pValue)
{
    m_trigChan = pValue;
    m_trigChanPV.getTimestamp() = pTimestamp;
    m_trigChanChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigChan(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_trigChan;
    *pTimestamp = m_trigChanPV.getTimestamp();
}

void ADQAIChannelGroup::setTimeout(const timespec& pTimestamp, const int32_t& pValue)
{
    m_timeout = pValue;
    m_timeoutPV.getTimestamp() = pTimestamp;
    m_timeoutChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTimeout(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_timeout;
    *pTimestamp = m_timeoutPV.getTimestamp();
}

void ADQAIChannelGroup::setStreamTime(const timespec& pTimestamp, const double& pValue)
{
    m_streamTime = pValue;
    m_streamTimePV.getTimestamp() = pTimestamp;
    m_streamTimeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getStreamTime(timespec* pTimestamp, double* pValue)
{
    *pValue = m_streamTime;
    *pTimestamp = m_streamTimePV.getTimestamp();
}

/* This function updates readback PVs according to changed ADQ parameters 
 */
void ADQAIChannelGroup::commitChanges(bool calledFromDaqThread)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    unsigned int status = 0;

    /* Changes to parameters are allowed when device is ON/STOPPING/INITIALISING states.
     * Changes are not applied when device is on acquisition state.
     */
    if (!calledFromDaqThread &&
        (m_stateMachine.getLocalState() != nds::state_t::on && m_stateMachine.getLocalState() != nds::state_t::stopping &&
         m_stateMachine.getLocalState() != nds::state_t::initializing))
    {
        return;
    }

    if (m_daqModeChanged)
    {
        m_daqModeChanged = false;
        m_daqModePV.push(now, m_daqMode);
    }

    if (m_patternModeChanged)
    {
        m_patternModeChanged = false;
        status = m_adqInterface->SetTestPatternMode(m_patternMode);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetTestPatternMode failed.");
        if (status)
        {
            m_patternModePV.push(now, m_patternMode);
            ADQNDS_MSG_INFOLOG_PV("SUCCESS: SetTestPatternMode");
        }
    }

    if (m_chanActiveChanged)
    {
        m_chanActiveChanged = false;

        if (!m_chanCnt)
        {
            int status = 0;
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: No channels are found.");
        }
        else
        {
            if (m_chanCnt == 1)
            {
                switch (m_chanActive)
                {
                    case 0:   // ch A
                        m_chanMask = 0x1;
                        m_chanInt = 1;
                        break;
                    case 1:   // ch B
                    case 2:   // ch A+B
                    case 3:   // ch D
                    case 4:   // ch A+B
                    case 5:   // ch C+D
                    case 6:   // ch A+B+C+D
                        int status = 0;
                        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Device has only one channel.");
                        m_chanMask = 0x1;
                        m_chanInt = 1;
                        break;
                }
            }

            if (m_chanCnt == 2)
            {
                switch (m_chanActive)
                {
                    case 0:   // ch A
                        m_chanMask = 0x1;
                        m_chanInt = 1;
                        break;
                    case 1:   // ch B
                        m_chanMask = 0x2;
                        m_chanInt = 2;
                        break;
                    case 2:   // ch A+B
                        m_chanMask = 0x3;
                        m_chanInt = 3;
                        break;
                    case 3:   // ch D
                    case 4:   // ch A+B
                    case 5:   // ch C+D
                    case 6:   // ch A+B+C+D
                        int status = 0;
                        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Device has only two channels.");
                        m_chanMask = 0x1;
                        m_chanInt = 1;
                        break;
                }
            }

            if (m_chanCnt == 4)
            {
                switch (m_chanActive)
                {
                    case 0:   // ch A
                        m_chanMask = 0x1;
                        m_chanInt = 1;
                        break;
                    case 1:   // ch B
                        m_chanMask = 0x2;
                        m_chanInt = 2;
                        break;
                    case 2:   // ch C
                        m_chanMask = 0x4;
                        m_chanInt = 4;
                        break;
                    case 3:   // ch D
                        m_chanMask = 0x8;
                        m_chanInt = 8;
                        break;
                    case 4:   // ch A+B
                        m_chanMask = 0x3;
                        m_chanInt = 3;
                        break;
                    case 5:   // ch C+D
                        m_chanMask = 0xC;
                        m_chanInt = 12;
                        break;
                    case 6:   // ch A+B+C+D
                        m_chanMask = 0xF;
                        m_chanInt = 15;
                        break;
                }
            }
        }

        m_chanMaskPV.push(now, m_chanMask);
        m_chanActivePV.push(now, m_chanActive);
    }

    if (m_dcBiasChanged)
    {
        m_dcBiasChanged = false;
        status = m_adqInterface->HasAdjustableBias();

        if (status)
        {
            unsigned int i = 0;
            for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
            {
                status = m_adqInterface->SetAdjustableBias(chan + 1, m_dcBias);
                if (status)
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == m_chanCnt)
            {
                ADQNDS_MSG_INFOLOG_PV("SUCCESS: SetAdjustableBias is set for all channels.");
                m_dcBiasPV.push(now, m_dcBias);
            }
            else
            {
                status = 0;
                ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetAdjustableBias failed on one or more channels.");
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

        status = m_adqInterface->GetNofDBSInstances(&dbsInstCnt);
        if (status)
        {
            unsigned int i = 0;
            for (unsigned char dbsInst = 0; dbsInst < dbsInstCnt; ++dbsInst)
            {
                status = m_adqInterface->SetupDBS(dbsInst, m_dbsBypass, m_dbsDc, m_dbsLowSat, m_dbsUpSat);
                if (status)
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == dbsInstCnt)
            {
                ADQNDS_MSG_INFOLOG_PV("SUCCESS: SetupDBS is set for all channels.");
                m_dbsBypassPV.push(now, m_dbsBypass);
                m_dbsDcPV.push(now, m_dbsDc);
                m_dbsLowSatPV.push(now, m_dbsLowSat);
                m_dbsUpSatPV.push(now, m_dbsUpSat);
            }
            else
            {
                status = 0;
                ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetupDBS failed on one or more channels.");
            }
        }
    }

    if (m_recordCntChanged || m_sampleCntChanged)
    {
        unsigned int sampleCntMax;

        m_recordCntChanged = false;
        m_sampleCntChanged = false;

        if (m_recordCnt < -1)
            m_recordCnt = -1;

        if (m_sampleCnt < 0)
            m_sampleCnt = 0;

        if ((m_recordCnt == -1) && (m_daqMode != 2))   // SW trigger or Multi-Record
        {
            m_recordCnt = 0;
            status = 0;
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Infinite collection is enabled only in Triggered mode.");
        }

        if (m_recordCnt >= 0)
        {
            m_recordCntPV.push(now, m_recordCnt);

            if (m_daqMode == 0)   // Multi-Record
            {
                status = m_adqInterface->GetMaxNofSamplesFromNofRecords(m_recordCnt, &sampleCntMax);
                ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Couldn't get the MAX number of samples: set number of records.");
                if (status)
                {
                    m_sampleCntMax = sampleCntMax;
                    m_sampleCntMaxPV.push(now, m_sampleCntMax);

                    if (m_sampleCnt > m_sampleCntMax)
                    {
                        status = 0;
                        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Chosen number of samples was higher than the maximum number of samples.");
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
            m_sampleCntTotalPV.push(now, m_sampleCnt);
        }
    }

    if (m_recordCntCollectChanged && (m_daqMode == 0))   // Multi-Record
    {
        m_recordCntCollectChanged = false;

        if (m_recordCntCollect > m_recordCnt)
        {
            m_recordCntCollect = m_recordCnt;
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Number of records to collect cannot be higher than total number of records.");
        }

        m_recordCntCollectPV.push(now, m_recordCntCollect);

        m_sampleCntTotal = m_sampleCnt * m_recordCntCollect;
        m_sampleCntTotalPV.push(now, m_sampleCntTotal);
    }

    if (m_trigModeChanged)
    {
        m_trigModeChanged = false;

        status = m_adqInterface->SetTriggerMode(m_trigMode + 1);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetTriggerMode failed.");
        if (status)
        {
            m_trigModePV.push(now, m_trigMode);
            ADQNDS_MSG_INFOLOG_PV("SUCCESS: SetTriggerMode");
        }
    }

    if (m_trigFreqChanged && (m_trigMode == 3))   // Internal trigger
    {
        m_trigFreqChanged = false;

        if (m_trigFreq <= 0)
            m_trigFreq = 1;

        m_trigFreqPV.push(now, m_trigFreq);
        m_trigPeriod = 1000000000 / m_trigFreq;
    }

    if (m_sampleSkipChanged && ((m_daqMode == 1) || (m_daqMode == 3)))   // Continuous streaming or Raw streaming
    {
        m_sampleSkipChanged = false;
        std::string adqOption = m_adqInterface->GetCardOption();

        if (m_sampleSkip <= 1)
        {
            m_sampleSkip = 2;
            ADQNDS_MSG_INFOLOG_PV("INFO: Sample skip can't be less than 2 -> changed to 2.");
        }

        if (m_sampleSkip > 65536)
        {
            m_sampleSkip = 65536;
            ADQNDS_MSG_INFOLOG_PV("INFO: Sample skip can't be more than 65536 -> changed to 65536.");
        }

        if ((adqOption.find("-2X") != std::string::npos) || (adqOption.find("-1X") != std::string::npos))
        {
            if (m_sampleSkip == 3)
            {
                m_sampleSkip = 4;
                ADQNDS_MSG_INFOLOG_PV("INFO: Sample skip can't be 3 -> changed to 4.");
            }

            if (m_sampleSkip == 5 || m_sampleSkip == 6 || m_sampleSkip == 7)
            {
                m_sampleSkip = 8;
                ADQNDS_MSG_INFOLOG_PV("INFO: Sample skip can't be 5, 6 or 7 -> changed to 8.");
            }
        }

        if ((adqOption.find("-4C") != std::string::npos) || (adqOption.find("-2C") != std::string::npos))
        {
            if (m_sampleSkip == 3)
            {
                m_sampleSkip = 4;
                ADQNDS_MSG_INFOLOG_PV("INFO: Sample skip can't be 3 -> changed to 4.");
            }
        }

        m_sampleSkipPV.push(now, m_sampleSkip);
    }

    if (m_trigEdgeChanged && (m_trigMode == 2))   // Level trigger
    {
        m_trigEdgeChanged = false;
        m_trigEdgePV.push(now, m_trigEdge);
    }

    if (m_trigLvlChanged && (m_trigMode == 2))   // Level trigger
    {
        m_trigLvlChanged = false;
        m_trigLvlPV.push(now, m_trigLvl);
    }

    if (m_trigChanChanged && (m_trigMode == 2))   // Level trigger
    {
        m_trigChanChanged = false;

        switch (m_trigChan)
        {
            case 0:   // None
                m_trigChanInt = 0;
                break;
            case 1:   // ch A
                m_trigChanInt = 1;
                break;
            case 2:   // ch B
            case 3:   // ch A+B
            case 4:   // ch D
            case 5:   // ch A+B
            case 6:   // ch C+D
            case 7:   // ch A+B+C+D
                int status = 0;
                ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Device has only one channel.");
                break;
        }

        if (m_chanCnt == 2)
        {
            switch (m_trigChan)
            {
                case 0:   // None
                    m_trigChanInt = 0;
                    break;
                case 1:   // ch A
                    m_trigChanInt = 1;
                    break;
                case 2:   // ch B
                    m_trigChanInt = 2;
                    break;
                case 3:   // ch A+B
                    if (m_adqType == 7)
                    {
                        int status = 0;
                        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: ADQ7 allows only one channel to generate the trigger.");
                    }
                    else
                    {
                        m_trigChanInt = 3;
                    }
                    break;
                case 4:   // ch D
                case 5:   // ch A+B
                case 6:   // ch C+D
                case 7:   // ch A+B+C+D
                    int status = 0;
                    ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Device has only two channels.");
                    break;
            }
        }

        if (m_chanCnt == 4)
        {
            switch (m_trigChan)
            {
                case 0:   // None
                    m_trigChanInt = 0;
                    break;
                case 1:   // ch A
                    m_trigChanInt = 1;
                    break;
                case 2:   // ch B
                    m_trigChanInt = 2;
                    break;
                case 3:   // ch C
                    m_trigChanInt = 4;
                    break;
                case 4:   // ch D
                    m_trigChanInt = 8;
                    break;
                case 5:   // ch A+B
                    m_trigChanInt = 3;
                    break;
                case 6:   // ch C+D
                    m_trigChanInt = 12;
                    break;
                case 7:   // ch A+B+C+D
                    m_trigChanInt = 15;
                    break;
            }
        }

        m_trigChanPV.push(now, m_trigChan);
    }

    if (m_timeoutChanged)
    {
        m_timeoutChanged = false;

        if (m_timeout <= 0)
        {
            m_timeout = 1000;
        }

        m_timeoutPV.push(now, m_timeout);
    }

    if (m_streamTimeChanged && (m_daqMode == 1))   // Continuous streaming
    {
        m_streamTimeChanged = false;

        if (m_streamTime <= 0)
        {
            m_streamTime = 1.0;
            ADQNDS_MSG_INFOLOG_PV("INFO: Streaming time can't be 0 seconds -> changed to 1.0 seconds.");
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
    if (m_daqMode == 3)
        m_daqThread = m_node.runInThread("DAQ-RawStream", std::bind(&ADQAIChannelGroup::daqRawStream, this));
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
    UNUSED(currentLocal);
    UNUSED(currentGlobal);
    UNUSED(nextLocal);
    return true;
}

static struct timespec tsref;
void timerStart(void)
{
    if (clock_gettime(CLOCK_REALTIME, &tsref) < 0)
    {
        std::cout << "WARNING: Failed to start timer." << std::endl;
        return;
    }
}
unsigned int timerTimeMs(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
    {
        std::cout << "WARNING: Failed to get system time." << std::endl;
        return -1;
    }
    return (unsigned int)((int)(ts.tv_sec - tsref.tv_sec) * 1000 + (int)(ts.tv_nsec - tsref.tv_nsec) / 1000000);
}

/* Data acquisition function for triggered streaming
 */
void ADQAIChannelGroup::daqTrigStream()
{
    int status = 0;
    unsigned int bufferSize;
    unsigned int recordsSum = 0;
    unsigned int preTrigSampleCnt = 0;
    unsigned int holdOffSampleCnt = 0;
    unsigned int buffersFilled;
    unsigned int recordsDone[CHANNEL_COUNT_MAX];
    unsigned int samplesAdded[CHANNEL_COUNT_MAX];
    unsigned int headersAdded[CHANNEL_COUNT_MAX];
    unsigned int samplesRemainData[CHANNEL_COUNT_MAX];
    unsigned int headerStatus[CHANNEL_COUNT_MAX];
    unsigned int headersDone = 0;
    unsigned int samplesRemain = 0;
    unsigned int gotRecordsInTotal = 0;
    bool recordsInTotalCheck = 0;

    if (m_adqType == 714 || m_adqType == 14)
    {
        bufferSize = 512 * 1024;
    }
    if (m_adqType == 7)
    {
        bufferSize = 256 * 1024;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        m_daqStreamHeaders[chan] = NULL;
        m_daqBuffers[chan] = NULL;
        m_daqExtra[chan] = NULL;

        recordsDone[chan] = 0;
        samplesAdded[chan] = 0;
        headersAdded[chan] = 0;
        samplesRemainData[chan] = 0;
        headerStatus[chan] = 0;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (!((1 << chan) & m_chanMask))
            continue;

        m_daqBuffers[chan] = (short int*)malloc((size_t)bufferSize);
        if (!m_daqBuffers[chan])
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: Failed to allocate memory for target buffers.");
        }

        m_daqStreamHeaders[chan] = (streamingHeader_t*)malloc((size_t)bufferSize);
        if (!m_daqStreamHeaders[chan])
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: Failed to allocate memory for target headers.");
        }

        m_daqExtra[chan] = (short int*)malloc((size_t)(sizeof(short) * m_sampleCnt));
        if (!m_daqExtra[chan])
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: Failed to allocate memory for target extradata.");
        }
    }

{ 

        switch (m_trigMode)
    {
    case 0:   // SW trigger
        ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Software trigger... ");
        break;
    case 1:   // External trigger
        ADQNDS_MSG_INFOLOG_PV("INFO: Issuing External trigger... ");
        break;
    case 2:   // Level trigger
        ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Level trigger... ");

        status = m_adqInterface->SetLvlTrigEdge(m_trigEdge);
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigEdge failed.");

        status = m_adqInterface->SetLvlTrigLevel(m_trigLvl);
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigLevel failed.");

        status = m_adqInterface->SetLvlTrigChannel(m_trigChanInt);
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigChannel failed.");
        break;
    case 3:   // Internal trigger
        ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Internal trigger... ");
        status = m_adqInterface->SetInternalTriggerPeriod(m_trigFreq);
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetInternalTriggerPeriod failed.");
        break;
    }

    // Compute the sum of the number of records specified by the user
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (!((1 << chan) & m_chanMask))
            continue;
        recordsSum += m_recordCnt;
    }

    status = m_adqInterface->TriggeredStreamingSetup(m_recordCnt, m_sampleCnt, preTrigSampleCnt, holdOffSampleCnt, m_chanMask);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: TriggeredStreamingSetup failed.");

    status = m_adqInterface->SetTransferBuffers(CHANNEL_COUNT_MAX, bufferSize);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetTransferBuffers failed.");

    status = m_adqInterface->StopStreaming();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: StopStreaming failed.");

    status = m_adqInterface->StartStreaming();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: StartStreaming failed.");

    if (m_trigMode == 0)
    {
        status = m_adqInterface->DisarmTrigger();
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: DisarmTrigger failed.");

        status = m_adqInterface->ArmTrigger();
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: ArmTrigger failed.");

        for (int i = 0; i < m_recordCnt; ++i)
        {
            status = m_adqInterface->SWTrig();
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SWTrig failed.");
        }
    }

    status = m_adqInterface->DisarmTrigger();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: DisarmTrigger failed.");

    status = m_adqInterface->ArmTrigger();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: ArmTrigger failed.");

    // Data acquisition loop
    do
    {
        // Mutex is used to save digitizer from interruptions during infinite collection
        std::lock_guard<std::mutex> lock(m_adqDevMutex);

        buffersFilled = 0;

            status = m_adqInterface->GetStreamOverflow();

        if (status)
        {
            status = 0;
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Streaming overflow detected.");
        }


            status = m_adqInterface->GetTransferBufferStatus(&buffersFilled);

        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: GetTransferBufferStatus failed.");

        // Poll for the transfer buffer status as long as the timeout has not been
        // reached and no buffers have been filled.
        while (!buffersFilled)
        {
            // Mark the loop start
            timerStart();
            while (!buffersFilled && (timerTimeMs() < (unsigned)m_timeout))
            {

                    status = m_adqInterface->GetTransferBufferStatus(&buffersFilled);

                ADQNDS_MSG_ERRLOG_PV(status, "ERROR: GetTransferBufferStatus failed.");
                // Sleep to avoid loading the processor too much
                sleep(10);
            }

            // Timeout reached, flush the transfer buffer to receive data
            if (!buffersFilled)
            {
                ADQNDS_MSG_INFOLOG_PV("INFO: Timeout, flushing DMA...");

                    status = m_adqInterface->FlushDMA();

                ADQNDS_MSG_ERRLOG_PV(status, "ERROR: FlushDMA failed.");
            }
        }

        ADQNDS_MSG_INFOLOG_PV("INFO: Receiving data...");

            status = m_adqInterface->GetDataStreaming((void**)m_daqBuffers, (void**)m_daqStreamHeaders, m_chanMask, samplesAdded, headersAdded, headerStatus);

        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: GetDataStreaming failed.");

        // Parse data
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            if (!((1 << chan) & m_chanMask))
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
                if (samplesRemainData[chan] > 0)
                {
                    if (headersDone == 0)
                    {
                        // There is not enough data in the transfer buffer to complete
                        // the record. Add all the samples to the extradata buffer.
                        std::memcpy(&m_daqExtra[chan][samplesRemainData[chan]], m_daqBuffers[chan], sizeof(short) * samplesAdded[chan]);
                        samplesRemain -= samplesAdded[chan];
                        samplesRemainData[chan] += samplesAdded[chan];
                    }
                    else
                    {
                        // There is enough data in the transfer buffer to complete
                        // the record. Add RecordLength-samples_extradata samples

                        if (m_recordCnt == -1)
                        {
                            // Read buffers from each channel and send them to DATA PVs
                            m_AIChannelsPtr[chan]->readData(m_daqExtra[chan], samplesRemainData[chan]);
                            m_AIChannelsPtr[chan]->readData(m_daqBuffers[chan], m_daqStreamHeaders[chan][0].recordLength - samplesRemainData[chan]);
                        }

                        samplesRemain -= m_daqStreamHeaders[chan][0].recordLength - samplesRemainData[chan];
                        samplesRemainData[chan] = 0;
                    }
                }
                else
                {
                    if (headersDone == 0)
                    {
                        // The samples in the transfer buffer begin a new record, this
                        // record is incomplete.
                        std::memcpy(m_daqExtra[chan], m_daqBuffers[chan], sizeof(short) * samplesAdded[chan]);
                        samplesRemain -= samplesAdded[chan];
                        samplesRemainData[chan] = samplesAdded[chan];
                    }
                    else
                    {
                        if (m_recordCnt == -1)
                        {
                            m_AIChannelsPtr[chan]->readData(m_daqBuffers[chan], m_daqStreamHeaders[chan][0].recordLength);
                        }

                        samplesRemain -= m_daqStreamHeaders[chan][0].recordLength;
                    }
                }
                // At this point: the first record in the transfer buffer or the entire
                // transfer buffer has been parsed.

                // Loop through complete records fully inside the buffer
                for (unsigned int i = 1; i < headersDone; ++i)
                {
                    if (m_recordCnt == -1)
                    {
                        m_AIChannelsPtr[chan]->readData((&m_daqBuffers[chan][samplesAdded[chan] - samplesRemain]), m_daqStreamHeaders[chan][i].recordLength);
                    }

                    samplesRemain -= m_daqStreamHeaders[chan][i].recordLength;
                }

                if (samplesRemain > 0)
                {
                    // There is an incomplete record at the end of the transfer buffer
                    // Copy the incomplete header to the start of the target_headers buffer
                    std::memcpy(m_daqStreamHeaders[chan], &m_daqStreamHeaders[chan][headersDone], sizeof(streamingHeader_t));

                    // Copy any remaining samples to the target_buffers_extradata buffer,
                    // they belong to the incomplete record
                    std::memcpy(m_daqExtra[chan], &m_daqBuffers[chan][samplesAdded[chan] - samplesRemain], sizeof(short) * samplesRemain);
                    samplesRemainData[chan] = samplesRemain;
                    samplesRemain = 0;
                }
            }

            if (m_recordCnt != -1)
            {
                // Read buffers from each channel and send them to DATA PVs
                m_AIChannelsPtr[chan]->readData(m_daqBuffers[chan], m_sampleCntTotal);
            }
        }

        // Update received_all_records
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            gotRecordsInTotal += recordsDone[chan];
        }

        // Determine if collection is completed or stopped
        recordsInTotalCheck = (gotRecordsInTotal >= recordsSum);
    } while (!recordsInTotalCheck && !(m_stateMachine.getLocalState() == nds::state_t::stopping));
    }

finish:
    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->DisarmTrigger();
    }

    ADQNDS_MSG_WARNLOG_PV(status, "ERROR: DisarmTrigger failed.");

    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->StopStreaming();
    }
    ADQNDS_MSG_WARNLOG_PV(status, "ERROR: StopStreaming failed.");

    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");
    commitChanges(true);

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqBuffers[chan])
        {
            free(m_daqBuffers[chan]);
            m_daqBuffers[chan] = NULL;
        }
        if (m_daqStreamHeaders[chan])
        {
            free(m_daqStreamHeaders[chan]);
            m_daqStreamHeaders[chan] = NULL;
        }
        if (m_daqExtra[chan])
        {
            free(m_daqExtra[chan]);
            m_daqExtra[chan] = NULL;
        }
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

/* Data acquisition function for multi-record
*/
void ADQAIChannelGroup::daqMultiRecord()   // Need to mention on GUI about triggering the device when ot SW trigger is used
{
    int trigged, status;
    void* daqVoidBuffers[CHANNEL_COUNT_MAX];

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        m_daqBuffers[chan] = NULL;
    }

    status = m_adqInterface->MultiRecordSetChannelMask(m_chanMask);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: MultiRecordSetChannelMask failed.");

    switch (m_trigMode)
    {
        case 0:   // SW trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Software trigger...");
            break;
        case 1:   // External trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing External trigger...");
            break;
        case 2:   // Level trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Level trigger...");

            status = m_adqInterface->SetLvlTrigEdge(m_trigEdge);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigEdge failed.");

            status = m_adqInterface->SetLvlTrigLevel(m_trigLvl);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigLevel failed.");

            status = m_adqInterface->SetLvlTrigChannel(m_trigChanInt);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigChannel failed.");
            break;
        case 3:   // Internal trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Internal trigger...");
            status = m_adqInterface->SetInternalTriggerPeriod(m_trigPeriod);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetInternalTriggerPeriod failed.");
            break;
    }

    status = m_adqInterface->MultiRecordSetup(m_recordCnt, m_sampleCnt);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: MultiRecordSetup failed.");

    status = m_adqInterface->DisarmTrigger();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: DisarmTrigger failed.");

    status = m_adqInterface->ArmTrigger();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: ArmTrigger failed.");

    if (m_trigMode == 0)   // SW trigger
    {
        do
        {
            trigged = m_adqInterface->GetAcquiredAll();
            for (int i = 0; i < m_recordCnt; ++i)
            {
                status = m_adqInterface->SWTrig();
                ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SWTrig failed.");
            }
        } while (trigged == 0);

        unsigned int acqRecTotal = m_adqInterface->GetAcquiredRecords();
        ndsInfoStream(m_node) << "INFO: GetAcquiredRecords: " << acqRecTotal << std::endl;
    }
    else
    {
        ADQNDS_MSG_INFOLOG_PV("INFO: Trigger the device to collect data.");
        do
        {
            trigged = m_adqInterface->GetAcquiredAll();
        } while (trigged == 0);

        unsigned int acqRecTotal = m_adqInterface->GetAcquiredRecords();
        ndsInfoStream(m_node) << "INFO: GetAcquiredRecords: " << acqRecTotal << std::endl;
    }

    status = m_adqInterface->GetStreamOverflow();
    if (status)
    {
        status = 0;
        ADQNDS_MSG_ERRLOG_PV(status, "WARNING: GetStreamOverflow detected.");
    }

    // Here sampleCntTotal should be calculated as (recordCntCollect * sampleCnt), what is taken care of in commitChanges method (recordCntCollectchanged)
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        m_daqBuffers[chan] = (short*)calloc(m_sampleCntTotal, sizeof(short));
    }

    // Create a pointer array containing the data buffer pointers
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        daqVoidBuffers[chan] = (void*)m_daqBuffers[chan];
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqBuffers[chan] == NULL)
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: Failed to allocate memory for target buffers.");
        }
    }

    status = m_adqInterface->GetData(daqVoidBuffers, m_sampleCntTotal, sizeof(short), 0, m_recordCntCollect, m_chanMask, 0, m_sampleCnt, ADQ_TRANSFER_MODE_NORMAL);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: GetData failed.");

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        // Read buffers by each channel and send them to DATA PVs
        m_AIChannelsPtr[chan]->readData((short*)daqVoidBuffers[chan], m_sampleCntTotal);
    }

finish:
    status = m_adqInterface->DisarmTrigger();
    ADQNDS_MSG_WARNLOG_PV(status, "ERROR: DisarmTrigger failed.");

    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");
    commitChanges(true);
    m_adqInterface->MultiRecordClose();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqBuffers[chan])
        {
            free(m_daqBuffers[chan]);
            m_daqBuffers[chan] = NULL;
        }
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

/* Data acquisition function for continuous streaming
*/
void ADQAIChannelGroup::daqContinStream()
{
    int status;
    unsigned int bufferSize, buffersFilled;
    time_t start_time, curr_time;
    double elapsedSeconds;
    int bufferStatusLoops;
    unsigned int samplesAdded[CHANNEL_COUNT_MAX];
    unsigned int headersAdded[CHANNEL_COUNT_MAX];
    unsigned int headerStatus[CHANNEL_COUNT_MAX];
    unsigned int samplesAddedTotal;
    uint64_t bytesParsedTotal;
    unsigned int streamCompleted = 0;

    if (m_adqType == 714 || m_adqType == 14)
    {
        bufferSize = 512 * 1024;
    }
    if (m_adqType == 7)
    {
        bufferSize = 256 * 1024;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; chan++)
    {
        m_daqBuffers[chan] = NULL;
        m_daqHeaders[chan] = NULL;
        if (m_chanMask & (1 << chan))
        {
            m_daqBuffers[chan] = (short*)malloc(bufferSize * sizeof(char));
            m_daqHeaders[chan] = (unsigned char*)malloc(10 * sizeof(uint32_t));
        }
    }

    status = m_adqInterface->StopStreaming();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: StopStreaming failed.");

    status = m_adqInterface->ContinuousStreamingSetup(m_chanMask);
    ADQNDS_MSG_WARNLOG_PV(status, "WARNING: ContinuousStreamingSetup failed.");

    status = m_adqInterface->SetSampleSkip(m_sampleSkip);
    ADQNDS_MSG_WARNLOG_PV(status, "WARNING: ContinuousStreamingSetup failed.");

    // Start streaming
    samplesAddedTotal = 0;
    bytesParsedTotal = 0;

    status = m_adqInterface->StartStreaming();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: StartStreaming failed.");

    time(&start_time);

    switch (m_trigMode)
    {
        case 0:   // SW trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Software trigger...");
            break;
        case 1:   // External trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing External trigger...");
            break;
        case 2:   // Level trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Level trigger...");

            status = m_adqInterface->SetLvlTrigEdge(m_trigEdge);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigEdge failed.");

            status = m_adqInterface->SetLvlTrigLevel(m_trigLvl);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigLevel failed.");

            status = m_adqInterface->SetLvlTrigChannel(m_trigChanInt);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigChannel failed.");
            break;
        case 3:   // Internal trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Internal trigger...");
            status = m_adqInterface->SetInternalTriggerPeriod(m_trigPeriod);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetInternalTriggerPeriod failed.");
            break;
    }

    if (m_trigMode == 0)
    {
        status = m_adqInterface->DisarmTrigger();
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: DisarmTrigger failed.");

        status = m_adqInterface->ArmTrigger();
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: ArmTrigger failed.");

        for (int i = 0; i < m_recordCnt; ++i)
        {
            status = m_adqInterface->SWTrig();
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SWTrig failed.");
        }
    }
    else
    {
        status = m_adqInterface->DisarmTrigger();
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: DisarmTrigger failed.");

        status = m_adqInterface->ArmTrigger();
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: ArmTrigger failed.");
    }

    time(&curr_time);
    while (!streamCompleted)
    {
        bufferStatusLoops = 0;
        buffersFilled = 0;

        while (buffersFilled == 0 && status)
        {
            status = m_adqInterface->GetTransferBufferStatus(&buffersFilled);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: GetTransferBufferStatus failed.");

            if (buffersFilled == 0)
            {
                sleep(10);
                bufferStatusLoops++;

                if (bufferStatusLoops > 2000)
                {
                    // If the DMA transfer is taking too long, we should flush the DMA buffer before it times out. The timeout defaults to 60 seconds.
                    ADQNDS_MSG_INFOLOG_PV("INFO: Timeout, flushing DMA...");
                    status = m_adqInterface->FlushDMA();
                    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: FlushDMA failed.");
                }
            }
        }

        for (unsigned int buf = 0; buf < buffersFilled; buf++)
        {
            ADQNDS_MSG_INFOLOG_PV("INFO: Receiving data...");
            status = m_adqInterface->GetDataStreaming((void**)m_daqBuffers, (void**)m_daqHeaders, m_chanMask, samplesAdded, headersAdded, headerStatus);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: GetDataStreaming failed.");

            for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
            {
                samplesAddedTotal += samplesAdded[chan];
            }

            bytesParsedTotal += (uint64_t)bufferSize;

            // Here, process data. (perform calculations, write to file, let another process operate on the buffers, etc).
            // This will likely be the bottleneck for the transfer rate.
            for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
            {
                if (!(m_chanMask & (1 << chan)))
                    continue;
                m_AIChannelsPtr[chan]->readData(m_daqBuffers[chan], samplesAddedTotal);
            }
        }

        time(&curr_time);
        elapsedSeconds = difftime(curr_time, start_time);

        if (elapsedSeconds > m_streamTime)
        {
            streamCompleted = 1;
            ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished due to achieved target stream time.");
        }

        status = m_adqInterface->GetStreamOverflow();
        if (status)
        {
            streamCompleted = 1;
            ADQNDS_MSG_INFOLOG_PV("ERROR: GetStreamOverflow detected.");
        }
    }

finish:
    status = m_adqInterface->DisarmTrigger();
    ADQNDS_MSG_WARNLOG_PV(status, "ERROR: DisarmTrigger failed.");

    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");
    commitChanges(true);
    m_adqInterface->StopStreaming();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqBuffers[chan])
        {
            free(m_daqBuffers[chan]);
            m_daqBuffers[chan] = NULL;
        }
        if (m_daqHeaders[chan])
        {
            free(m_daqHeaders[chan]);
            m_daqHeaders[chan] = NULL;
        }
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

/* Data acquisition function for raw streaming
*/
void ADQAIChannelGroup::daqRawStream()   // No idea how to implement multiple-channel-acquisition
{
    int status;
    unsigned int bufferSize, buffersFilled;
    int collectResult;
    unsigned int m_sampleCntCollect;

    if (m_chanActive > 3)
    {
        status = 0;
        ADQNDS_MSG_ERRLOG_PV(status, "ERROR: Channel mask should represent single channel.");
    }

    if (m_adqType == 714 || m_adqType == 14)
    {
        bufferSize = 64 * 1024;
    }
    if (m_adqType == 7)
    {
        bufferSize = 512 * 1024;
    }

    ADQNDS_MSG_INFOLOG_PV("INFO: Setting up streaming...");

    status = m_adqInterface->SetTransferBuffers(CHANNEL_COUNT_MAX, bufferSize);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetTransferBuffers failed.");

    m_sampleSkip = 8;

    status = m_adqInterface->SetSampleSkip(m_sampleSkip);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetSampleSkip failed.");

    status = m_adqInterface->SetStreamStatus(1);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetStreamStatus1 failed.");
    status = m_adqInterface->SetStreamConfig(2, 1);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetStreamConfig21 failed.");
    status = m_adqInterface->SetStreamConfig(3, m_chanInt);
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetStreamConfig3mask failed.");

    switch (m_trigMode)
    {
        case 0:   // SW trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Software trigger... ");
            break;
        case 1:   // External trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing External trigger... ");
            break;
        case 2:   // Level trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Level trigger... ");

            status = m_adqInterface->SetLvlTrigEdge(m_trigEdge);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigEdge failed.");

            status = m_adqInterface->SetLvlTrigLevel(m_trigLvl);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigLevel failed.");

            status = m_adqInterface->SetLvlTrigChannel(m_trigChanInt);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetLvlTrigChannel failed.");
            break;
        case 3:   // Internal trigger
            ADQNDS_MSG_INFOLOG_PV("INFO: Issuing Internal trigger... ");
            status = m_adqInterface->SetInternalTriggerPeriod(m_trigFreq);
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: SetInternalTriggerPeriod failed.");
            break;
    }

    ADQNDS_MSG_INFOLOG_PV("INFO: Receving data...");

    // Created temporary target for streaming data
    m_rawDaqBuffer = NULL;

    // Allocate temporary buffer for streaming data
    m_rawDaqBuffer = (signed short*)malloc(bufferSize * sizeof(signed short));

    status = m_adqInterface->StopStreaming();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: StopStreaming failed.");

    status = m_adqInterface->StartStreaming();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: StartStreaming failed.");

    m_sampleCntCollect = bufferSize;

    while (m_sampleCntCollect > 0)
    {
        unsigned int samples_in_buffer;
        do
        {
            collectResult = m_adqInterface->GetTransferBufferStatus(&buffersFilled);

        } while ((buffersFilled == 0) && (collectResult));

        collectResult = m_adqInterface->CollectDataNextPage();
        samples_in_buffer = MIN(m_adqInterface->GetSamplesPerPage(), m_sampleCntCollect);
        ndsInfoStream(m_node) << "GetSamplesPerPage " << m_adqInterface->GetSamplesPerPage() << std::endl;

        status = m_adqInterface->GetStreamOverflow();
        if (status)
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV(status, "ERROR: Streaming overflow detected.");
        }

        // Buffer all data in RAM before writing to disk, if streaming to disk is need a high performance
        // procedure could be implemented here.
        // Data format is set to 16 bits, so buffer size is Samples*2 bytes
        memcpy((void*)&m_rawDaqBuffer[bufferSize - m_sampleCntCollect],
               m_adqInterface->GetPtrStream(),
               samples_in_buffer * sizeof(signed short));
        m_sampleCntCollect -= samples_in_buffer;
    }

    status = m_adqInterface->StopStreaming();
    ADQNDS_MSG_ERRLOG_PV(status, "ERROR: StopStreaming failed.");

    m_sampleCntCollect = bufferSize;

    ADQNDS_MSG_INFOLOG_PV("INFO: Writing data to PVs...");

    // Read buffer and send its data to DATA PV
    m_AIChannelsPtr[m_chanActive]->readData(m_rawDaqBuffer, m_sampleCntCollect);

finish:
    status = m_adqInterface->DisarmTrigger();
    ADQNDS_MSG_WARNLOG_PV(status, "ERROR: DisarmTrigger failed.");

    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");
    commitChanges(true);

    if (m_rawDaqBuffer)
    {
        free(m_rawDaqBuffer);
        m_rawDaqBuffer = NULL;
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
    ndsInfoStream(m_node) << "Buffers freed." << std::endl;

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqBuffers[chan])
        {
            free(m_daqBuffers[chan]);
            m_daqBuffers[chan] = NULL;
        }
        if (m_daqStreamHeaders[chan])
        {
            free(m_daqStreamHeaders[chan]);
            m_daqStreamHeaders[chan] = NULL;
        }
        if (m_daqHeaders[chan])
        {
            free(m_daqHeaders[chan]);
            m_daqHeaders[chan] = NULL;
        }
        if (m_daqExtra[chan])
        {
            free(m_daqExtra[chan]);
            m_daqExtra[chan] = NULL;
        }
    }

    if (m_rawDaqBuffer)
    {
        free(m_rawDaqBuffer);
        m_rawDaqBuffer = NULL;
    }

    ndsInfoStream(m_node) << "ADQ Group Channel class was destructed." << std::endl;
}
