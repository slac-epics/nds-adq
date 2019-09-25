//
// Copyright (c) 2018 Cosylab d.d.
// This software is distributed under the terms found
// in file LICENSE.txt that is included with this distribution.
//

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

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface*& adqInterface, void* adqCtrlUnit) :
    ADQInfo(name, parentNode, adqInterface, adqCtrlUnit), 
    m_node(nds::Port(name + GROUP_CHAN_DEVICE, nds::nodeType_t::generic)),
    m_adqInterface(adqInterface), 
    m_logMsgPV(createPvRb<std::string>("LogMsg", &ADQAIChannelGroup::getLogMsg)),
    m_daqModePV(createPvRb<int32_t>("DAQMode-RB", &ADQAIChannelGroup::getDaqMode)),
    m_patternModePV(createPvRb<int32_t>("PatternMode-RB", &ADQAIChannelGroup::getPatternMode)),
    m_chanActivePV(createPvRb<int32_t>("ChanActive-RB", &ADQAIChannelGroup::getChanActive)),
    m_chanMaskPV(createPvRb<int32_t>("ChanMask-RB", &ADQAIChannelGroup::getChanMask)),
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
    m_sampleDecPV(createPvRb<int32_t>("SampDec-RB", &ADQAIChannelGroup::getSampleDec)),
    m_preTrigSampPV(createPvRb<int32_t>("PreTrigSamp-RB", &ADQAIChannelGroup::getPreTrigSamp)),
    m_trigHoldOffSampPV(createPvRb<int32_t>("TrigHoldOffSamp-RB", &ADQAIChannelGroup::getTrigHoldOffSamp)),
    m_clockSrcPV(createPvRb<int32_t>("ClockSrc-RB", &ADQAIChannelGroup::getClockSrc)),
    m_clockRefOutPV(createPvRb<int32_t>("ClockRefOut-RB", &ADQAIChannelGroup::getClockRefOut)),
    m_trigModePV(createPvRb<int32_t>("TrigMode-RB", &ADQAIChannelGroup::getTrigMode)),
    m_swTrigEdgePV(createPvRb<int32_t>("SWTrigEdge-RB", &ADQAIChannelGroup::getSWTrigEdge)),
    m_levelTrigLvlPV(createPvRb<int32_t>("LevelTrigLvl-RB", &ADQAIChannelGroup::getLevelTrigLvl)),
    m_levelTrigEdgePV(createPvRb<int32_t>("LevelTrigEdge-RB", &ADQAIChannelGroup::getLevelTrigEdge)),
    m_levelTrigChanPV(createPvRb<int32_t>("LevelTrigChan-RB", &ADQAIChannelGroup::getLevelTrigChan)),
    m_levelTrigChanMaskPV(createPvRb<int32_t>("LevelTrigChanMask-RB", &ADQAIChannelGroup::getLevelTrigChanMask)),
    m_externTrigDelayPV(createPvRb<int32_t>("ExternTrigDelay-RB", &ADQAIChannelGroup::getExternTrigDelay)),
    m_externTrigThresholdPV(createPvRb<double>("ExternTrigThreshold-RB", &ADQAIChannelGroup::getExternTrigThreshold)),
    m_externTrigEdgePV(createPvRb<int32_t>("ExternTrigEdge-RB", &ADQAIChannelGroup::getExternTrigEdge)),
    m_internTrigHighSampPV(createPvRb<int32_t>("InternTrigHighSamp-RB", &ADQAIChannelGroup::getInternTrigHighSamp)),
    m_internTrigLowSampPV(createPvRb<int32_t>("InternTrigLowSamp-RB", &ADQAIChannelGroup::getInternTrigLowSamp)),
    m_internTrigFreqPV(createPvRb<int32_t>("InternTrigFreq-RB", &ADQAIChannelGroup::getInternTrigFreq)),
    m_internTrigEdgePV(createPvRb<int32_t>("InternTrigEdge-RB", &ADQAIChannelGroup::getInternTrigEdge)),
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
        channelName << "Ch" << channelNum;
        m_AIChannelsPtr.push_back(std::make_shared<ADQAIChannel>(channelName.str(), m_node, channelNum));
    }

    // PV for error/warning/info messages
    m_logMsgPV.setScanType(nds::scanType_t::interrupt);
    m_logMsgPV.setMaxElements(512);
    m_node.addChild(m_logMsgPV);

    // PV for data acquisition modes
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

    // PVs for DBS setup
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
    createPv<int32_t>("SampDec", m_sampleDecPV, &ADQAIChannelGroup::setSampleDec, &ADQAIChannelGroup::getSampleDec);
    createPv<int32_t>("PreTrigSamp", m_preTrigSampPV, &ADQAIChannelGroup::setPreTrigSamp, &ADQAIChannelGroup::getPreTrigSamp);
    createPv<int32_t>("TrigHoldOffSamp", m_trigHoldOffSampPV, &ADQAIChannelGroup::setTrigHoldOffSamp, &ADQAIChannelGroup::getTrigHoldOffSamp);

    // PVs for clock
    createPv<int32_t>("ClockSrc", m_clockSrcPV, &ADQAIChannelGroup::setClockSrc, &ADQAIChannelGroup::getClockSrc);
    createPv<int32_t>("ClockRefOut", m_clockRefOutPV, &ADQAIChannelGroup::setClockRefOut, &ADQAIChannelGroup::getClockRefOut);

    // PV for Trigger Mode
    nds::enumerationStrings_t trigModeList = { "SW trigger", "External trigger", "Level trigger", "Internal trigger" };
    createPvEnum<int32_t>("TrigMode", m_trigModePV, trigModeList, &ADQAIChannelGroup::setTrigMode, &ADQAIChannelGroup::getTrigMode);

    // PV for level trigger level
    createPv<int32_t>("LevelTrigLvl", m_levelTrigLvlPV, &ADQAIChannelGroup::setLevelTrigLvl, &ADQAIChannelGroup::getLevelTrigLvl);

    // PV for level trigger edge
    nds::enumerationStrings_t triggerEdgeList = { "Falling edge", "Rising edge", "Both edges" };
    createPvEnum<int32_t>("LevelTrigEdge", m_levelTrigEdgePV, triggerEdgeList, &ADQAIChannelGroup::setLevelTrigEdge, &ADQAIChannelGroup::getLevelTrigEdge);

    // PV for level trigger channel
    nds::enumerationStrings_t trigChanList = { "A", "B", "C", "D", "A+B", "C+D", "A+B+C+D" };
    createPvEnum<int32_t>("LevelTrigChan", m_levelTrigChanPV, trigChanList, &ADQAIChannelGroup::setLevelTrigChan, &ADQAIChannelGroup::getLevelTrigChan);

    m_levelTrigChanMaskPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_levelTrigChanMaskPV);

    // PV for SW trigger edge
    createPvEnum<int32_t>("SWTrigEdge", m_swTrigEdgePV, triggerEdgeList, &ADQAIChannelGroup::setSWTrigEdge, &ADQAIChannelGroup::getSWTrigEdge);

    // PV for external trigger delay
    createPv<int32_t>("ExternTrigDelay", m_externTrigDelayPV, &ADQAIChannelGroup::setExternTrigDelay, &ADQAIChannelGroup::getExternTrigDelay);

    // PV for external trigger threshold
    createPv<double>("ExternTrigThreshold", m_externTrigThresholdPV, &ADQAIChannelGroup::setExternTrigThreshold, &ADQAIChannelGroup::getExternTrigThreshold);

    // PV for external trigger edge
    createPvEnum<int32_t>("ExternTrigEdge", m_externTrigEdgePV, triggerEdgeList, &ADQAIChannelGroup::setExternTrigEdge, &ADQAIChannelGroup::getExternTrigEdge);

    // PV for internal trigger high samples length
    createPv<int32_t>("InternTrigHighSamp", m_internTrigHighSampPV, &ADQAIChannelGroup::setInternTrigHighSamp, &ADQAIChannelGroup::getInternTrigHighSamp);

    // PV for internal trigger low samples length
    createPv<int32_t>("InternTrigLowSamp", m_internTrigLowSampPV, &ADQAIChannelGroup::setInternTrigLowSamp, &ADQAIChannelGroup::getInternTrigLowSamp);

    // PV for trigger frequency
    createPv<int32_t>("InternTrigFreq", m_internTrigFreqPV, &ADQAIChannelGroup::setInternTrigFreq, &ADQAIChannelGroup::getInternTrigFreq);

    // PV for internal trigger edge
    createPvEnum<int32_t>("InternTrigEdge", m_internTrigEdgePV, triggerEdgeList, &ADQAIChannelGroup::setInternTrigEdge, &ADQAIChannelGroup::getInternTrigEdge);

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

