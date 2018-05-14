#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstring>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

#define sleep(interval) usleep(1000*interval) // usleep - microsecond interval

static struct timespec tsref;
void timer_start(void)
{
    if (clock_gettime(CLOCK_REALTIME, &tsref) < 0) {
        printf("\nFailed to start timer.");
        return;
    }
}
unsigned int timer_time_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        printf("\nFailed to get system time.");
        return -1;
    }
    return (unsigned int)((int)(ts.tv_sec - tsref.tv_sec) * 1000 +
        (int)(ts.tv_nsec - tsref.tv_nsec) / 1000000);
}

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adq_dev(adq_dev),
    m_trigmode(0),
    m_adjustBias(0),
    m_daqmode(2),
    m_pattmode(0),
    m_channels(6),
    m_channelmask("0x08"),
    m_nofrecords(100),
    m_nofsamples(1000),
    m_dbs_bypass(0),
    m_dbs_dctarget(0),
    m_dbs_lowsat(0),
    m_dbs_upsat(0),
    m_trigmodeChanged(true),
    m_biasChanged(true),
    m_dbsbypassChanged(true),
    m_dbsdcChanged(true),
    m_dbslowsatChanged(true),
    m_dbsupsatChanged(true),
    m_pattmodeChanged(true),
    m_nofrecordsChanged(true),
    m_nofsamplesChanged(true),
    m_daqmodeChanged(true),
    m_channelsChanged(true),
    m_channelmaskChanged(true),
    m_trigmodePV(nds::PVDelegateIn<std::int32_t>("TriggerMode-RB", std::bind(&ADQAIChannelGroup::getTriggerMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_adjustBiasPV(nds::PVDelegateIn<std::int32_t>("AdjustBias-RB", std::bind(&ADQAIChannelGroup::getAdjustBias,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_bypassPV(nds::PVDelegateIn<std::int32_t>("DBS-Bypass-RB", std::bind(&ADQAIChannelGroup::getDBSbypass,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_dctargetPV(nds::PVDelegateIn<std::int32_t>("DBS-DC-RB", std::bind(&ADQAIChannelGroup::getDBSdc,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_lowsatPV(nds::PVDelegateIn<std::int32_t>("DBS-LowSat-RB", std::bind(&ADQAIChannelGroup::getDBSlowsat,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_upsatPV(nds::PVDelegateIn<std::int32_t>("DBS-UpSat-RB", std::bind(&ADQAIChannelGroup::getDBSupsat,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_pattmodePV(nds::PVDelegateIn<std::int32_t>("PatternMode-RB", std::bind(&ADQAIChannelGroup::getPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_channelsPV(nds::PVDelegateIn<std::int32_t>("Channels-RB", std::bind(&ADQAIChannelGroup::getChannels,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_channelmaskPV(nds::PVDelegateIn<std::string>("ChannelMask-RB", std::bind(&ADQAIChannelGroup::getChannelMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_nofrecordsPV(nds::PVDelegateIn<std::int32_t>("NofRecords-RB", std::bind(&ADQAIChannelGroup::getNofRecords,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_maxsamplesPV(nds::PVDelegateIn<std::int32_t>("MaxSamples-RB", std::bind(&ADQAIChannelGroup::getMaxSamples,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_nofsamplesPV(nds::PVDelegateIn<std::int32_t>("NofSamples-RB", std::bind(&ADQAIChannelGroup::getNofSamples,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_daqmodePV(nds::PVDelegateIn<std::int32_t>("DAQMode-RB", std::bind(&ADQAIChannelGroup::getDAQMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{ 
    parentNode.addChild(m_node);

    nofchan = m_adq_dev->GetNofChannels();

    for (size_t numChannel(0); numChannel != nofchan; ++numChannel)
    {
        std::ostringstream channelName;
        channelName << "CH" << numChannel;
        m_AIChannels.push_back(std::make_shared<ADQAIChannel>(channelName.str(), m_node, numChannel, m_adq_dev));
    }

    // PVs for Trigger Mode
    nds::enumerationStrings_t triggerModeList;
    triggerModeList.push_back("SW trigger");
    triggerModeList.push_back("External trigger");
    triggerModeList.push_back("Level trigger");
    triggerModeList.push_back("Internal trigger");
    nds::PVDelegateOut<std::int32_t> node(nds::PVDelegateOut<std::int32_t>("TriggerMode", std::bind(&ADQAIChannelGroup::setTriggerMode,
                                                                                                  this,
                                                                                                  std::placeholders::_1,
                                                                                                  std::placeholders::_2),
                                                                                          std::bind(&ADQAIChannelGroup::getTriggerMode,
                                                                                                  this,
                                                                                                  std::placeholders::_1,
                                                                                                  std::placeholders::_2)));
    node.setEnumeration(triggerModeList);
    m_node.addChild(node);

    m_trigmodePV.setScanType(nds::scanType_t::interrupt);
    m_trigmodePV.setEnumeration(triggerModeList);
    m_node.addChild(m_trigmodePV);

    // PVs for Adjustable Bias
    m_node.addChild(nds::PVDelegateOut<std::int32_t>("AdjustBias",
                                                   std::bind(&ADQAIChannelGroup::setAdjustBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getAdjustBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_adjustBiasPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_adjustBiasPV);

    // PV for DBS setup
    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-Bypass",
                                                   std::bind(&ADQAIChannelGroup::setDBSbypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSbypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_bypassPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_bypassPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-DC",
                                                   std::bind(&ADQAIChannelGroup::setDBSdc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSdc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_dctargetPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_dctargetPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-LowSat",
                                                   std::bind(&ADQAIChannelGroup::setDBSlowsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSlowsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_lowsatPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_lowsatPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-UpSat",
                                                   std::bind(&ADQAIChannelGroup::setDBSupsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSupsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_upsatPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_upsatPV);

    // PV for Pattern Mode
    nds::enumerationStrings_t patternModeList;
    patternModeList.push_back("Normal");
    patternModeList.push_back("Test (x)");
    patternModeList.push_back("Count upwards");
    patternModeList.push_back("Count downwards");
    patternModeList.push_back("Alternating ups and downs");
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

    m_pattmodePV.setScanType(nds::scanType_t::interrupt);
    m_pattmodePV.setEnumeration(patternModeList);
    m_node.addChild(m_pattmodePV);

    // PV for channel enabling
    nds::enumerationStrings_t channelMaskList;
    channelMaskList.push_back("A+B+C+D");
    channelMaskList.push_back("A+B");
    channelMaskList.push_back("C+D");
    channelMaskList.push_back("A");
    channelMaskList.push_back("B");
    channelMaskList.push_back("C");
    channelMaskList.push_back("D");
    node = nds::PVDelegateOut<std::int32_t>("Channels", std::bind(&ADQAIChannelGroup::setChannels,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                        std::bind(&ADQAIChannelGroup::getChannels,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));
    node.setEnumeration(channelMaskList);
    m_node.addChild(node);

    m_channelsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_channelsPV);

    nds::PVDelegateOut<std::string> node_str("ChannelMask", std::bind(&ADQAIChannelGroup::setChannelMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                            std::bind(&ADQAIChannelGroup::getChannelMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));

    m_node.addChild(node_str);

    m_channelmaskPV.setScanType(nds::scanType_t::interrupt);
    m_channelmaskPV.setMaxElements(4);
    m_node.addChild(m_channelmaskPV);

    // PVs for records
    node = nds::PVDelegateOut<std::int32_t>("NofRecords", std::bind(&ADQAIChannelGroup::setNofRecords,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getNofRecords,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_nofrecordsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_nofrecordsPV);

    //PVs for samples
    m_maxsamplesPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_maxsamplesPV);

    node = nds::PVDelegateOut<std::int32_t>("NofSamples", std::bind(&ADQAIChannelGroup::setNofSamples,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getNofSamples,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_nofsamplesPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_nofsamplesPV);

    // PVs for data acquisition modes
    nds::enumerationStrings_t daqModeList;
    daqModeList.push_back("Multi-Record");
    daqModeList.push_back("Continuous streaming");
    daqModeList.push_back("Triggered streaming");
    node = nds::PVDelegateOut<std::int32_t>("DAQMode", std::bind(&ADQAIChannelGroup::setDAQMode,
                                                                                     this,
                                                                                     std::placeholders::_1,
                                                                                     std::placeholders::_2),
                                                       std::bind(&ADQAIChannelGroup::getDAQMode,
                                                                                     this,
                                                                                     std::placeholders::_1,
                                                                                     std::placeholders::_2));
    node.setEnumeration(daqModeList);
    m_node.addChild(node);

    m_daqmodePV.setScanType(nds::scanType_t::interrupt);
    m_daqmodePV.setEnumeration(triggerModeList);
    m_node.addChild(m_daqmodePV);

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

void ADQAIChannelGroup::setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigmode = pValue;
    m_trigmodeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTriggerMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigmode;
}

void ADQAIChannelGroup::setAdjustBias(const timespec &pTimestamp, const std::int32_t &pValue) 
{
    m_adjustBias = pValue;
    m_biasChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getAdjustBias(timespec* pTimestamp, std::int32_t* pValue)    
{
    *pValue = m_adjustBias;
}

void ADQAIChannelGroup::setDBSbypass(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_bypass = pValue;
    m_dbsbypassChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSbypass(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_bypass;
}

void ADQAIChannelGroup::setDBSdc(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_dctarget = pValue;
    m_dbsdcChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSdc(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_dctarget;
}

void ADQAIChannelGroup::setDBSlowsat(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_lowsat = pValue;
    m_dbslowsatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSlowsat(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_lowsat;
}

void ADQAIChannelGroup::setDBSupsat(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_upsat = pValue;
    m_dbsupsatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSupsat(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_upsat;
}

void ADQAIChannelGroup::setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_pattmode = pValue;
    m_pattmodeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getPatternMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_pattmode;
}

void ADQAIChannelGroup::setChannels(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_channels = pValue;
    m_channelsChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getChannels(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_channels;
}

void ADQAIChannelGroup::setChannelMask(const timespec &pTimestamp, const std::string &pValue)
{
    m_channelmask = pValue;
    commitChanges();
}

void ADQAIChannelGroup::getChannelMask(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_channelmask;
}

void ADQAIChannelGroup::setNofRecords(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_nofrecords = pValue;
    m_nofrecordsChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getNofRecords(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_nofrecords;
}

void ADQAIChannelGroup::getMaxSamples(timespec* pTimestamp, std::int32_t* pValue)        
{
    *pValue = m_maxsamples;
}

void ADQAIChannelGroup::setNofSamples(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_nofsamples = pValue;
    m_nofsamplesChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getNofSamples(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_nofsamples;
}

void ADQAIChannelGroup::setDAQMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_daqmode = pValue;
    m_daqmodeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDAQMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_daqmode;
}

void ADQAIChannelGroup::commitChanges(bool calledFromAcquisitionThread)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    int i;

    /* Allow changes to parameters when device is ON/STOPPING/INITIALISING states.
     * Do not apply changes when device is on acquisition state.
     */
    if (!calledFromAcquisitionThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) {
        return;
    }
    
    if (m_biasChanged)
    {
        m_biasChanged = false;
        const char chan[5] = "ABCD";
        success = m_adq_dev->HasAdjustableBias();
        if (success)
        {
            for (ch = 0; ch < nofchan; ++ch)
            {
                success = m_adq_dev->SetAdjustableBias(ch + 1, m_adjustBias);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: " << "Failed setting adjustable bias for channel " << chan[ch] << std::endl;
                }
                else
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == nofchan)
            {
                ndsInfoStream(m_node) << "SUCCESS:" << "Adjustable Bias for all channels is set." << std::endl;
            }
        }

        m_adjustBiasPV.push(now, m_adjustBias);
    }

    if (m_dbsbypassChanged || m_dbsdcChanged || m_dbslowsatChanged || m_dbsupsatChanged)
    {
        m_dbsbypassChanged = false;
        m_dbsdcChanged = false;
        m_dbslowsatChanged = false;
        m_dbsupsatChanged = false;

        success = m_adq_dev->GetNofDBSInstances(&dbs_n_of_inst);
        if (success)
        {
            ndsInfoStream(m_node) << "Number of DBS instances: " << dbs_n_of_inst << std::endl;
            i = 0;
            for (dbs_inst = 0; dbs_inst < dbs_n_of_inst; ++dbs_inst)
            {
                success = m_adq_dev->SetupDBS(dbs_inst, m_dbs_bypass, m_dbs_dctarget, m_dbs_lowsat, m_dbs_upsat);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: Failed setting up DBS instance " << dbs_inst << std::endl;
                }
                else
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == dbs_n_of_inst)
            {
                ndsInfoStream(m_node) << "SUCCESS: All " << dbs_n_of_inst << " DBS instances are set up." << std::endl;
            }
        }

        m_dbs_bypassPV.push(now, m_dbs_bypass);
        m_dbs_dctargetPV.push(now, m_dbs_dctarget);
        m_dbs_lowsatPV.push(now, m_dbs_lowsat);
        m_dbs_upsatPV.push(now, m_dbs_upsat);
    }

    if (m_pattmodeChanged)
    {
        m_pattmodeChanged = false;
        success = m_adq_dev->SetTestPatternMode(m_pattmode);
        if (!success)
        {
            ndsWarningStream(m_node) << "FAILURE: Failed setting up pattern mode." << std::endl;
        }
        else
        {
            ndsInfoStream(m_node) << "SUCCESS: Pattern Mode is set to " << m_pattmode << std::endl;
        }

        m_pattmodePV.push(now, m_pattmode);
    }

    if (m_channelsChanged)
    {
        m_channelsChanged = false;
        m_channelbits = 0000;

        if (!nofchan)
        {
            ndsWarningStream(m_node) << "FAILURE: No channels are found." << std::endl;
        }
        else
        {
            switch (m_channels)
            {
            case 0: // ch A+B+C+D
                m_channelbits |= 1111;
                m_channelmask = 0xF;
                break;
            case 1: // ch A+B  
                m_channelbits |= 1100;
                m_channelmask = 0x3;
                break;
            case 2: // ch C+D  
                m_channelbits |= 0011;
                m_channelmask = 0xC;
                break;
            case 3: // ch A
                m_channelbits |= 1000;
                m_channelmask = 0x1;
                break;
            case 4: // ch B
                m_channelbits |= 0100;
                m_channelmask = 0x2;
                break;
            case 5: // ch C
                m_channelbits |= 0010;
                m_channelmask = 0x4;
                break;
            case 6: // ch D
                m_channelbits |= 0001;
                m_channelmask = 0x8;
                break;
            }
        }

        channelmask_char = *m_channelmask.c_str();

        m_channelmaskPV.push(now, m_channelmask);
        m_channelsPV.push(now, m_channelbits);
    }

    if (m_nofsamplesChanged || m_nofrecordsChanged || m_daqmodeChanged || m_trigmodeChanged)
    {
        m_nofsamplesChanged = false;
        m_nofrecordsChanged = false;
        m_daqmodeChanged = false;
        m_trigmodeChanged = false;

        success = m_adq_dev->SetTriggerMode(m_trigmode+1);
        if (!success)
        {
            ndsWarningStream(m_node) << "FAILURE: Trigger Mode was not set." << std::endl;
        }
        else
        {
            ndsInfoStream(m_node) << "SUCCESS: Trigger Mode is set to " << m_trigmode << std::endl;

            if (m_nofrecords != -1)
            {
                success = m_adq_dev->GetMaxNofSamplesFromNofRecords(m_nofrecords, &max_nofsamples);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: Couldn't get the MAX number of samples (set number of records)." << std::endl;
                }
                else
                {
                    m_maxsamples = max_nofsamples;
                    m_maxsamplesPV.push(now, m_maxsamples);
                    ndsInfoStream(m_node) << "SUCCESS: Maximum number of samples is " << m_maxsamples << std::endl;

                    if (m_nofsamples > m_maxsamples)
                    {
                        ndsErrorStream(m_node) << "ERROR: Chosen number of samples is higher than the MAX: " << m_nofsamples << ">" << m_maxsamples << std::endl;
                    }
                }
            }

            switch (m_daqmode)
            {
            case 0:   // Multi-Record
                success = m_adq_dev->MultiRecordSetChannelMask(m_channelbits);
                if (success)
                {
                    success = m_adq_dev->MultiRecordSetup(m_nofrecords, m_nofsamples);
                    if (success)
                    {

                    }
                }
                break;
            case 1:   // Continuous streaming
                success = m_adq_dev->ContinuousStreamingSetup(m_channelbits);
                break;
            case 2:   // Triggered streaming
                break;
            }
            
        }
        
        m_nofrecordsPV.push(now, m_nofrecords);
        m_nofsamplesPV.push(now, m_nofsamples);
        m_daqmodePV.push(now, m_daqmode);
        m_trigmodePV.push(now, m_trigmode);
    }
}


void ADQAIChannelGroup::onSwitchOn()
{
    // Enable all channels
    for (auto const& channel : m_AIChannels) 
    {
        channel->setState(nds::state_t::on);
    }
    commitChanges(true);
}

void ADQAIChannelGroup::onSwitchOff()
{
    // Disable all channels 
    for (auto const& channel : m_AIChannels) 
    {
        channel->setState(nds::state_t::off);
    }
    commitChanges();
}

void ADQAIChannelGroup::onStart()
{
    // Start all Channels 
    for (auto const& channel : m_AIChannels) 
    {
        channel->setState(nds::state_t::running);
    }

    // Start data acquisition
    m_stop = false;
    m_acquisitionThread = m_node.runInThread("Acquisition", std::bind(&ADQAIChannelGroup::acquisition, this));
}

void ADQAIChannelGroup::onStop()
{
    m_stop = true;  
    m_adq_dev->StopStreaming();

    m_acquisitionThread.join();

    // Stop channels
    for (auto const& channel : m_AIChannels) 
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

void ADQAIChannelGroup::acquisition()
{
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

    int trigged;
    int i;

    unsigned int m_tr_nofbuf = 8;
    unsigned int m_tr_bufsize = 512 * 1024;

    StreamingHeader_t* tr_headers[4] = { NULL, NULL, NULL, NULL };
    short* tr_buffers[4] = { NULL, NULL, NULL, NULL };
    short* tr_extra[4] = { NULL, NULL, NULL, NULL };
    short* record_data;
    unsigned int pretrig_samples = 0;
    unsigned int holdoff_samples = 0;
    unsigned int buffers_filled = 0;
    unsigned int succes_loc;
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

    for (ch = 0, i = 0; ch < nofchan; ++ch, ++i)
    {
        tr_buffers[ch] = (short int*)malloc((size_t)m_tr_bufsize);
        if (!tr_buffers[ch])
        {
            ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target buffers." << std::endl;
            break;
        }
        else
        {
            tr_headers[ch] = (StreamingHeader_t*)malloc((size_t)m_tr_bufsize);
            if (!tr_headers[ch])
            {
                ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target buffers." << std::endl;
                break;
            }
            else
            {
                tr_extra[ch] = (short int*)malloc((size_t)(sizeof(short)*m_nofsamples));
                if (!tr_extra[ch])
                {
                    ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target extradata." << std::endl;
                    break;
                }
            }
        }
    }

    if (!i == nofchan)
    {
        onStop();
        return;
    }
    else
    {
        record_data = (short int*)malloc((size_t)(sizeof(short)*m_nofsamples));
        if (!tr_extra[ch])
        {
            ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for ProcessRecord." << std::endl;
            return;
        }

        for (ch = 0; ch < nofchan; ++ch)
        {
            nofrecords_sum += m_nofrecords;
        }

        success = m_adq_dev->TriggeredStreamingSetup(m_nofrecords, m_nofsamples, pretrig_samples, holdoff_samples, channelmask_char);
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at TriggeredStreamingSetup." << std::endl;
            return;
        }
        else
        {
            success = m_adq_dev->SetTransferBuffers(m_tr_nofbuf, m_tr_bufsize);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at SetTransferBuffers." << std::endl;
                return;
            }
            else
            {
                success = m_adq_dev->StopStreaming();
                if (!success)
                {
                    ndsErrorStream(m_node) << "ERROR: Failed at StopStreaming." << std::endl;
                    return;
                }
                else
                {
                    success = m_adq_dev->StartStreaming();
                    if (!success)
                    {
                        ndsErrorStream(m_node) << "ERROR: Failed at StartStreaming." << std::endl;
                        return;
                    }
                    else
                    {
                        switch (m_trigmode)
                        {
                        case 0:        // SW trigger
                            ndsInfoStream(m_node) << "Issuing Software trigger... " << std::endl;
                            success = m_adq_dev->DisarmTrigger();
                            if (success)
                            {
                                success = m_adq_dev->ArmTrigger();
                                if (success)
                                {
                                    for (i = 0; i < m_nofrecords; ++i)
                                    {
                                        success = m_adq_dev->SWTrig();
                                        if (!success)
                                        {
                                            ndsErrorStream(m_node) << "ERROR: Failed at SWTrig." << std::endl;
                                            return;
                                        }
                                    }

                                    do
                                    {
                                        buffers_filled = 0;
                                        succes_loc = 1;

                                        success = m_adq_dev->GetStreamOverflow();
                                        if (success)
                                        {
                                            ndsErrorStream(m_node) << "ERROR: Streaming overflow detected." << std::endl;
                                            return;
                                        }

                                        success = m_adq_dev->GetTransferBufferStatus(&buffers_filled);
                                        if (!success)
                                        {
                                            ndsErrorStream(m_node) << "ERROR: Failed at GetTransferBufferStatus." << std::endl;
                                            return;
                                        }

                                        // Poll for the transfer buffer status as long as the timeout has not been
                                        // reached and no buffers have been filled.
                                        while (!buffers_filled)
                                        {
                                            // Mark the loop start
                                            timer_start();
                                            while (!buffers_filled && (timer_time_ms() < timeout_ms))
                                            {
                                                success = m_adq_dev->GetTransferBufferStatus(&buffers_filled);
                                                if (!success)
                                                {
                                                    ndsErrorStream(m_node) << "ERROR: Failed at GetTransferBufferStatus." << std::endl;
                                                    return;
                                                }
                                                // Sleep to avoid loading the processor too much
                                                sleep(10);
                                            }

                                            // Timeout reached, flush the transfer buffer to receive data
                                            if (!buffers_filled)
                                            {
                                                ndsInfoStream(m_node) << "Timeout, flushing DMA..." << std::endl;
                                                success = m_adq_dev->FlushDMA();
                                                if (!success)
                                                {
                                                    ndsErrorStream(m_node) << "ERROR: Failed at FlushDMA." << std::endl;
                                                    return;
                                                }
                                            }
                                        }

                                        success = m_adq_dev->GetDataStreaming((void**)tr_buffers, (void**)tr_headers, channelmask_char, samples_added, headers_added, header_status);
                                        if (!success)
                                        {
                                            ndsErrorStream(m_node) << "ERROR: Failed at GetDataStreaming." << std::endl;
                                            goto finish;
                                        }
                                        else
                                        {
                                            for (ch = 0; ch < 4; ++ch) {
                                                if (headers_added[ch] > 0) {
                                                    if (header_status[ch])
                                                        headers_done = headers_added[ch];
                                                    else
                                                        // One incomplete record in the buffer (header is copied to the front
                                                        // of the buffer later)
                                                        headers_done = headers_added[ch] - 1;

                                                    // If there is at least one complete header
                                                    records_completed[ch] += headers_done;
                                                }

                                                // Parse the added samples
                                                if (samples_added[ch] > 0) {
                                                    samples_remaining = samples_added[ch];

                                                    // Handle incomplete record at the start of the buffer
                                                    if (samples_extradata[ch] > 0) {
                                                        if (headers_done == 0) {
                                                            // There is not enough data in the transfer buffer to complete
                                                            // the record. Add all the samples to the extradata buffer.
                                                            std::memcpy(&tr_extra[ch][samples_extradata[ch]],
                                                                tr_buffers[ch],
                                                                sizeof(short)*samples_added[ch]);
                                                            samples_remaining -= samples_added[ch];
                                                            samples_extradata[ch] += samples_added[ch];
                                                        }
                                                        else {
                                                            // Move data to record_data
                                                            std::memcpy((void*)record_data, tr_extra[ch], sizeof(short)*samples_extradata[ch]);
                                                            std::memcpy((void*)(record_data + samples_extradata[ch]), tr_buffers[ch], sizeof(short)*(tr_headers[ch][0].RecordLength - samples_extradata[ch]));

                                                            samples_remaining -= tr_headers[ch][0].RecordLength -
                                                                samples_extradata[ch];
                                                            samples_extradata[ch] = 0;

                                                            /*
                                                            adq14_triggered_streaming_process_record(record_data, &target_headers[ch][0]);
                                                            printf("Completed record %u on channel %u, %u samples.\n",
                                                                target_headers[ch][0].RecordNumber, ch,
                                                                target_headers[ch][0].RecordLength);
                                                                */
                                                        }
                                                    }
                                                    else {
                                                        if (headers_done == 0) {
                                                            // The samples in the transfer buffer begin a new record, this
                                                            // record is incomplete.
                                                            std::memcpy(tr_extra[ch],
                                                                tr_buffers[ch],
                                                                sizeof(short)*samples_added[ch]);
                                                            samples_remaining -= samples_added[ch];
                                                            samples_extradata[ch] = samples_added[ch];
                                                        }
                                                        else {

                                                            // Copy data to record buffer
                                                            std::memcpy((void*)record_data, tr_buffers[ch], sizeof(short)*tr_headers[ch][0].RecordLength);
                                                            samples_remaining -= tr_headers[ch][0].RecordLength;
                                                            
                                                            /*
                                                            adq14_triggered_streaming_process_record(record_data, &target_headers[ch][0]);
                                                            printf("Completed record %u on channel %u, %u samples.\n",
                                                                target_headers[ch][0].RecordNumber, ch,
                                                                target_headers[ch][0].RecordLength);
                                                                */
                                                        }
                                                    }
                                                    // At this point: the first record in the transfer buffer or the entire
                                                    // transfer buffer has been parsed.

                                                    // Loop through complete records fully inside the buffer
                                                    for (i = 1; i < headers_done; ++i) {
                                                        // Copy data to record buffer
                                                        std::memcpy((void*)record_data, (&tr_buffers[ch][samples_added[ch] - samples_remaining]), sizeof(short)*tr_headers[ch][i].RecordLength);

                                                        samples_remaining -= tr_headers[ch][i].RecordLength;

                                                        /*
                                                        adq14_triggered_streaming_process_record(record_data, &target_headers[ch][i]);
                                                        printf("Completed record %u on channel %u, %u samples.\n",
                                                            target_headers[ch][i].RecordNumber, ch,
                                                            target_headers[ch][i].RecordLength);
                                                            */
                                                    }

                                                    if (samples_remaining > 0) {
                                                        // There is an incomplete record at the end of the transfer buffer
                                                        // Copy the incomplete header to the start of the target_headers buffer
                                                        std::memcpy(tr_headers[ch],
                                                            &tr_headers[ch][headers_done],
                                                            sizeof(StreamingHeader_t));

                                                        // Copy any remaining samples to the target_buffers_extradata buffer,
                                                        // they belong to the incomplete record
                                                        std::memcpy(tr_extra[ch],
                                                            &tr_buffers[ch][samples_added[ch] - samples_remaining],
                                                            sizeof(short)*samples_remaining);
                                                        // printf("Incomplete at end of transfer buffer. %u samples.\n", samples_remaining);
                                                        // printf("Copying %u samples to the extradata buffer\n", samples_remaining);
                                                        samples_extradata[ch] = samples_remaining;
                                                        samples_remaining = 0;
                                                    }
                                                }
                                            }

                                            // Update received_all_records
                                            nof_received_records_sum = 0;
                                            for (ch = 0; ch < 4; ++ch)
                                            {
                                                nof_received_records_sum += records_completed[ch];
                                            }

                                            // Determine if collection is completed
                                            received_all_records = (nof_received_records_sum >= nof_records_sum);

                                        }
                                    } while (!received_all_records);

                                    for (auto const& channel : m_AIChannels)
                                    {
                                        channel->read(records_completed + channel->m_channelNum, m_nofsamples);
                                    }
                                    
                                    commitChanges(true);
                                }
                            }
                            break;
                        case 1:       // External trigger
                        case 2:       // Level trigger
                        case 3:       // Internal trigger
                            break;
                        }
                    }
                }
            }
        }
    }

finish:
    ndsInfoStream(m_node) << "Acquisition finished." << std::endl;

    m_adq_dev->StopStreaming();
    
    for (ch = 0; ch < 4; ch++) {
        if (tr_buffers[ch])
            free(tr_buffers[ch]);
        if (tr_headers[ch])
            free(tr_headers[ch]);
        if (tr_extra[ch])
            free(tr_extra[ch]);
    }

    try {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) {
        /* We are probably already in "stopping", no need to panic... */
    }
}
   