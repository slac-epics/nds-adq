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

//// urojec L3: camelCase
//// urojec L2:

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
    m_sampleCntMaxPV(nds::PVDelegateIn<std::int32_t>("SampCntMax-RB", std::bind(&ADQAIChannelGroup::getSampleCntMax,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleCntPV(nds::PVDelegateIn<std::int32_t>("SampCnt-RB", std::bind(&ADQAIChannelGroup::getSampleCnt,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleCntTotalPV(nds::PVDelegateIn<std::int32_t>("SampCntTotal-RB", std::bind(&ADQAIChannelGroup::getSamplesTotal,
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
    m_logMsgPV(nds::PVDelegateIn<std::string>("LogMsg", std::bind(&ADQAIChannelGroup::getLogMsg,
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

    // PVs for data acquisition modes
    nds::enumerationStrings_t daqModeList = { "Multi-Record", "Continuous streaming", "Triggered streaming" };
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

    m_sampleCntTotalPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleCntTotalPV);

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

    // PV for error/warning/info messages
    m_logMsgPV.setScanType(nds::scanType_t::interrupt);
    m_logMsgPV.setMaxElements(512);
    m_node.addChild(m_logMsgPV);

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

void ADQAIChannelGroup::getSampleCntMax(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_sampleCntMax;
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

void ADQAIChannelGroup::getSamplesTotal(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_sampleCntTotal;
}

void ADQAIChannelGroup::commitChanges(bool calledFromDaqThread)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    unsigned int success;

    /* Allow changes to parameters when device is ON/STOPPING/INITIALISING states.
     * Do not apply changes when device is on acquisition state.
     */
    if (!calledFromDaqThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) {
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
        const char chan_char[5] = "ABCD";
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

        if (m_recordCnt != -1)
        {
            m_recordCntPV.push(now, m_recordCnt);
            success = m_adqDevPtr->GetMaxNofSamplesFromNofRecords(m_recordCnt, &sampleCntMax);
            ADQNDS_MSG_WARNLOG(success, "Couldn't get the MAX number of samples (set number of records).");
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
                
                m_sampleCntPV.push(now, m_sampleCnt);

                m_sampleCntTotal = m_sampleCnt * m_recordCnt;
                m_sampleCntTotalPV.push(now, m_sampleCntTotal);
            }
        }
        else
        {
            m_sampleCntTotal = m_sampleCnt * m_recordCnt;

            m_recordCntPV.push(now, m_recordCnt);
            m_sampleCntPV.push(now, m_sampleCnt);
            m_sampleCntTotalPV.push(now, m_sampleCntTotal);
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

    if (m_trigFreqChanged)
    {
        m_trigFreqChanged = false;
        m_trigFreqPV.push(now, m_trigFreq);
        m_trigPeriod = 1000000000 / m_trigFreq;
    }

    /* This block processes changes applied to ADQ specific PVs
     */

    if (m_chanActiveChanged)     // ADQ Specific setting
    {
        m_chanActiveChanged = false;

        if (m_daqMode == 0) // Multi-Record
        {
            if ((m_adqDevPtr->GetADQType()) == 714 || (m_adqDevPtr->GetADQType()) == 14)
            {
                success = m_adqDevPtr->MultiRecordSetChannelMask(m_chanBits);
                ADQNDS_MSG_WARNLOG(success, "WARNING: MultiRecordSetChannelMask failed.");
            }
        }
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
}

void ADQAIChannelGroup::onStop()
{
    m_stopDaq = true;
    m_adqDevPtr->StopStreaming();

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
    throw nds::StateMachineRollBack("Cannot recover");
}

bool ADQAIChannelGroup::allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal)
{
    return true;
}

void ADQAIChannelGroup::daqTrigStream()
{
    unsigned int success;
    unsigned int nofrecords_sum;

    unsigned int m_tr_nofbuf = 8;
    unsigned int m_tr_bufsize;

    streamingHeader_t* tr_headers[4] = { NULL, NULL, NULL, NULL };
    short* tr_buffers[4] = { NULL, NULL, NULL, NULL };
    short* tr_extra[4] = { NULL, NULL, NULL, NULL };
    short* record_data;
    //short* all_records_data;
    unsigned int pretrig_samples = 0;
    unsigned int holdoff_samples = 0;
    unsigned int buffers_filled = 0;
    unsigned int records_completed[4] = { 0, 0, 0, 0 };
    unsigned int samples_added[4] = { 0, 0, 0, 0 };
    unsigned int headers_added[4] = { 0, 0, 0, 0 };
    unsigned int samples_extradata[4] = { 0, 0, 0, 0 };
    unsigned int header_status[4] = { 0, 0, 0, 0 };
    unsigned int headers_done = 0;
    unsigned int timeout_ms = 1000;
    unsigned int received_all_records = 0;
    unsigned int samples_remaining;
    unsigned int nof_received_records_sum = 0;
    unsigned int nof_records_sum = 0;

    int adqType = m_adqDevPtr->GetADQType();
    if (adqType == 714 || adqType == 14)
    {
        m_tr_bufsize = 512 * 1024;
    }
    if (adqType == 7)
    {
        m_tr_bufsize = 256 * 1024;
    }

    // Convert chanMask from std::string to unsigned char for certain ADQAPI functions
    unsigned char chanMaskChar = *m_chanMask.c_str();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        if (!((1 << chan) & chanMaskChar))
            continue;
        tr_buffers[chan] = (short int*)malloc((size_t)m_tr_bufsize);
        if (!tr_buffers[chan])
        {
            ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target buffers." << std::endl;
            goto finish;
        }
        else
        {
            tr_headers[chan] = (streamingHeader_t*)malloc((size_t)m_tr_bufsize);
            if (!tr_headers[chan])
            {
                ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target buffers." << std::endl;
                goto finish;
            }
            else
            {
                tr_extra[chan] = (short int*)malloc((size_t)(sizeof(short)*m_sampleCnt));
                if (!tr_extra[chan])
                {
                    ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target extradata." << std::endl;
                    goto finish;
                }
            }
        }
    }

    if (adqType == 714 || adqType == 14) // Only ADQ14 function
    {
        // Allocate memory for record data (used for ProcessRecord function template)
        record_data = (short int*)malloc((size_t)(sizeof(short)*m_sampleCnt));
    }

    if (!record_data)
    {
        ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for ProcessRecord." << std::endl;
        goto finish;
    }

    // Compute the sum of the number of records specified by the user
    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        nofrecords_sum += m_recordCnt;
    }

    switch (m_trigMode)
    {
    case 0: // SW trigger
        ADQNDS_MSG_INFOLOG("Issuing Software trigger... ");
        break;
    case 1: // External trigger  ----- need to investigate the API doc to develop this method
        ADQNDS_MSG_INFOLOG("Issuing External trigger... ");
        break;
    case 2: // Level trigger ----- need to investigate the API doc to develop this method
        ADQNDS_MSG_INFOLOG("Issuing Level trigger... ");
       
        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigEdge failed.");
        
        success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigLevel failed.");

        success = m_adqDevPtr->SetLvlTrigChannel(m_trigChan);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigChannel failed.");
        break;
    case 3: // Internal trigger ----- need to investigate the API doc to develop this method
        ADQNDS_MSG_INFOLOG("Issuing Internal trigger... ");
        success = m_adqDevPtr->SetInternalTriggerPeriod(m_trigFreq); 
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetInternalTriggerPeriod failed.");
        break;
    }

    success = m_adqDevPtr->TriggeredStreamingSetup(m_recordCnt, m_sampleCnt, pretrig_samples, holdoff_samples, chanMaskChar);
    ADQNDS_MSG_ERRLOG(success, "ERROR: TriggeredStreamingSetup failed.");

    if (adqType == 7) // Probably not needed, it is an Internal function (as mentioned in ADQAPI document)
    {
        success = m_adqDevPtr->ResetWriteCountMax();
        ADQNDS_MSG_WARNLOG(success, "WARNING: ResetWriteCountMax failed.");
    }

    success = m_adqDevPtr->SetStreamStatus(1);
    ADQNDS_MSG_ERRLOG(success, "ERROR: SetStreamStatus failed.");

    success = m_adqDevPtr->SetTransferBuffers(m_tr_nofbuf, m_tr_bufsize);
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
        buffers_filled = 0;

        success = m_adqDevPtr->GetStreamOverflow();
        if (success)
        {
            ndsErrorStream(m_node) << "ERROR: Streaming overflow detected." << std::endl;
            goto finish;
        }

        success = m_adqDevPtr->GetTransferBufferStatus(&buffers_filled);
        ADQNDS_MSG_ERRLOG(success, "ERROR: GetTransferBufferStatus failed.");

        // Poll for the transfer buffer status as long as the timeout has not been
        // reached and no buffers have been filled.
        while (!buffers_filled)
        {
            // Mark the loop start
            timerStart();
            while (!buffers_filled && (timerTimeMs() < timeout_ms))
            {
                success = m_adqDevPtr->GetTransferBufferStatus(&buffers_filled);
                ADQNDS_MSG_ERRLOG(success, "ERROR: GetTransferBufferStatus failed.");
                // Sleep to avoid loading the processor too much
                sleep(10);
            }

            // Timeout reached, flush the transfer buffer to receive data
            if (!buffers_filled)
            {
                ADQNDS_MSG_INFOLOG("Timeout, flushing DMA...");
                success = m_adqDevPtr->FlushDMA();
                ADQNDS_MSG_ERRLOG(success, "ERROR: FlushDMA failed.");
            }
        }

        ADQNDS_MSG_INFOLOG("Receiving data...");
        success = m_adqDevPtr->GetDataStreaming((void**)tr_buffers, (void**)tr_headers, chanMaskChar, samples_added, headers_added, header_status);
        ADQNDS_MSG_ERRLOG(success, "ERROR: GetDataStreaming failed.");

        // Parse data
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            if (!((1 << chan) & chanMaskChar))
                continue;

            if (headers_added[chan] > 0)
            {
                if (header_status[chan])
                {
                    headers_done = headers_added[chan];
                }
                else
                {
                    // One incomplete record in the buffer (header is copied to the front
                    // of the buffer later)
                    headers_done = headers_added[chan] - 1;
                }

                // If there is at least one complete header
                records_completed[chan] += headers_done;
            }

            // Parse  added samples
            if (samples_added[chan] > 0)
            {
                samples_remaining = samples_added[chan];

                // Handle incomplete record at the start of the buffer
                if (samples_extradata[chan] > 0)
                {
                    if (headers_done == 0)
                    {
                        // There is not enough data in the transfer buffer to complete
                        // the record. Add all the samples to the extradata buffer.
                        std::memcpy(&tr_extra[chan][samples_extradata[chan]], tr_buffers[chan], sizeof(short)*samples_added[chan]);
                        samples_remaining -= samples_added[chan];
                        samples_extradata[chan] += samples_added[chan];
                    }
                    else
                    {
                        // Move data to record_data
                        std::memcpy((void*)record_data, tr_extra[chan], sizeof(short)*samples_extradata[chan]);
                        std::memcpy((void*)(record_data + samples_extradata[chan]), tr_buffers[chan], sizeof(short)*(tr_headers[chan][0].recordLength - samples_extradata[chan]));

                        samples_remaining -= tr_headers[chan][0].recordLength - samples_extradata[chan];
                        samples_extradata[chan] = 0;

                        daqTrigStreamProcessRecord(record_data, &tr_headers[chan][0]);
                        ndsInfoStream(m_node) << "Completed record " << tr_headers[chan][0].recordNumber <<
                            " on channel " << chan << ", " << tr_headers[chan][0].recordLength << " samples." << std::endl;
                    }

                    m_AIChannelsPtr[chan]->readTrigStream(tr_buffers[chan], m_sampleCnt);
                }
                else
                {
                    if (headers_done == 0)
                    {
                        // The samples in the transfer buffer begin a new record, this
                        // record is incomplete.
                        std::memcpy(tr_extra[chan], tr_buffers[chan], sizeof(short)*samples_added[chan]);
                        samples_remaining -= samples_added[chan];
                        samples_extradata[chan] = samples_added[chan];
                    }
                    else
                    {

                        // Copy data to record buffer
                        std::memcpy((void*)record_data, tr_buffers[chan], sizeof(short)*tr_headers[chan][0].recordLength);
                        samples_remaining -= tr_headers[chan][0].recordLength;

                        daqTrigStreamProcessRecord(record_data, &tr_headers[chan][0]);
                        ndsInfoStream(m_node) << "Completed record " << tr_headers[chan][0].recordNumber <<
                            " on channel " << chan << ", " << tr_headers[chan][0].recordLength << " samples." << std::endl;
                    }

                    m_AIChannelsPtr[chan]->readTrigStream(tr_buffers[chan], m_sampleCnt);
                }
                // At this point: the first record in the transfer buffer or the entire
                // transfer buffer has been parsed.

                // Loop through complete records fully inside the buffer
                for (unsigned int i = 1; i < headers_done; ++i)
                {
                    // Copy data to record buffer
                    std::memcpy((void*)record_data, (&tr_buffers[chan][samples_added[chan] - samples_remaining]), sizeof(short)*tr_headers[chan][i].recordLength);

                    samples_remaining -= tr_headers[chan][i].recordLength;

                    daqTrigStreamProcessRecord(record_data, &tr_headers[chan][i]);
                    ndsInfoStream(m_node) << "Completed record " << tr_headers[chan][i].recordNumber <<
                        " on channel " << chan << ", " << tr_headers[chan][i].recordLength << " samples." << std::endl;

                    m_AIChannelsPtr[chan]->readTrigStream(tr_buffers[chan], m_sampleCnt);
                }

                if (samples_remaining > 0)
                {
                    // There is an incomplete record at the end of the transfer buffer
                    // Copy the incomplete header to the start of the target_headers buffer
                    std::memcpy(tr_headers[chan], &tr_headers[chan][headers_done], sizeof(streamingHeader_t));

                    // Copy any remaining samples to the target_buffers_extradata buffer,
                    // they belong to the incomplete record
                    std::memcpy(tr_extra[chan], &tr_buffers[chan][samples_added[chan] - samples_remaining], sizeof(short)*samples_remaining);
                    // printf("Incomplete at end of transfer buffer. %u samples.\n", samples_remaining);
                    // printf("Copying %u samples to the extradata buffer\n", samples_remaining);
                    samples_extradata[chan] = samples_remaining;
                    samples_remaining = 0;
                }
            }

            // Read buffers by each channel and send them to DATA PVs
            //tr_buffers_ptr = (double*)tr_buffers[ch];
            //m_AIChannelsPtr[chan]->readTrigStream(tr_buffers[chan], m_sampleCntTotal);
        }

        // Update received_all_records
        nof_received_records_sum = 0;
        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
        {
            nof_received_records_sum += records_completed[chan];
        }

        // Determine if collection is completed
        received_all_records = (nof_received_records_sum >= nof_records_sum);

        if (adqType == 7) // Probably not needed, it is an Internal function (as mentioned in ADQAPI document)
        {
            unsigned int* writeCntMax;
            success = m_adqDevPtr->GetWriteCountMax(writeCntMax);
            ADQNDS_MSG_WARNLOG(success, "WARNING: GetWriteCountMax failed.");
            if (success)
            {
                unsigned int* varTmp;
                *varTmp = 1024 * 1024;
                double writeCntMaxTmp = *writeCntMax * 128 / *varTmp;
                ndsInfoStream(m_node) << "Peak: " << writeCntMaxTmp << " MiB/n " << std::endl;
            }
        }
    } while (!received_all_records);

finish:
    ADQNDS_MSG_INFOLOG("Acquisition finished.");
    commitChanges(true);
    m_adqDevPtr->StopStreaming();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan) {
        if (tr_buffers[chan])
            free(tr_buffers[chan]);
        if (tr_headers[chan])
            free(tr_headers[chan]);
        if (tr_extra[chan])
            free(tr_extra[chan]);
    }

    try {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) {
        /* We are probably already in "stopping", no need to panic... */
    }
}

void ADQAIChannelGroup::daqTrigStreamProcessRecord(short* recordData, streamingHeader_t* recordHeader)  // Need to create PV for average samples, probably it is important, since they have this method
{
    // You may process a full record here (will be called once with every completed record)
    // Calculate average over all samples (as an example of processing a record)
    int64_t acc_result;
    double  acc_result_d;
    unsigned int m_tr_bufsize = 512 * 1024;

    acc_result = 0;
    for (unsigned int k = 0; k < recordHeader->recordLength; k++)
    {
        acc_result += recordData[k];
    }
    acc_result_d = (double)acc_result / (double)recordHeader->recordLength;

#ifdef PRINT_RECORD_INFO
    // Print record info
    ndsInfoStream(m_node) << "--------------------------------------" << std::endl;
    ndsInfoStream(m_node) << " Device (SPD-" << (int)recordHeader->serialNumber  << "), Channel " << (int)recordHeader->channel << ", Record " << recordHeader->recordNumber << std::endl;
    ndsInfoStream(m_node) << " Samples         = " << recordHeader->recordLength << std::endl;
    ndsInfoStream(m_node) << " Status          = " << (int)recordHeader->recordStatus << std::endl;
    ndsInfoStream(m_node) << " Timestamp       = " << recordHeader->timestamp << std::endl;
    ndsInfoStream(m_node) << " Average samples = " << acc_result_d << std::endl;
    ndsInfoStream(m_node) << "--------------------------------------" << std::endl;
#endif
}

void ADQAIChannelGroup::daqMultiRecord()  // doesn't work yet, fails at GetData
{
    int trigged;
    unsigned int success;
    short* buf_a = NULL;
    short* buf_b = NULL;
    short* buf_c = NULL;
    short* buf_d = NULL;
    void* tr_buffers[8]; // GetData allows for a digitizer with max 8 channels, the unused pointers should be null pointers

    short* tr_buffers_ptr;

    // Convert chanMask from std::string to unsigned char for GetData() function
    unsigned char chanMaskChar = *m_chanMask.c_str();

    success = m_adqDevPtr->MultiRecordSetup(m_recordCnt, m_sampleCnt);
    ADQNDS_MSG_ERRLOG(success, "ERROR: MultiRecordSetup failed.");

    switch (m_trigMode)
    {
    case 0: // SW trigger
        ADQNDS_MSG_INFOLOG("Issuing Software trigger...");
        break;
    case 1: // External trigger
        ADQNDS_MSG_INFOLOG("Issuing External trigger...");
        break;
    case 2: // Level trigger
        ADQNDS_MSG_INFOLOG("Issuing Level trigger...");

        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigEdge failed.");

        success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigLevel failed.");

        success = m_adqDevPtr->SetLvlTrigChannel(m_trigChan);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigChannel failed.");
        break;
    case 3: // Internal trigger
        ADQNDS_MSG_INFOLOG("Issuing Internal trigger...");
        success = m_adqDevPtr->SetInternalTriggerPeriod(m_trigPeriod);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetInternalTriggerPeriod failed.");
        break;
    }

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
    buf_a = (short*)calloc(m_sampleCntTotal, sizeof(short));
    buf_b = (short*)calloc(m_sampleCntTotal, sizeof(short));
    buf_c = (short*)calloc(m_sampleCntTotal, sizeof(short));
    buf_d = (short*)calloc(m_sampleCntTotal, sizeof(short));

    // Create a pointer array containing the data buffer pointers
    tr_buffers[0] = (void*)buf_a;
    tr_buffers[1] = (void*)buf_b;
    tr_buffers[2] = (void*)buf_c;
    tr_buffers[3] = (void*)buf_d;

    if (buf_a == NULL || buf_b == NULL || buf_c == NULL || buf_d == NULL)
    {
        ndsErrorStream(m_node) << "ERROR: Buffer(s) == NULL" << std::endl;
        goto finish;
    }

    success = m_adqDevPtr->GetData(tr_buffers, m_sampleCntTotal, sizeof(short), 0, m_recordCntCollect, chanMaskChar, 0, m_sampleCnt, ADQ_TRANSFER_MODE_NORMAL);
    ADQNDS_MSG_ERRLOG(success, "ERROR: GetData failed.");

    success = m_adqDevPtr->DisarmTrigger();
    ADQNDS_MSG_ERRLOG(success, "ERROR: DisarmTrigger failed.");

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        // Read buffers by each channel and send them to DATA PVs
        tr_buffers_ptr = ((short*)tr_buffers[chan]);
        m_AIChannelsPtr[chan]->readMultiRecord(tr_buffers[chan], m_sampleCntTotal);
    }


finish:
    ADQNDS_MSG_INFOLOG("Acquisition finished.");
    commitChanges(true);
    m_adqDevPtr->MultiRecordClose();

    if (buf_a != NULL)
        free(buf_a);
    if (buf_b != NULL)
        free(buf_b);
    if (buf_c != NULL)
        free(buf_c);
    if (buf_d != NULL)
        free(buf_d);

    try {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) {
        /* We are probably already in "stopping", no need to panic... */
    }
}

void ADQAIChannelGroup::daqContinStream() // ----- need to investigate the API doc to develop this method
{
    unsigned int sampSkip;
    double stream_time;
    unsigned int nof_buffers, tr_bufsize, buffers_filled;
    time_t start_time, curr_time;
    double elapsed_seconds;

    unsigned int chan, buf;

    short* target_buffers[4];
    unsigned char* target_headers[4];

    int bufferstatusloops;
    int success;

    unsigned int s = 0;

    unsigned int samples_added[4];
    unsigned int headers_added[4];
    unsigned int header_status[4];
    unsigned int tot_samples_added;
    uint64_t tot_bytes_parsed;

    unsigned int stream_completed = 0;

    sampSkip = 16384;
    stream_time = 5.0; // seconds

    nof_buffers = 8;

    int adqType = m_adqDevPtr->GetADQType();
    if (adqType == 714 || adqType == 14)
    {
        tr_bufsize = 512 * 1024;
    }
    if (adqType == 7)
    {
        tr_bufsize = 256 * 1024;
    }

    // Convert chanMask from std::string to unsigned char for SetupContinuousStreaming() function
    unsigned char chanMaskChar = *m_chanMask.c_str();

    for (chan = 0; chan < m_chanCnt; chan++) 
    {
        target_buffers[chan] = NULL;
        target_headers[chan] = NULL;
        if (chanMaskChar & (1 << chan)) 
        {
            target_buffers[chan] = (short*)malloc(tr_bufsize * sizeof(char));
            target_headers[chan] = (unsigned char*)malloc(10 * sizeof(uint32_t));
        }
    }

    success = m_adqDevPtr->StopStreaming();
    ADQNDS_MSG_ERRLOG(success, "ERROR: StopStreaming failed.");

    success = m_adqDevPtr->ContinuousStreamingSetup(chanMaskChar);
    ADQNDS_MSG_WARNLOG(success, "WARNING: ContinuousStreamingSetup failed.");

    success = m_adqDevPtr->SetSampleSkip(sampSkip);
    ADQNDS_MSG_WARNLOG(success, "WARNING: ContinuousStreamingSetup failed.");

    // Start streaming
    tot_samples_added = 0;
    tot_bytes_parsed = 0;

    success = m_adqDevPtr->StartStreaming();
    ADQNDS_MSG_ERRLOG(success, "ERROR: StartStreaming failed.");

    time(&start_time);

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

    switch (m_trigMode)
    {
    case 0: // SW trigger
        ADQNDS_MSG_INFOLOG("Issuing Software trigger...");
        break;
    case 1: // External trigger
        ADQNDS_MSG_INFOLOG("Issuing External trigger...");
        break;
    case 2: // Level trigger
        ADQNDS_MSG_INFOLOG("Issuing Level trigger...");

        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigEdge failed.");

        success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigLevel failed.");

        success = m_adqDevPtr->SetLvlTrigChannel(m_trigChan);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetLvlTrigChannel failed.");
        break;
    case 3: // Internal trigger
        ADQNDS_MSG_INFOLOG("Issuing Internal trigger...");
        success = m_adqDevPtr->SetInternalTriggerPeriod(m_trigPeriod);
        ADQNDS_MSG_ERRLOG(success, "ERROR: SetInternalTriggerPeriod failed.");
        break;
    }

    time(&curr_time);
    while (!stream_completed)
    {

        bufferstatusloops = 0;
        buffers_filled = 0;

        while (buffers_filled == 0 && success)
        {
            success = m_adqDevPtr->GetTransferBufferStatus(&buffers_filled);

            if (buffers_filled == 0)
            {
                sleep(10);
                bufferstatusloops++;

                if (bufferstatusloops > 2000)
                {
                    // If the DMA transfer is taking too long, we should flush the DMA buffer before it times out. The timeout defaults to 60 seconds.
                    ADQNDS_MSG_INFOLOG("Timeout, flushing DMA...");
                    success = m_adqDevPtr->FlushDMA();
                    ADQNDS_MSG_ERRLOG(success, "ERROR: FlushDMA failed.");
                }
            }
        }

        for (buf = 0; buf < buffers_filled; buf++)
        {
            ADQNDS_MSG_INFOLOG("Receiving data...");
            success = m_adqDevPtr->GetDataStreaming((void**)target_buffers, (void**)target_headers, chanMaskChar, samples_added, headers_added, header_status);
            ADQNDS_MSG_ERRLOG(success, "ERROR: GetDataStreaming failed.");

            tot_samples_added += samples_added[0] + samples_added[1] + samples_added[2] + samples_added[3];
            tot_bytes_parsed += (uint64_t)tr_bufsize;

            // Here, process data. (perform calculations, write to file, let another process operate on the buffers, etc). 
            // This will likely be the bottleneck for the transfer rate.
            // NOT NEEDED !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            //for (chan = 0; chan < m_chanCnt; chan++) {
            //    if (!(chanMaskChar & (1 << chan)))
            //        continue;
            //    for (s = 0; s < samples_added[0]; s++) {
            //        fprintf(outfile[chan], "%d\n", target_buffers[chan][s]);
            //    }
            //}
        }

        time(&curr_time);
        elapsed_seconds = difftime(curr_time, start_time);

        if (elapsed_seconds > stream_time) 
        {
            stream_completed = 1;
            ADQNDS_MSG_INFOLOG("Acquisition finished due to achieved target stream time.");
        }

        success = m_adqDevPtr->GetStreamOverflow();
        if (success)
        {
            stream_completed = 1;
            success = 0;
            ADQNDS_MSG_ERRLOG(success, "ERROR: GetStreamOverflow detected.");
        }
    }

finish:
    ADQNDS_MSG_INFOLOG("Acquisition finished.");
    commitChanges(true);
    m_adqDevPtr->StopStreaming();

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan) 
    {
        if (target_buffers[chan])
            free(target_buffers[chan]);
        if (target_headers[chan])
            free(target_headers[chan]);
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