void ADQAIChannelGroup::setSampleDec(const timespec& pTimestamp, const int32_t& pValue)
{
    m_sampleDec = pValue;
    m_sampleDecPV.getTimestamp() = pTimestamp;
    m_sampleDecChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getSampleDec(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_sampleDec;
    *pTimestamp = m_sampleDecPV.getTimestamp();
}

void ADQAIChannelGroup::setPreTrigSamp(const timespec& pTimestamp, const int32_t& pValue)
{
    m_preTrigSamp = pValue;
    m_preTrigSampPV.getTimestamp() = pTimestamp;
    m_preTrigSampChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getPreTrigSamp(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_preTrigSamp;
    *pTimestamp = m_preTrigSampPV.getTimestamp();
}

void ADQAIChannelGroup::setTrigHoldOffSamp(const timespec& pTimestamp, const int32_t& pValue)
{
    m_trigHoldOffSamp = pValue;
    m_trigHoldOffSampPV.getTimestamp() = pTimestamp;
    m_trigHoldOffSampChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTrigHoldOffSamp(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_trigHoldOffSamp;
    *pTimestamp = m_trigHoldOffSampPV.getTimestamp();
}

void ADQAIChannelGroup::setClockSrc(const timespec& pTimestamp, const int32_t& pValue)
{
    m_clockSrc = pValue;
    m_clockSrcPV.getTimestamp() = pTimestamp;
    m_clockSrcChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getClockSrc(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_clockSrc;
    *pTimestamp = m_clockSrcPV.getTimestamp();
}

void ADQAIChannelGroup::setClockRefOut(const timespec& pTimestamp, const int32_t& pValue)
{
    m_clockRefOut = pValue;
    m_clockRefOutPV.getTimestamp() = pTimestamp;
    m_clockRefOutChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getClockRefOut(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_clockRefOut;
    *pTimestamp = m_clockRefOutPV.getTimestamp();
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

void ADQAIChannelGroup::setSWTrigEdge(const timespec& pTimestamp, const int32_t& pValue)
{
    m_swTrigEdge = pValue;
    m_swTrigEdgePV.getTimestamp() = pTimestamp;
    m_swTrigEdgeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getSWTrigEdge(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_swTrigEdge;
    *pTimestamp = m_swTrigEdgePV.getTimestamp();
}

void ADQAIChannelGroup::setLevelTrigLvl(const timespec& pTimestamp, const int32_t& pValue)
{
    m_levelTrigLvl = pValue;
    m_levelTrigLvlPV.getTimestamp() = pTimestamp;
    m_levelTrigLvlChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getLevelTrigLvl(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_levelTrigLvl;
    *pTimestamp = m_levelTrigLvlPV.getTimestamp();
}

void ADQAIChannelGroup::setLevelTrigEdge(const timespec& pTimestamp, const int32_t& pValue)
{
    m_levelTrigEdge = pValue;
    m_levelTrigEdgePV.getTimestamp() = pTimestamp;
    m_levelTrigEdgeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getLevelTrigEdge(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_levelTrigEdge;
    *pTimestamp = m_levelTrigEdgePV.getTimestamp();
}

void ADQAIChannelGroup::setLevelTrigChan(const timespec& pTimestamp, const int32_t& pValue)
{
    m_levelTrigChan = pValue;
    m_levelTrigChanPV.getTimestamp() = pTimestamp;
    m_levelTrigChanChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getLevelTrigChan(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_levelTrigChan;
    *pTimestamp = m_levelTrigChanPV.getTimestamp();
}

void ADQAIChannelGroup::getLevelTrigChanMask(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_levelTrigChanMask;
    *pTimestamp = m_levelTrigChanMaskPV.getTimestamp();
}

void ADQAIChannelGroup::setExternTrigDelay(const timespec& pTimestamp, const int32_t& pValue)
{
    m_externTrigDelay = pValue;
    m_externTrigDelayPV.getTimestamp() = pTimestamp;
    m_externTrigDelayChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getExternTrigDelay(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_externTrigDelay;
    *pTimestamp = m_externTrigDelayPV.getTimestamp();
}

void ADQAIChannelGroup::setExternTrigThreshold(const timespec& pTimestamp, const double& pValue)
{
    m_externTrigThreshold = pValue;
    m_externTrigThresholdPV.getTimestamp() = pTimestamp;
    m_externTrigThresholdChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getExternTrigThreshold(timespec* pTimestamp, double* pValue)
{
    *pValue = m_externTrigThreshold;
    *pTimestamp = m_externTrigThresholdPV.getTimestamp();
}

void ADQAIChannelGroup::setExternTrigEdge(const timespec& pTimestamp, const int32_t& pValue)
{
    m_externTrigEdge = pValue;
    m_externTrigEdgePV.getTimestamp() = pTimestamp;
    m_externTrigEdgeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getExternTrigEdge(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_externTrigEdge;
    *pTimestamp = m_externTrigEdgePV.getTimestamp();
}

void ADQAIChannelGroup::setInternTrigHighSamp(const timespec& pTimestamp, const int32_t& pValue)
{
    m_internTrigHighSamp = pValue;
    m_internTrigHighSampPV.getTimestamp() = pTimestamp;
    m_internTrigHighSampChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getInternTrigHighSamp(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_internTrigHighSamp;
    *pTimestamp = m_internTrigHighSampPV.getTimestamp();
}

void ADQAIChannelGroup::setInternTrigLowSamp(const timespec& pTimestamp, const int32_t& pValue)
{
    m_internTrigLowSamp = pValue;
    m_internTrigLowSampPV.getTimestamp() = pTimestamp;
    m_internTrigLowSampChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getInternTrigLowSamp(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_internTrigLowSamp;
    *pTimestamp = m_internTrigLowSampPV.getTimestamp();
}

void ADQAIChannelGroup::setInternTrigFreq(const timespec& pTimestamp, const int32_t& pValue)
{
    m_internTrigFreq = pValue;
    m_internTrigFreqPV.getTimestamp() = pTimestamp;
    m_internTrigFreqChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::setInternTrigEdge(const timespec& pTimestamp, const int32_t& pValue)
{
    m_internTrigEdge = pValue;
    m_internTrigEdgePV.getTimestamp() = pTimestamp;
    m_internTrigEdgeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getInternTrigEdge(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_internTrigEdge;
    *pTimestamp = m_internTrigEdgePV.getTimestamp();
}

void ADQAIChannelGroup::getInternTrigFreq(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_internTrigFreq;
    *pTimestamp = m_internTrigFreqPV.getTimestamp();
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

/* This function updates readback PVs according to changed ADQ parameters.
 * It also sets trigger options with ADQ API functions.
 */
void ADQAIChannelGroup::commitChanges(bool calledFromDaqThread)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
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

        if ((m_daqMode == 2) && ((m_adqType == 714) || (m_adqType == 14)))   // Triggered streaming and ADQ14
        {
            status = m_adqInterface->HasTriggeredStreamingFunctionality();
            if (!status)
            {
                ADQNDS_MSG_WARNLOG_PV(status, "WARNING: Device doesn't have triggered streaming functionaly.");
                m_daqMode = 1;
            }
        }
        
        if (m_daqMode == 3) // Raw streaming -> check channel mask, only one channel can be active
        {
            m_chanActiveChanged = true;
        }
        
        m_daqModePV.push(now, m_daqMode);

        // Trigger sample and records numbers to update
        m_recordCntPV.read(&now, &m_recordCnt);
        m_recordCntPV.push(now, m_recordCnt);
        m_sampleCntPV.read(&now, &m_sampleCnt);
        m_sampleCntPV.push(now, m_sampleCnt);
        // Trigger records to collect to update
        m_recordCntCollectPV.read(&now, &m_recordCntCollect);
        m_recordCntCollectPV.push(now, m_recordCntCollect);
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

    if (m_trigModeChanged)
    {
        m_trigModeChanged = false;

        status = m_adqInterface->SetTriggerMode(m_trigMode + 1);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetTriggerMode failed.");
        if (status)
        {
            m_trigMode = m_adqInterface->GetTriggerMode();
            m_trigMode -= 1;
            m_trigModePV.push(now, m_trigMode);
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
                        ADQNDS_MSG_INFOLOG_PV("INFO: Device has only one channel -> changed to channel A.");
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
                        if (m_daqMode != 3)
                        {
                            m_chanMask = 0x3;
                            m_chanInt = 3;
                        }
                        else
                        {
                            ADQNDS_MSG_INFOLOG_PV("INFO: Only one channel can be set for Raw Streaming -> changed to "
                                                  "channel B.");
                            m_chanMask = 0x2;
                            m_chanInt = 2;
                        }
                        break;
                    case 3:   // ch D
                    case 4:   // ch A+B
                    case 5:   // ch C+D
                    case 6:   // ch A+B+C+D
                        ADQNDS_MSG_INFOLOG_PV("INFO: Device has only two channels -> changed to channels A+B.");
                        m_chanMask = 0x3;
                        m_chanInt = 3;
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

                if ((m_daqMode == 3) && (m_chanInt > 8))   // Raw streaming
                {
                    ADQNDS_MSG_INFOLOG_PV("INFO: Only one channel can be set for Raw Streaming -> changed to channel "
                                          "D.");
                    m_chanMask = 0x8;
                    m_chanInt = 8;
                    m_chanActive = 3;
                }
            }
        }

        m_chanMaskPV.push(now, m_chanMask);
        m_chanActivePV.push(now, m_chanActive);
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
            if (m_dbsBypass < 0)
                m_dbsBypass = 0;
            if (m_dbsBypass > 1)
                m_dbsBypass = 1;
            if (m_dbsLowSat < 0)
                m_dbsLowSat = 0;
            if (m_dbsUpSat < 0)
                m_dbsUpSat = 0;

            unsigned int i = 0;
            for (unsigned char dbsInst = 0; dbsInst < dbsInstCnt; ++dbsInst)
            {
                status = m_adqInterface->SetupDBS(dbsInst, m_dbsBypass, m_dbsDc, m_dbsLowSat, m_dbsUpSat);
                if (status)
                {
                    ++i;
                }
            }
            SLEEP(1000);

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
        else
        {
            status = 0;
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: GetNofDBSInstances failed.");
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

        if ((m_recordCnt == -1) && (m_daqMode != 2))
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

            // Trigger records to collect to update
            m_recordCntCollectPV.read(&now, &m_recordCntCollect);
            m_recordCntCollectPV.push(now, m_recordCntCollect);
        }
        else
        {
            m_recordCntPV.push(now, m_recordCnt);
            m_sampleCntPV.push(now, m_sampleCnt);
            m_sampleCntTotalPV.push(now, m_sampleCnt);

            // Trigger records to collect to update
            m_recordCntCollectPV.read(&now, &m_recordCntCollect);
            m_recordCntCollectPV.push(now, m_recordCntCollect);
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

    if (m_sampleSkipChanged)
    {
        m_sampleSkipChanged = false;
        std::string adqOption = m_adqInterface->GetCardOption();

        if (m_sampleSkip < 1)
        {
            m_sampleSkip = 1;
            ADQNDS_MSG_INFOLOG_PV("INFO: Sample skip can't be less than 1 -> changed to 1.");
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

        status = m_adqInterface->SetSampleSkip(m_sampleSkip);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetSampleSkip failed.");

        m_sampleSkip = m_adqInterface->GetSampleSkip();
        m_sampleSkipPV.push(now, m_sampleSkip);

        // Trigger sample rate with decimation to update
        double tmp = 0;
        m_sampRateDecPV.read(&now, &tmp);
        m_sampRateDecPV.push(now, tmp);
    }

    if (m_sampleDecChanged)
    {
        m_sampleDecChanged = false;
        std::string adqOption = m_adqInterface->GetCardOption();

        if ((m_adqType == 714 || m_adqType == 14) && (adqOption.find("-FWSDR") != std::string::npos))
        {
            if (m_sampleDec < 0)
            {
                m_sampleDec = 0;
            }

            status = m_adqInterface->SetSampleDecimation(m_sampleDec);
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetSampleDecimation failed.");

            if (status)
            {
                m_sampleDec = m_adqInterface->GetSampleDecimation();
                m_sampleSkipPV.push(now, m_sampleDec);
            }

            // Trigger sample rate with decimation to update
            double tmp = 0;
            m_sampRateDecPV.read(&now, &tmp);
            m_sampRateDecPV.push(now, tmp);
        }
        else
        {
            ADQNDS_MSG_INFOLOG_PV("INFO: Sample decimation is not supported on this device.");
        }
    }

    if (m_preTrigSampChanged)
    {
        m_preTrigSampChanged = false;

        if (m_preTrigSamp > m_sampleCnt)
        {
            m_preTrigSamp = 0;
            ADQNDS_MSG_INFOLOG_PV("INFO: Number of pre-trigger samples must be less than number of samples per "
                                  "record.");
        }

        if (m_preTrigSamp < 0)
        {
            m_preTrigSamp = 0;
        }

        if (m_daqMode == 0)   // Multi-Record
        {
            status = m_adqInterface->SetPreTrigSamples(m_preTrigSamp);
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetPreTrigSamples failed.");
        }
        m_preTrigSampPV.push(now, m_preTrigSamp);

        if (m_preTrigSamp > 0)
        {
            ADQNDS_MSG_INFOLOG_PV("INFO: Trigger hold-off is resetted to zero.");
            m_trigHoldOffSampPV.push(now, 0);
        }
    }

    if (m_trigHoldOffSampChanged)
    {
        m_trigHoldOffSampChanged = false;

        if (m_trigHoldOffSamp > m_sampleCnt)
        {
            m_trigHoldOffSamp = 0;
            ADQNDS_MSG_INFOLOG_PV("INFO: Number of hold-off samples must be less than number of samples per record.");
        }

        if (m_trigHoldOffSamp < 0)
        {
            m_trigHoldOffSamp = 0;
        }

        if (m_daqMode == 0)   // Multi-Record
        {
            status = m_adqInterface->SetTriggerHoldOffSamples(m_trigHoldOffSamp);
            ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetTriggerHoldOffSamples failed.");
        }
        m_trigHoldOffSampPV.push(now, m_trigHoldOffSamp);

        if (m_trigHoldOffSamp > 0)
        {
            ADQNDS_MSG_INFOLOG_PV("INFO: Pre-trigger setting is resetted to zero.");
            m_preTrigSampPV.push(now, 0);
        }
    }

    if (m_trigMode == 0)   // SW trigger
    {
        if (m_swTrigEdgeChanged)
        {
            m_swTrigEdgeChanged = false;

            status = m_adqInterface->SetTriggerEdge(m_trigMode + 1, m_swTrigEdge);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetTriggerEdge failed.");

            if (status)
            {
                unsigned int trigEdge = 0;
                status = m_adqInterface->GetTriggerEdge(m_trigMode + 1, &trigEdge);
                ADQNDS_MSG_WARNLOG_PV(status, "ERROR: GetTriggerEdge failed.");
                if (status)
                {
                    m_swTrigEdge = trigEdge;
                    m_swTrigEdgePV.push(now, m_swTrigEdge);
                }
            }
        }
    }

    if (m_trigMode == 1)   // External trigger
    {
        if (m_externTrigDelayChanged)
        {
            if ((m_adqType == 714) || (m_adqType == 14))
            {
                if (m_externTrigDelay < 0)
                    m_externTrigDelay = 0;
                if (m_externTrigDelay > 37)
                    m_externTrigDelay = 37;

                ADQNDS_MSG_INFOLOG_PV("INFO: Trigger delay: for ADQ14 valid range is [0, 37].");
            }

            if (m_adqType == 7)
            {
                if (m_externTrigDelay < 1)
                    m_externTrigDelay = 1;
                if (m_externTrigDelay > 63)
                    m_externTrigDelay = 63;

                ADQNDS_MSG_INFOLOG_PV("INFO: Trigger delay: for ADQ7 valid range is [1, 63].");
            }

            status = m_adqInterface->SetExternalTriggerDelay(m_externTrigDelay);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetExternalTriggerDelay failed.");

            m_externTrigDelayChanged = false;
            m_externTrigDelayPV.push(now, m_externTrigDelay);
        }

        if (m_externTrigThresholdChanged)
        {
            m_externTrigThresholdChanged = false;
            if (m_adqInterface->HasVariableTrigThreshold(EXTERN_TRIG_COUNT))
            {
                if (m_adqType == 714 || m_adqType == 14)
                {
                    status = m_adqInterface->SetExtTrigThreshold(EXTERN_TRIG_COUNT, m_externTrigThreshold);
                    ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetExtTrigThreshold failed.");

                    if (status)
                        m_externTrigThresholdPV.push(now, m_externTrigThreshold);
                }
                if (m_adqType == 7)
                {
                    status = m_adqInterface->SetTriggerThresholdVoltage(m_trigMode + 1, m_externTrigThreshold);
                    ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetExtTrigThreshold failed.");

                    if (status)
                        m_externTrigThresholdPV.push(now, m_externTrigThreshold);
                }
            }
            else
            {
                ADQNDS_MSG_INFOLOG_PV("INFO: Device doesn't have a programmable extern trigger threshold.");
            }
        }

        if (m_externTrigEdgeChanged)
        {
            m_externTrigEdgeChanged = false;

            status = m_adqInterface->SetTriggerEdge(m_trigMode + 1, m_externTrigEdge);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetTriggerEdge failed.");

            if (status)
            {
                unsigned int trigEdge = 0;
                status = m_adqInterface->GetTriggerEdge(m_trigMode + 1, &trigEdge);
                ADQNDS_MSG_WARNLOG_PV(status, "ERROR: GetTriggerEdge failed.");
                if (status)
                {
                    m_externTrigEdge = trigEdge;
                    m_externTrigEdgePV.push(now, m_externTrigEdge);
                }
            }
        }
    }

    if (m_clockSrcChanged)
    {
        m_clockSrcChanged = false;

        if (m_clockSrc < 0)
        {
            m_clockSrc = 0;
        }

        if (m_clockSrc > 6)
        {
            m_clockSrc = 6;
        }

        status = m_adqInterface->SetClockSource(m_clockSrc);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetClockSource failed.");

        if (m_adqType == 714 || m_adqType == 14)
        {
            m_clockSrc = m_adqInterface->GetClockSource();
        }

        if (status)
            m_clockSrcPV.push(now, m_clockSrc);
    }

    if (m_clockRefOutChanged)
    {
        m_clockRefOutChanged = false;

        if (m_clockRefOut < 0)
        {
            m_clockRefOut = 0;
        }

        if (m_clockRefOut > 1)
        {
            m_clockRefOut = 1;
        }

        status = m_adqInterface->EnableClockRefOut(m_clockRefOut);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: EnableClockRefOut failed.");
        if (status)
            m_clockRefOutPV.push(now, m_clockRefOut);
    }

    if (m_trigMode == 2)   // Level trigger
    {
        if (m_levelTrigEdgeChanged)
        {
            m_levelTrigEdgeChanged = false;

            status = m_adqInterface->SetTriggerEdge(m_trigMode + 1, m_levelTrigEdge);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetLvlTrigEdge failed.");

            if (status)
            {
                unsigned int trigEdge = 0;
                status = m_adqInterface->GetTriggerEdge(m_trigMode + 1, &trigEdge);
                ADQNDS_MSG_WARNLOG_PV(status, "ERROR: GetTriggerEdge failed.");
                if (status)
                {
                    m_levelTrigEdge = trigEdge;
                    m_levelTrigEdgePV.push(now, m_levelTrigEdge);
                }
            }
        }

        if (m_levelTrigLvlChanged)
        {
            m_levelTrigLvlChanged = false;

            status = m_adqInterface->SetLvlTrigLevel(m_levelTrigLvl);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetLvlTrigLevel failed.");

            if (m_adqType == 714 || m_adqType == 14)
                m_levelTrigLvl = m_adqInterface->GetLvlTrigLevel();

            m_levelTrigLvlPV.push(now, m_levelTrigLvl);
        }

        if (m_levelTrigChanChanged)
        {
            m_levelTrigChanChanged = false;

            if (m_chanCnt == 1)
            {
                switch (m_levelTrigChan)
                {
                    case 0:   // ch A
                        m_levelTrigChanMask = 1;
                        break;
                    case 1:   // ch B
                        m_levelTrigChanMask = 1;
                        break;
                    case 2:   // ch C
                    case 3:   // ch D
                    case 4:   // ch A+B
                    case 5:   // ch C+D
                    case 6:   // ch A+B+C+D
                        ADQNDS_MSG_INFOLOG_PV("INFO: Device has only one channel -> changed to channel A");
                        m_levelTrigChanMask = 1;
                        break;
                }
            }

            if (m_chanCnt == 2)
            {
                switch (m_levelTrigChan)
                {
                    case 0:   // ch A
                        m_levelTrigChanMask = 1;
                        break;
                    case 1:   // ch B
                        m_levelTrigChanMask = 2;
                        break;
                    case 2:   // ch A+B
                        if (m_adqType == 7)
                        {
                            ADQNDS_MSG_INFOLOG_PV("INFO: ADQ7 allows only one channel to generate the trigger -> "
                                                  "changed to channel B");
                            m_levelTrigChanMask = 2;
                        }
                        else
                        {
                            m_levelTrigChanMask = 3;
                        }
                        break;
                    case 3:   // ch D
                    case 4:   // ch A+B
                    case 5:   // ch C+D
                    case 6:   // ch A+B+C+D
                        ADQNDS_MSG_INFOLOG_PV("INFO: Device has only two channels -> changed to channel B");
                        m_levelTrigChanMask = 2;
                        break;
                }
            }

            if (m_chanCnt == 4)
            {
                switch (m_levelTrigChan)
                {
                    case 0:   // ch A
                        m_levelTrigChanMask = 1;
                        break;
                    case 1:   // ch B
                        m_levelTrigChanMask = 2;
                        break;
                    case 2:   // ch C
                        m_levelTrigChanMask = 4;
                        break;
                    case 3:   // ch D
                        m_levelTrigChanMask = 8;
                        break;
                    case 4:   // ch A+B
                        m_levelTrigChanMask = 3;
                        break;
                    case 5:   // ch C+D
                        m_levelTrigChanMask = 12;
                        break;
                    case 6:   // ch A+B+C+D
                        m_levelTrigChanMask = 15;
                        break;
                }
            }

            m_levelTrigChanPV.push(now, m_levelTrigChan);

            status = m_adqInterface->SetLvlTrigChannel(m_levelTrigChanMask);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetLvlTrigChannel failed.");

            if (m_adqType == 714 || m_adqType == 14)
                m_levelTrigChanMask = m_adqInterface->GetLvlTrigChannel();

            m_levelTrigChanMaskPV.push(now, m_levelTrigChanMask);
        }
    }

    if (m_trigMode == 3)   // Internal trigger
    {
        if (m_internTrigHighSampChanged || m_internTrigLowSampChanged)
        {
            m_internTrigHighSampChanged = false;
            m_internTrigLowSampChanged = false;

            if ((m_internTrigHighSamp < 4) || (m_internTrigLowSamp < 4))
            {
                m_internTrigHighSamp = 4;
                m_internTrigLowSamp = 4;
                ADQNDS_MSG_INFOLOG_PV("INFO: Sample length can't be less than 4.");
            }

            status = m_adqInterface->SetInternalTriggerHighLow(m_internTrigHighSamp, m_internTrigLowSamp);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetInternalTriggerHighLow failed.");

            m_internTrigHighSampPV.push(now, m_internTrigHighSamp);
            m_internTrigLowSampPV.push(now, m_internTrigLowSamp);
        }

        if (m_internTrigFreqChanged)
        {
            m_internTrigFreqChanged = false;

            if (m_internTrigFreq <= 0)
                m_internTrigFreq = 1;

            m_internTrigPeriod = 1000000000 / m_internTrigFreq;

            status = m_adqInterface->SetInternalTriggerPeriod(m_internTrigPeriod);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetInternalTriggerPeriod failed.");

            m_internTrigFreqPV.push(now, m_internTrigFreq);
        }

        if (m_internTrigEdgeChanged)
        {
            m_internTrigEdgeChanged = false;

            status = m_adqInterface->SetTriggerEdge(m_trigMode + 1, m_internTrigEdge);
            ADQNDS_MSG_WARNLOG_PV(status, "ERROR: SetTriggerEdge failed.");

            if (status)
            {
                unsigned int trigEdge = 0;
                status = m_adqInterface->GetTriggerEdge(m_trigMode + 1, &trigEdge);
                ADQNDS_MSG_WARNLOG_PV(status, "ERROR: GetTriggerEdge failed.");
                if (status)
                {
                    m_internTrigEdge = trigEdge;
                    m_internTrigEdgePV.push(now, m_internTrigEdge);
                }
            }
        }
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

    // Check changes on channels
    for (auto const& channel : m_AIChannelsPtr)
    {
        channel->commitChanges(true, m_adqInterface, m_logMsgPV);
    }
}

/* Sets the device and its channels to state ON. Allows to apply changes to PVs.
 */
void ADQAIChannelGroup::onSwitchOn()
{
    commitChanges(true);

    // Enable all channels
    for (auto const& channel : m_AIChannelsPtr)
    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        channel->setState(nds::state_t::on);
        channel->commitChanges(true, m_adqInterface, m_logMsgPV);
    }
}

/* Sets the device and its channels to state OFF. Disables applying changes to PVs.
 */
void ADQAIChannelGroup::onSwitchOff()
{
    commitChanges();

    // Disable all channels
    for (auto const& channel : m_AIChannelsPtr)
    {
        channel->setState(nds::state_t::off);
    }
}

/* Sets the device and its channels to state RUNNING. 
 * Starts a chosen data acquisition in a thread.
 */
void ADQAIChannelGroup::onStart()
{
    for (auto const& channel : m_AIChannelsPtr)
    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        channel->setState(nds::state_t::running);
        channel->commitChanges(true, m_adqInterface, m_logMsgPV);
    }

    m_stopDaq = false;
    
    ADQNDS_MSG_INFOLOG_PV("INFO: Starting the acquisition...");

    if (m_daqMode == 0)
        m_daqThread = m_node.runInThread("DAQ-MultiRecord", std::bind(&ADQAIChannelGroup::daqMultiRecord, this));
    if (m_daqMode == 1)
        m_daqThread = m_node.runInThread("DAQ-ContinStream", std::bind(&ADQAIChannelGroup::daqContinStream, this));
    if (m_daqMode == 2)
        m_daqThread = m_node.runInThread("DAQ-TrigStream", std::bind(&ADQAIChannelGroup::daqTrigStream, this));
    if (m_daqMode == 3)
        m_daqThread = m_node.runInThread("DAQ-RawStream", std::bind(&ADQAIChannelGroup::daqRawStream, this));
}

/* Sets the device and its channels to state ON.
 * Stops data acquisition and joins DAQ thread.
 */
void ADQAIChannelGroup::onStop()
{
    m_stopDaq = true;
    m_daqThread.join();

    commitChanges();

    for (auto const& channel : m_AIChannelsPtr)
    {
        channel->setState(nds::state_t::on);
    }
}

/* Not supported.
 */
void ADQAIChannelGroup::recover()
{
    throw nds::StateMachineRollBack("INFO: Cannot recover");
}

/* Allows changing the device's state.
 */
bool ADQAIChannelGroup::allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal)
{
    UNUSED(currentLocal);
    UNUSED(currentGlobal);
    UNUSED(nextLocal);
    return true;
}

/* Time snap of the starting time, used in Triggered and Continuous Streaming.
 */
static struct timespec tsref;
void timerStart(void)
{
    if (clock_gettime(CLOCK_REALTIME, &tsref) < 0)
    {
        std::cout << "WARNING: Failed to start timer." << std::endl;
        return;
    }
}

/* Returns time in ms that is spend since the timerStart, used in Triggered and Continuous Streaming.
 */
unsigned int timerSpentTimeMs(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0)
    {
        std::cout << "WARNING: Failed to get system time." << std::endl;
        return -1;
    }
    return (unsigned int)((int)(ts.tv_sec - tsref.tv_sec) * 1000 + (int)(ts.tv_nsec - tsref.tv_nsec) / 1000000);
}

/* Data acquisition method for triggered streaming
 */
void ADQAIChannelGroup::daqTrigStream()
{
    int status = 0;
    unsigned int bufferSize;
    unsigned int recordsSumCnt = 0;
    unsigned int buffersFilled;
    unsigned int recordsDoneCnt[CHANNEL_COUNT_MAX];
    unsigned int samplesAddedCnt[CHANNEL_COUNT_MAX];
    unsigned int headersAdded[CHANNEL_COUNT_MAX];
    unsigned int samplesLeftoverSamplesCnt[CHANNEL_COUNT_MAX];
    unsigned int headerStatus[CHANNEL_COUNT_MAX];
    unsigned int headersDoneCnt = 0;
    unsigned int samplesRemainCnt = 0;
    unsigned int recordsDoneTotalCnt = 0;
    bool recordsInTotalCheck = 0;

    if (m_adqType == 714 || m_adqType == 14)
    {
        bufferSize = BUFFERSIZE_ADQ14;
    }
    if (m_adqType == 7)
    {
        bufferSize = BUFFERSIZE_ADQ7;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        m_daqStreamHeaders[chan] = NULL;
        m_daqDataBuffer[chan] = NULL;
        m_daqLeftoverSamples[chan] = NULL;

        recordsDoneCnt[chan] = 0;
        samplesAddedCnt[chan] = 0;
        headersAdded[chan] = 0;
        samplesLeftoverSamplesCnt[chan] = 0;
        headerStatus[chan] = 0;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (!((1 << chan) & m_chanMask))
            continue;

        m_daqDataBuffer[chan] = (short*)malloc((size_t)bufferSize);
        if (!m_daqDataBuffer[chan])
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: Failed to allocate memory for target buffers.");
        }

        m_daqStreamHeaders[chan] = (streamingHeader_t*)malloc((size_t)bufferSize);
        if (!m_daqStreamHeaders[chan])
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: Failed to allocate memory for target headers.");
        }

        m_daqLeftoverSamples[chan] = (short int*)malloc((size_t)(sizeof(short) * m_sampleCnt));
        if (!m_daqLeftoverSamples[chan])
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: Failed to allocate memory for target extradata.");
        }
    }

    // Sum each channel's number of records to collect specified by the user
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (!((1 << chan) & m_chanMask))
            continue;
        recordsSumCnt += m_recordCnt;
    }

    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->TriggeredStreamingSetup(m_recordCnt, m_sampleCnt, m_preTrigSamp, m_trigHoldOffSamp, m_chanMask);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: TriggeredStreamingSetup failed.");

        status = m_adqInterface->SetStreamStatus(1);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SetStreamStatus to 1 (Stream enabled) failed.");

        status = m_adqInterface->SetTransferBuffers(CHANNEL_COUNT_MAX, bufferSize);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SetTransferBuffers failed.");

        status = m_adqInterface->StopStreaming();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: StopStreaming failed.");

        status = m_adqInterface->StartStreaming();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: StartStreaming failed.");

        if (m_trigMode == 0)
        {
            for (int i = 0; i < m_recordCnt; ++i)
            {
                status = m_adqInterface->SWTrig();
                ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SWTrig failed.");
            }
        }
    }

    /* Data acquisition loop.
     * During data collection each record is sent to data PV separately.
     */
    do
    {
        // Mutex is used to save digitizer's API library from interruptions during infinite collection
        {
            std::lock_guard<std::mutex> lock(m_adqDevMutex);

            buffersFilled = 0;

            status = m_adqInterface->GetStreamOverflow();
            if (status)
            {
                status = 0;
                ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "WARNING: Streaming overflow detected. Stopping the data acquisition.");
            }

            status = m_adqInterface->GetTransferBufferStatus(&buffersFilled);
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: GetTransferBufferStatus failed.");

            // Poll for the transfer buffer status as long as the timeout has not been
            // reached and no buffers have been filled.
            while (!(m_stateMachine.getLocalState() == nds::state_t::stopping) && !buffersFilled)
            {
                // Mark the loop start
                timerStart();
                while (!buffersFilled && (timerSpentTimeMs() < (unsigned)m_timeout) && !(m_stateMachine.getLocalState() == nds::state_t::stopping))
                {
                    status = m_adqInterface->GetTransferBufferStatus(&buffersFilled);
                    ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: GetTransferBufferStatus failed.");
                    // Sleep to avoid loading the processor too much
                    SLEEP(10);
                }

                // Timeout reached, flush the transfer buffer to receive data
                if (!buffersFilled)
                {
                    ADQNDS_MSG_INFOLOG_PV("INFO: Timeout, flushing DMA...");

                    status = m_adqInterface->FlushDMA();
                    ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: FlushDMA failed.");
                }
            }
            
            if (m_stateMachine.getLocalState() == nds::state_t::stopping)
            {
                ADQNDS_MSG_INFOLOG_PV("INFO: Data acquisition was stopped.");
                goto finish;
            }
            
            ADQNDS_MSG_INFOLOG_PV("INFO: Receiving data...");
            status = m_adqInterface->GetDataStreaming((void**)m_daqDataBuffer, (void**)m_daqStreamHeaders, m_chanMask, samplesAddedCnt, headersAdded, headerStatus);
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: GetDataStreaming failed.");
        }

        // Go through each channel's m_daqDataBuffer data: check on incomplete records, add samples of incomplete records to the next records
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            if (!((1 << chan) & m_chanMask))
                continue;

            if (headersAdded[chan] > 0)
            {
                if (headerStatus[chan])
                {
                    headersDoneCnt = headersAdded[chan];
                }
                else
                {
                    // One incomplete record in the buffer (header is copied to the front
                    // of the buffer later)
                    headersDoneCnt = headersAdded[chan] - 1;
                }

                // If there is at least one complete header
                recordsDoneCnt[chan] += headersDoneCnt;
            }

            // Go through added samples
            if (samplesAddedCnt[chan] > 0)
            {
                samplesRemainCnt = samplesAddedCnt[chan];

                // Handle incomplete record at the start of the buffer
                if (samplesLeftoverSamplesCnt[chan] > 0)
                {
                    if (headersDoneCnt == 0)
                    {
                        // There is not enough data in the m_daqDataBuffer to complete the record.
                        // Add all the samples to the m_daqLeftoverSamples buffer.
                        std::memcpy(&m_daqLeftoverSamples[chan][samplesLeftoverSamplesCnt[chan]],
                                    m_daqDataBuffer[chan],
                                    sizeof(short) * samplesAddedCnt[chan]);
                        samplesRemainCnt -= samplesAddedCnt[chan];
                        samplesLeftoverSamplesCnt[chan] += samplesAddedCnt[chan];
                    }
                    else
                    {
                        // There is enough data in the transfer buffer to complete the record

                        m_AIChannelsPtr[chan]->readData(m_daqLeftoverSamples[chan], samplesLeftoverSamplesCnt[chan]);
                        m_AIChannelsPtr[chan]->readData(m_daqDataBuffer[chan],
                                                        (m_daqStreamHeaders[chan][0].recordLength - samplesLeftoverSamplesCnt[chan]));

                        samplesRemainCnt -= m_daqStreamHeaders[chan][0].recordLength - samplesLeftoverSamplesCnt[chan];
                        samplesLeftoverSamplesCnt[chan] = 0;
                    }
                }
                else
                {
                    if (headersDoneCnt == 0)
                    {
                        // The samples in the transfer buffer begin a new record, this
                        // record is incomplete.
                        std::memcpy(m_daqLeftoverSamples[chan], m_daqDataBuffer[chan], sizeof(short) * samplesAddedCnt[chan]);
                        samplesRemainCnt -= samplesAddedCnt[chan];
                        samplesLeftoverSamplesCnt[chan] = samplesAddedCnt[chan];
                    }
                    else
                    {
                        m_AIChannelsPtr[chan]->readData(m_daqDataBuffer[chan], m_daqStreamHeaders[chan][0].recordLength);

                        samplesRemainCnt -= m_daqStreamHeaders[chan][0].recordLength;
                    }
                }
                // At this point: the first record in m_daqDataBuffer or the entire
                // transfer buffer has been parsed.

                // Loop through complete records inside m_daqDataBuffer.
                // If infinite data collection is set, then read each completed record from each channel's m_daqDataBuffer
                for (unsigned int i = 1; i < headersDoneCnt; ++i)
                {
                    m_AIChannelsPtr[chan]->readData((&m_daqDataBuffer[chan][samplesAddedCnt[chan] - samplesRemainCnt]),
                                                    m_daqStreamHeaders[chan][i].recordLength);

                    samplesRemainCnt -= m_daqStreamHeaders[chan][i].recordLength;
                }

                // If there is an incomplete record at the end of m_daqDataBuffer, copy its header to m_daqStreamHeaders
                // and its samples to m_daqLeftoverSamples buffer.
                if (samplesRemainCnt > 0)
                {
                    std::memcpy(m_daqStreamHeaders[chan], &m_daqStreamHeaders[chan][headersDoneCnt], sizeof(streamingHeader_t));

                    std::memcpy(m_daqLeftoverSamples[chan],
                                &m_daqDataBuffer[chan][samplesAddedCnt[chan] - samplesRemainCnt],
                                sizeof(short) * samplesRemainCnt);
                    samplesLeftoverSamplesCnt[chan] = samplesRemainCnt;
                    samplesRemainCnt = 0;
                }
            }
        }

        // Update recordsDoneTotalCnt
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            recordsDoneTotalCnt += recordsDoneCnt[chan];
        }

        // Determine if collection is completed or stopped
        recordsInTotalCheck = (recordsDoneTotalCnt >= recordsSumCnt);

    } while (!recordsInTotalCheck && !(m_stateMachine.getLocalState() == nds::state_t::stopping));

finish:
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);

    status = m_adqInterface->SetStreamStatus(0);
    ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetStreamStatus to 0 (Stream disabled) failed.");

    status = m_adqInterface->StopStreaming();
    ADQNDS_MSG_WARNLOG_PV(status, "WARNING: StopStreaming failed.");
}
    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqDataBuffer[chan])
        {
            free(m_daqDataBuffer[chan]);
            m_daqDataBuffer[chan] = NULL;
        }
        if (m_daqStreamHeaders[chan])
        {
            free(m_daqStreamHeaders[chan]);
            m_daqStreamHeaders[chan] = NULL;
        }
        if (m_daqLeftoverSamples[chan])
        {
            free(m_daqLeftoverSamples[chan]);
            m_daqLeftoverSamples[chan] = NULL;
        }
    }

    try
    {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error)
    {
        // We are probably already in "stopping" state, no need to panic...
    }

    commitChanges(true);
}

/* Data acquisition method for Multi-Record
*/
void ADQAIChannelGroup::daqMultiRecord()
{
    int trigged, status;
    void* daqVoidBuffers[CHANNEL_COUNT_MAX];
    unsigned int streamCompleted = 0;

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        m_daqDataBuffer[chan] = NULL;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        m_daqDataBuffer[chan] = (short*)calloc(m_sampleCntTotal, sizeof(short));
    }

    // Create a pointer array containing the data buffer pointers
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        daqVoidBuffers[chan] = (void*)m_daqDataBuffer[chan];
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqDataBuffer[chan] == NULL)
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: Failed to allocate memory for target buffers.");
        }
    }

    while (!(m_stateMachine.getLocalState() == nds::state_t::stopping) && !streamCompleted)
    {
    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->MultiRecordSetChannelMask(m_chanMask);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: MultiRecordSetChannelMask failed.");

        status = m_adqInterface->MultiRecordSetup(m_recordCnt, m_sampleCnt);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: MultiRecordSetup failed.");

        status = m_adqInterface->DisarmTrigger();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: DisarmTrigger failed.");

        status = m_adqInterface->ArmTrigger();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: ArmTrigger failed.");
            
        ADQNDS_MSG_INFOLOG_PV("Triggering...");

        if (m_trigMode == 0)   // SW trigger
        {
            do
            {
                trigged = m_adqInterface->GetAcquiredAll();
                for (int i = 0; i < m_recordCnt; ++i)
                {
                    status = m_adqInterface->SWTrig();
                    ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SWTrig failed.");
                }
            } while (trigged == 0);
        }
        else
        {
            do
            {
                trigged = m_adqInterface->GetAcquiredAll();
            } while ((trigged == 0) && !(m_stateMachine.getLocalState() == nds::state_t::stopping));
        }
            
        if (m_stateMachine.getLocalState() == nds::state_t::stopping)
        {
            ADQNDS_MSG_INFOLOG_PV("INFO: Data acquisition was stopped.");
            goto finish;
        }
            
            std::ostringstream textTmp;
            unsigned int acqRecTotal = m_adqInterface->GetAcquiredRecords();
            textTmp << "INFO: GetAcquiredRecords: " << acqRecTotal;
            std::string textForPV(textTmp.str());
            ADQNDS_MSG_INFOLOG_PV(textForPV);

        status = m_adqInterface->GetStreamOverflow();
        if (status)
        {
            status = 0;
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "WARNING: Streaming overflow detected. Stopping the data acquisition.");
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->GetData(daqVoidBuffers, m_sampleCntTotal, sizeof(short), 0, m_recordCntCollect, m_chanMask, 0, m_sampleCnt, ADQ_TRANSFER_MODE_NORMAL);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: GetData failed.");
    }

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        // Read buffers by each channel and send them to DATA PVs
        m_AIChannelsPtr[chan]->readData((short*)daqVoidBuffers[chan], m_sampleCntTotal);
    }
    
    streamCompleted = 1;
    }
    
