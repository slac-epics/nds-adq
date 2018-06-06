#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <time.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQFourteen.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

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
    m_sampleCntMaxPV(nds::PVDelegateIn<std::int32_t>("SampleCntMax-RB", std::bind(&ADQAIChannelGroup::getSampleCntMax,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleCntPV(nds::PVDelegateIn<std::int32_t>("SampleCnt-RB", std::bind(&ADQAIChannelGroup::getSampleCnt,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampleCntTotalPV(nds::PVDelegateIn<std::int32_t>("SampleCntTotal-RB", std::bind(&ADQAIChannelGroup::getSamplesTotal,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigModePV(nds::PVDelegateIn<std::int32_t>("TrigMode-RB", std::bind(&ADQAIChannelGroup::getTrigMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);

    m_chanCnt = m_adqDevPtr->GetNofChannels();

    // Create vector of pointers to each chanel
    for (size_t channelNum(0); channelNum != m_chanCnt; ++channelNum)
    {
        std::ostringstream channelName;
        channelName << "CH" << channelNum;
        m_AIChannelsPtr.push_back(std::make_shared<ADQAIChannel>(channelName.str(), m_node, channelNum));
    }

    // PVs for data acquisition modes
    //// urojec L2: why all this push back, why not just fix the list. It is not something that dynamically changes
    //// ppipp: I honestly didn't try with fixed list; saw in Niklas' example and it worked
    ////        will try with fixed list; there are no official examples with enumlists in NDS doxygen
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
    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DCBias",
                                                   std::bind(&ADQAIChannelGroup::setDcBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDcBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dcBiasPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dcBiasPV);

    // PV for DBS setup
    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBSBypass",
                                                   std::bind(&ADQAIChannelGroup::setDbsBypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsBypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbsBypassPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbsBypassPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBSDC",
                                                   std::bind(&ADQAIChannelGroup::setDbsDc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsDc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbsDcPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbsDcPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBSLowSat",
                                                   std::bind(&ADQAIChannelGroup::setDbsLowSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsLowSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbsLowSatPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbsLowSatPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBSUpSat",
                                                   std::bind(&ADQAIChannelGroup::setDbsUpSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDbsUpSat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

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

    // PVs for channel masks, readback
    //m_chanActivePV.setScanType(nds::scanType_t::interrupt);
    //m_node.addChild(m_chanActivePV);

    //m_chanMaskPV.setScanType(nds::scanType_t::interrupt);
    //m_chanMaskPV.setMaxElements(STRING_ENUM);
    //m_node.addChild(m_chanMaskPV);

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

    //PVs for samples
    m_sampleCntMaxPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_sampleCntMaxPV);

    node = nds::PVDelegateOut<std::int32_t>("SampleCnt", std::bind(&ADQAIChannelGroup::setSampleCnt,
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

    //PVs for trigger level
    //m_trigLvlPV.setScanType(nds::scanType_t::interrupt);
    //m_node.addChild(m_trigLvlPV);

    // PVs for trigger edge
    //m_trigEdgePV.setScanType(nds::scanType_t::interrupt);
    //m_node.addChild(m_trigEdgePV);

    // PVs for trigger channel  
    //m_trigChanPV.setScanType(nds::scanType_t::interrupt);
    //m_node.addChild(m_trigChanPV);

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

/*
void ADQAIChannelGroup::getChanActive(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_chanActive;
}

void ADQAIChannelGroup::getChanMask(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_chanMask;
}

void ADQAIChannelGroup::getTrigLvl(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigLvl;
}

void ADQAIChannelGroup::getTrigEdge(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigEdge;
}

void ADQAIChannelGroup::getTrigChan(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigChan;
}
*/

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

        if (!success)
        {
            ndsWarningStream(m_node) << "FAILURE: Trigger Mode was not set." << std::endl;
        }
        else
        {
            ndsInfoStream(m_node) << "SUCCESS: Trigger Mode is set to " << m_trigMode << std::endl;
            m_trigModePV.push(now, m_trigMode);
        }
    }

    if (m_patternModeChanged)
    {
        m_patternModeChanged = false;
        success = m_adqDevPtr->SetTestPatternMode(m_patternMode);
        if (!success)
        {
            ndsWarningStream(m_node) << "FAILURE: Failed setting up pattern mode." << std::endl;
        }
        else
        {
            ndsInfoStream(m_node) << "SUCCESS: Pattern Mode is set to " << m_patternMode << std::endl;
            m_patternModePV.push(now, m_patternMode);
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
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: " << "Failed setting DC bias for channel " << chan_char[chan] << std::endl;
                }
                else
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == m_chanCnt)
            {
                ndsInfoStream(m_node) << "SUCCESS:" << "DC Bias is set for all channels." << std::endl;
                m_dcBiasPV.push(now, m_dcBias);
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
            ndsInfoStream(m_node) << "Number of DBS instances: " << dbsInstCnt << std::endl;
            unsigned int i = 0;
            for (unsigned char dbsInst = 0; dbsInst < dbsInstCnt; ++dbsInst)
            {
                success = m_adqDevPtr->SetupDBS(dbsInst, m_dbsBypass, m_dbsDc, m_dbsLowSat, m_dbsUpSat);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: Failed setting up DBS instance " << dbsInst << std::endl;
                }
                else
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == dbsInstCnt)
            {
                ndsInfoStream(m_node) << "SUCCESS: All " << dbsInstCnt << " DBS instances are set up." << std::endl;
                m_dbsBypassPV.push(now, m_dbsBypass);
                m_dbsDcPV.push(now, m_dbsDc);
                m_dbsLowSatPV.push(now, m_dbsLowSat);
                m_dbsUpSatPV.push(now, m_dbsUpSat);
            }
        }
    }

    if (m_chanActiveChanged)     // Needs to be moved to ADQ classes (7 and 14 have different options) ----------------------------------------
    {
        

        if (m_daqMode == 0) // Multi-Record
        {
            success = m_adqDevPtr->MultiRecordSetChannelMask(m_chanBits);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at MultiRecordSetChannelMask." << std::endl;
            }
        }
        if (m_daqMode == 1) // Continuous Streaming
        {
            success = m_adqDevPtr->ContinuousStreamingSetup(m_chanBits);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at ContinuousStreamingSetup." << std::endl;
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
            if (!success)
            {
                ndsWarningStream(m_node) << "FAILURE: Couldn't get the MAX number of samples (set number of records)." << std::endl;
            }
            else
            {
                m_sampleCntMax = sampleCntMax;
                m_sampleCntMaxPV.push(now, m_sampleCntMax);
                ndsInfoStream(m_node) << "SUCCESS: Maximum number of samples is " << m_sampleCntMax << std::endl;

                if (m_sampleCnt > m_sampleCntMax)
                {
                    ndsErrorStream(m_node) << "ERROR: Chosen number of samples is higher than the MAX: " << m_sampleCnt << ">" << m_sampleCntMax << std::endl;
                }
                else
                {
                    m_sampleCntPV.push(now, m_sampleCnt);

                    m_sampleCntTotal = m_sampleCnt * m_recordCnt;
                    m_sampleCntTotalPV.push(now, m_sampleCntTotal);
                }
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
            m_recordCntCollect = m_recordCnt;

        m_recordCntCollectPV.push(now, m_recordCntCollect);

        m_sampleCntTotal = m_sampleCnt * m_recordCntCollect;
        m_sampleCntTotalPV.push(now, m_sampleCntTotal);
    }

    /* This block processes changes applied to ADQ specific PVs
     * these will be run everytime any changes are applied to ChannelGroup PVs
     * Possible to create a public variable that will be checked on before processing this thing
     
    m_chanActivePV.read(&now, &m_chanActive);
    m_trigLvlPV.read(&now, &m_trigLvl);
    m_trigEdgePV.read(&now, &m_trigEdge);
    m_trigChanPV.read(&now, &m_trigChan);

    // for chanActivePV
    if (!m_chanCnt)
    {
        ndsWarningStream(m_node) << "FAILURE: No channels are found." << std::endl;
    }
    else
    {
        

        if (m_daqMode == 0) // Multi-Record
        {
            success = m_adqDevPtr->MultiRecordSetChannelMask(m_chanBits);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at MultiRecordSetChannelMask." << std::endl;
            }
        }
        if (m_daqMode == 1) // Continuous Streaming
        {
            success = m_adqDevPtr->ContinuousStreamingSetup(m_chanBits);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at ContinuousStreamingSetup." << std::endl;
            }

        }

    }

    // for trigEdgePV
    if (m_trigMode == 2)
    {
        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigEdge." << std::endl;
        }
    }

    // for trigChanPV
    int trigchan_int;

    switch (m_trigChan)
    {
    case 0: // None
        trigchan_int = 0;
        break;
    case 1: // ch A
        trigchan_int = 1;
        break;
    case 2: // ch B
        trigchan_int = 2;
        break;
    case 3: // ch C
        trigchan_int = 4;
        break;
    case 4: // ch D
        trigchan_int = 8;
        break;
    case 5: // ch A+B
        trigchan_int = 3;
        break;
    case 6: // ch C+D
        trigchan_int = 12;
        break;
    case 7: // ch A+B+C+D
        trigchan_int = 15;
        break;
    }

    if (m_trigMode == 2)
    {
        success = m_adqDevPtr->SetLvlTrigChannel(trigchan_int);
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigChannel." << std::endl;
        }
    }

    // for trigEdgePV
    if (m_trigLvlChanged)
    {
        m_trigLvlChanged = false;
        m_trigLvlPV.push(now, m_trigLvl);
        if (m_trigMode == 2)
        {
            success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigLevel." << std::endl;
            }
        }
    }
    */
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
    unsigned int m_tr_bufsize = 512 * 1024;

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

    // Allocate memory for record data (used for ProcessRecord function template)
    record_data = (short int*)malloc((size_t)(sizeof(short)*m_sampleCnt));

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

    success = m_adqDevPtr->TriggeredStreamingSetup(m_recordCnt, m_sampleCnt, pretrig_samples, holdoff_samples, chanMaskChar);
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at TriggeredStreamingSetup." << std::endl;
        goto finish;
    }
    else
    {
        success = m_adqDevPtr->SetTransferBuffers(m_tr_nofbuf, m_tr_bufsize);
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at SetTransferBuffers." << std::endl;
            goto finish;
        }
        else
        {
            success = m_adqDevPtr->StopStreaming();
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at StopStreaming." << std::endl;
                goto finish;
            }
            else
            {
                success = m_adqDevPtr->StartStreaming();
                if (!success)
                {
                    ndsErrorStream(m_node) << "ERROR: Failed at StartStreaming." << std::endl;
                    goto finish;
                }
                else
                {
                    switch (m_trigMode)
                    {
                    case 0: // SW trigger
                        ndsInfoStream(m_node) << "Issuing Software trigger... " << std::endl;
                        success = m_adqDevPtr->DisarmTrigger();
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
                            goto finish;
                        }
                        else
                        {
                            success = m_adqDevPtr->ArmTrigger();
                            if (!success)
                            {
                                ndsErrorStream(m_node) << "ERROR: Failed at ArmTrigger." << std::endl;
                                goto finish;
                            }
                            else
                            {
                                for (int i = 0; i < m_recordCnt; ++i)
                                {
                                    success = m_adqDevPtr->SWTrig();
                                    if (!success)
                                    {
                                        ndsErrorStream(m_node) << "ERROR: Failed at SWTrig." << std::endl;
                                        goto finish;
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
                                    if (!success)
                                    {
                                        ndsErrorStream(m_node) << "ERROR: Failed at GetTransferBufferStatus." << std::endl;
                                        goto finish;
                                    }

                                    // Poll for the transfer buffer status as long as the timeout has not been
                                    // reached and no buffers have been filled.
                                    while (!buffers_filled)
                                    {
                                        // Mark the loop start
                                        timerStart();
                                        while (!buffers_filled && (timerTimeMs() < timeout_ms))
                                        {
                                            success = m_adqDevPtr->GetTransferBufferStatus(&buffers_filled);
                                            if (!success)
                                            {
                                                ndsErrorStream(m_node) << "ERROR: Failed at GetTransferBufferStatus." << std::endl;
                                                goto finish;
                                            }
                                            // Sleep to avoid loading the processor too much
                                            sleep(10);
                                        }

                                        // Timeout reached, flush the transfer buffer to receive data
                                        if (!buffers_filled)
                                        {
                                            ndsInfoStream(m_node) << "Timeout, flushing DMA..." << std::endl;
                                            success = m_adqDevPtr->FlushDMA();
                                            if (!success)
                                            {
                                                ndsErrorStream(m_node) << "ERROR: Failed at FlushDMA." << std::endl;
                                                goto finish;
                                            }
                                        }
                                    }

                                    ndsInfoStream(m_node) << "Receiving data..." << std::endl;
                                    success = m_adqDevPtr->GetDataStreaming((void**)tr_buffers, (void**)tr_headers, chanMaskChar, samples_added, headers_added, header_status);
                                    if (!success)
                                    {
                                        ndsErrorStream(m_node) << "ERROR: Failed at GetDataStreaming." << std::endl;
                                        goto finish;
                                    }
                                    else
                                    {
                                        // Parse data
                                        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
                                        {
                                            if (!((1 << chan) & chanMaskChar))
                                                continue;

                                            if (headers_added[chan] > 0)
                                            {
                                                if (header_status[chan])
                                                    headers_done = headers_added[chan];
                                                else
                                                    // One incomplete record in the buffer (header is copied to the front
                                                    // of the buffer later)
                                                    headers_done = headers_added[chan] - 1;

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
                                            m_AIChannelsPtr[chan]->readTrigStream(tr_buffers[chan], m_sampleCntTotal);
                                        }

                                        // Update received_all_records
                                        nof_received_records_sum = 0;
                                        for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
                                        {
                                            nof_received_records_sum += records_completed[chan];
                                        }

                                        // Determine if collection is completed
                                        received_all_records = (nof_received_records_sum >= nof_records_sum);

                                    }
                                } while (!received_all_records);
                            }
                        }
                        break;
                    case 1: // External trigger  ----- need to investigate the API doc to develop this method
                        break;
                    case 2: // Level trigger ----- need to investigate the API doc to develop this method
                        ndsInfoStream(m_node) << "Issuing Level trigger... " << std::endl;
                        success = m_adqDevPtr->SetLvlTrigEdge(m_trigEdge);
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigEdge." << std::endl;
                            goto finish;
                        }
                        success = m_adqDevPtr->SetLvlTrigLevel(m_trigLvl);
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigLevel." << std::endl;
                        }
                        goto finish;
                        break;
                    case 3: // Internal trigger ----- need to investigate the API doc to develop this method
                        ndsInfoStream(m_node) << "Issuing Internal trigger... " << std::endl;
                        success = m_adqDevPtr->SetInternalTriggerPeriod(m_trigLvl); // it uses FREQ/Period variable, not lvl
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at SetInternalTriggerPeriod." << std::endl;
                            goto finish;
                        }
                        else
                        {

                        }
                        break;
                    }
                }
            }
        }
    }

finish:
    ndsInfoStream(m_node) << "Acquisition finished." << std::endl;
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
#ifdef PRINT_RECORD_INFO
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

    // Convert chanMask from std::string to unsigned char for certain ADQAPI functions
    unsigned char chanMaskChar = *m_chanMask.c_str();

    success = m_adqDevPtr->MultiRecordSetup(m_recordCnt, m_sampleCnt);
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at MultiRecordSetup." << std::endl;
        goto finish;
    }

    if (m_trigMode == 0) // SW trigger
    {
        ndsInfoStream(m_node) << "Issuing Software trigger... " << std::endl;
        success = m_adqDevPtr->DisarmTrigger();
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
            goto finish;
        }
        else
        {
            success = m_adqDevPtr->ArmTrigger();
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at ArmTrigger." << std::endl;
                goto finish;
            }
            else
            {
                success = m_adqDevPtr->SWTrig();
                if (!success)
                {
                    ndsErrorStream(m_node) << "ERROR: Failed at SWTrig." << std::endl;
                    goto finish;
                }
                else
                {
                    do
                    {
                        trigged = m_adqDevPtr->GetAcquiredAll();
                        success = m_adqDevPtr->SWTrig();
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at SWTrig." << std::endl;
                            goto finish;
                        }
                    } while (trigged == 0);
                }
            }
        }
    }

    if (m_trigMode == 1 || m_trigMode == 2 || m_trigMode == 3) // External or Level or Internal trigger
    {
        ndsWarningStream(m_node) << "Trigger the device to collect data." << std::endl;
        sleep(10);

        success = m_adqDevPtr->DisarmTrigger();
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
            goto finish;
        }
        else
        {
            success = m_adqDevPtr->ArmTrigger();
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at ArmTrigger." << std::endl;
                goto finish;
            }
            else
            {
                do
                {
                    trigged = m_adqDevPtr->GetAcquiredAll();
                } while (trigged == 0);
            }
        }
    }
    ndsInfoStream(m_node) << "All records are triggered." << std::endl;

    success = m_adqDevPtr->GetStreamOverflow();
    if (success)
    {
        ndsErrorStream(m_node) << "ERROR: Streaming overflow detected." << std::endl;
        goto finish;
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
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at GetData." << std::endl;
        goto finish;
    }

    success = m_adqDevPtr->DisarmTrigger();
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
        goto finish;
    }

    // Read buffers by each channel and send them to DATA PVs

    for (unsigned int chan = 0; chan < m_chanCnt; ++chan)
    {
        // Read buffers by each channel and send them to DATA PVs
        tr_buffers_ptr = ((short*)tr_buffers[chan]);
        //m_AIChannelsPtr[ch]->read_multirec(tr_buffers[ch], total_samples);
        m_AIChannelsPtr[chan]->readTrigStream(tr_buffers_ptr, m_sampleCntTotal);
    }


finish:
    ndsInfoStream(m_node) << "Acquisition finished." << std::endl;
    commitChanges(true);
    m_adqDevPtr->StopStreaming();

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

}