finish:
    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->DisarmTrigger();
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: DisarmTrigger failed.");
        m_adqInterface->MultiRecordClose();
    }

    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqDataBuffer[chan])
        {
            free(m_daqDataBuffer[chan]);
            m_daqDataBuffer[chan] = NULL;
        }
    }

    try
    {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error)
    {
        // We are probably already in "stopping" state, no need to panic...
    }

    commitChanges(true);
}

/* Data acquisition method for continuous streaming
*/
void ADQAIChannelGroup::daqContinStream()
{
    int status;
    unsigned int bufferSize, buffersFilled;
    double elapsedSeconds;
    int bufferStatusLoops;
    unsigned int samplesAdded[CHANNEL_COUNT_MAX];
    unsigned int headersAdded[CHANNEL_COUNT_MAX];
    unsigned int headerStatus[CHANNEL_COUNT_MAX];
    unsigned int samplesAddedTotal;
    unsigned int streamCompleted = 0;

    if (m_adqType == 714 || m_adqType == 14)
    {
        bufferSize = BUFFERSIZE_ADQ14;
    }
    if (m_adqType == 7)
    {
        bufferSize = BUFFERSIZE_ADQ7;
    }

    for (unsigned int chan = 0; chan < m_chanCnt; chan++)
    {
        m_daqDataBuffer[chan] = NULL;
        m_daqStreamHeaders[chan] = NULL;
        if (m_chanMask & (1 << chan))
        {
            m_daqDataBuffer[chan] = (short*)malloc(bufferSize*sizeof(char));
            m_daqStreamHeaders[chan] = (streamingHeader_t*)malloc(10*sizeof(uint32_t));
        }
    }

    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->StopStreaming();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: StopStreaming failed.");

        status = m_adqInterface->ContinuousStreamingSetup(m_chanMask);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: ContinuousStreamingSetup failed.");
        
        status = m_adqInterface->SetTransferBuffers(CHANNEL_COUNT_MAX, bufferSize);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SetTransferBuffers failed.");

        // Start streaming
        samplesAddedTotal = 0;

        status = m_adqInterface->StartStreaming();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: StartStreaming failed.");

        timerStart();
        
        if (m_trigMode == 0)
        {
            status = m_adqInterface->SWTrig();
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SWTrig failed.");
        }
        
        while (!streamCompleted)
        {
            bufferStatusLoops = 0;
            buffersFilled = 0;

            while (buffersFilled == 0 && !(m_stateMachine.getLocalState() == nds::state_t::stopping))
            {                       
                status = m_adqInterface->GetTransferBufferStatus(&buffersFilled);
                ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: GetTransferBufferStatus failed.");

                if (buffersFilled == 0)
                {
                    SLEEP(10);
                    bufferStatusLoops++;

                    if (bufferStatusLoops > 2000)
                    {
                        // If the DMA transfer is taking too long, we should flush the DMA buffer before it times out. The timeout defaults to 60 seconds.
                        ADQNDS_MSG_INFOLOG_PV("INFO: Timeout, flushing DMA...");
                        status = m_adqInterface->FlushDMA();
                        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: FlushDMA failed.");
                    }
                }
            }
            
            if (m_stateMachine.getLocalState() == nds::state_t::stopping)
            {
                ADQNDS_MSG_INFOLOG_PV("INFO: Data acquisition was stopped.");
                goto finish;
            }

            for (unsigned int buf = 0; buf < buffersFilled; buf++)
            {
                ADQNDS_MSG_INFOLOG_PV("INFO: Receiving data...");
                status = m_adqInterface->GetDataStreaming((void**)m_daqDataBuffer,
                                                          (void**)m_daqStreamHeaders,
                                                          m_chanMask,
                                                          samplesAdded,
                                                          headersAdded,
                                                          headerStatus);
                ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: GetDataStreaming failed.");

                for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
                {
                    samplesAddedTotal += samplesAdded[chan];
                }

                // Read data from each channel's buffer
                for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
                {
                    int32_t sampleCnt = samplesAdded[chan];
                    m_AIChannelsPtr[chan]->readData(m_daqDataBuffer[chan], sampleCnt);
                }
            }
            
            elapsedSeconds = timerSpentTimeMs() / 1000;

            if (elapsedSeconds > m_streamTime)
            {
                streamCompleted = 1;
                ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished due to achieved target stream time.");
            }

            status = m_adqInterface->GetStreamOverflow();
            if (status)
            {
                ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "WARNING: Streaming overflow detected. Stopping the data acquisition.");
            }
        }
    }

finish:
    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->StopStreaming();
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: StopStreaming failed.");
    }

    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqDataBuffer[chan])
        {
            free(m_daqDataBuffer[chan]);
            m_daqDataBuffer[chan] = NULL;
        }
        if (m_daqStreamHeaders[chan])
        {
            free(m_daqStreamHeaders[chan]);
            m_daqStreamHeaders[chan] = NULL;
        }
    }

    try
    {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error)
    {
        // We are probably already in "stopping" state, no need to panic...
    }

    commitChanges(true);
}

/* Data acquisition method for raw streaming.
 * Works per channel.
 */
void ADQAIChannelGroup::daqRawStream()
{
    int status;
    unsigned int bufferSize, buffersFilled;
    unsigned int m_sampleCntCollect;

    if (m_adqType == 714 || m_adqType == 14)
    {
        bufferSize = BUFFERSIZE_ADQ14;
    }
    if (m_adqType == 7)
    {
        bufferSize = BUFFERSIZE_ADQ7;
    }

    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->SetTransferBuffers(CHANNEL_COUNT_MAX, bufferSize);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SetTransferBuffers failed.");

        status = m_adqInterface->SetStreamStatus(1);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SetStreamStatus to 1 (Stream enabled) failed.");
        status = m_adqInterface->SetStreamConfig(2, 1);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SetStreamConfig to 2-1 (Raw without headers) failed.");
        status = m_adqInterface->SetStreamConfig(3, m_chanInt);
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: SetStreamConfig to 3-x (channel mask) failed.");
    }

    ADQNDS_MSG_INFOLOG_PV("INFO: Receving data...");

    // Created temporary target for streaming data
    m_daqRawDataBuffer = NULL;

    // Allocate temporary buffer for streaming data
    m_daqRawDataBuffer = (signed short*)malloc(bufferSize * sizeof(signed short));

    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);
        status = m_adqInterface->StopStreaming();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: StopStreaming failed.");

        status = m_adqInterface->StartStreaming();
        ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: StartStreaming failed.");
    }

    m_sampleCntCollect = bufferSize;

    while (!(m_stateMachine.getLocalState() == nds::state_t::stopping) && (m_sampleCntCollect > 0))
    {
        unsigned int bufferSampCnt;
        {
            std::lock_guard<std::mutex> lock(m_adqDevMutex);
            do
            {
                status = m_adqInterface->GetTransferBufferStatus(&buffersFilled);
                ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: GetTransferBufferStatus failed.");

            } while ((buffersFilled == 0) && (status));

            status = m_adqInterface->CollectDataNextPage();
            ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "ERROR: CollectDataNextPage failed.");

            bufferSampCnt = MIN(m_adqInterface->GetSamplesPerPage(), m_sampleCntCollect);
            ndsInfoStream(m_node) << "GetSamplesPerPage " << m_adqInterface->GetSamplesPerPage() << std::endl;

            status = m_adqInterface->GetStreamOverflow();
            if (status)
            {
                status = 0;
                ADQNDS_MSG_ERRLOG_PV_GOTO_FINISH(status, "WARNING: Streaming overflow detected. Stopping the data acquisition.");
            }
        }

        // Buffer all data in RAM before writing to disk
        // Data format is set to 16 bits, so buffer size is Samples*2 bytes
        memcpy((void*)&m_daqRawDataBuffer[bufferSize - m_sampleCntCollect],
               m_adqInterface->GetPtrStream(),
               bufferSampCnt * sizeof(signed short));
        m_sampleCntCollect -= bufferSampCnt;
    }

    m_sampleCntCollect = bufferSize;

    ADQNDS_MSG_INFOLOG_PV("INFO: Writing data to PVs...");

    // Read buffer and send its data to DATA PV
    m_AIChannelsPtr[m_chanActive]->readData(m_daqRawDataBuffer, m_sampleCntCollect);

finish:
    {
        std::lock_guard<std::mutex> lock(m_adqDevMutex);

        // Reset settings from Raw Streaming to normal streaming
        status = m_adqInterface->SetStreamStatus(0);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetStreamStatus to 0 (Stream disabled) failed.");
        status = m_adqInterface->SetStreamConfig(2, 0);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetStreamConfig to 2-1 (With headers) failed.");
        status = m_adqInterface->SetStreamConfig(3, 0);
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: SetStreamConfig to 3-x (channel mask) failed.");

        status = m_adqInterface->StopStreaming();
        ADQNDS_MSG_WARNLOG_PV(status, "WARNING: StopStreaming failed.");
    }

    ADQNDS_MSG_INFOLOG_PV("INFO: Acquisition finished.");

    if (m_daqRawDataBuffer)
    {
        free(m_daqRawDataBuffer);
        m_daqRawDataBuffer = NULL;
    }

    try
    {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error)
    {
        // We are probably already in "stopping", no need to panic...
    }

    commitChanges(true);
}

ADQAIChannelGroup::~ADQAIChannelGroup()
{
    ndsInfoStream(m_node) << "Stopping the application..." << std::endl;
    
    if (m_stateMachine.getLocalState() == nds::state_t::running)
    {     
        ndsInfoStream(m_node) << "Stopping the acquisition..." << std::endl;
        //m_stopDaq = true;
        //m_stateMachine.setState(nds::state_t::on);
        
        //int status = m_adqInterface->StopStreaming();
        //ndsInfoStream(m_node) << status << " StopStreaming" << std::endl;
        
        //m_stateMachine.setState(nds::state_t::on);
        //SLEEP(200);
        //m_daqThread.join();

        //for (auto const& channel : m_AIChannelsPtr)
        //{
        //    channel->setState(nds::state_t::on);
        //}
        //m_stateMachine.setState(nds::state_t::on);
    }
    
    ndsInfoStream(m_node) << "Freeing buffers..." << std::endl;

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (m_daqDataBuffer[chan])
        {
            free(m_daqDataBuffer[chan]);
            m_daqDataBuffer[chan] = NULL;
        }
        if (m_daqStreamHeaders[chan])
        {
            free(m_daqStreamHeaders[chan]);
            m_daqStreamHeaders[chan] = NULL;
        }
        if (m_daqLeftoverSamples[chan])
        {
            free(m_daqLeftoverSamples[chan]);
            m_daqLeftoverSamples[chan] = NULL;
        }
    }

    if (m_daqRawDataBuffer)
    {
        free(m_daqRawDataBuffer);
        m_daqRawDataBuffer = NULL;
    }
}
