#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

#define sleep(interval) usleep(1000*interval) // usleep - microsecond interval

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adq_dev(adq_dev),
    m_trigmodeChanged(false),
    m_biasChanged(false),
    m_dbsChanged(false),
    m_pattmodeChanged(false),
    m_nofrecordsChanged(false),
    m_nofsamplesChanged(false),
    m_daqmodeChanged(false),
    m_channelmaskChanged(false),
    m_trigmodePV(nds::PVDelegateIn<std::int32_t>("TriggerMode-RB", std::bind(&ADQAIChannelGroup::getTriggerMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_adjustBiasPV(nds::PVDelegateIn<std::int32_t>("AdjustBias-RB", std::bind(&ADQAIChannelGroup::getAdjustBias,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_settingsPV(nds::PVDelegateIn<std::vector<std::int32_t>>("DBS-RB", std::bind(&ADQAIChannelGroup::getDBS,
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

    ndsInfoStream(m_node) << "Number of channels:" << nofchan << std::endl;

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
    nds::PVDelegateOut<std::vector<std::int32_t>> node_vect(nds::PVDelegateOut<std::vector<std::int32_t>>("DBS", std::bind(&ADQAIChannelGroup::setDBS,
                                                                                                                         this,
                                                                                                                         std::placeholders::_1,
                                                                                                                         std::placeholders::_2),
                                                                                                                std::bind(&ADQAIChannelGroup::getDBS,
                                                                                                                         this,
                                                                                                                         std::placeholders::_1,
                                                                                                                         std::placeholders::_2)));
    node_vect.setMaxElements(4);
    m_node.addChild(node_vect);

    m_dbs_settingsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_settingsPV);

    // PV for Pattern Mode
    nds::enumerationStrings_t patternModeList;
    patternModeList.push_back("Normal");
    patternModeList.push_back("Test");
    patternModeList.push_back("Count upwards");
    patternModeList.push_back("Count downwards");
    patternModeList.push_back("Alternating ups and downs");
    node = nds::PVDelegateOut<std::int32_t>("PatternMode", std::bind(&ADQAIChannelGroup::setPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));
    node.setEnumeration(patternModeList);
    m_node.addChild(node);

    m_pattmodePV.setScanType(nds::scanType_t::interrupt);
    m_pattmodePV.setEnumeration(patternModeList);
    m_node.addChild(m_pattmodePV);

    // PV for channel enabling
    node = nds::PVDelegateOut<std::int32_t>("Channels", std::bind(&ADQAIChannelGroup::setChannels,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                       std::bind(&ADQAIChannelGroup::getChannels,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);

    m_channelsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_channelsPV);

    // PV for records and samples
    m_nofrecordsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_nofrecordsPV);

    m_maxsamplesPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_maxsamplesPV);

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

    // PVs for state machine
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


}

void ADQAIChannelGroup::setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigmode = pValue + 1;
    m_trigmodeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTriggerMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigmode - 1;
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

void ADQAIChannelGroup::setDBS(const timespec &pTimestamp, const std::vector<std::int32_t> &pValue)
{
    m_dbs_settings = pValue;
    m_dbsChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBS(timespec* pTimestamp, std::vector<std::int32_t>* pValue)
{
    *pValue = m_dbs_settings;
}

void ADQAIChannelGroup::setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_pattmode = pValue;
    m_pattmodeChanged = true;
}

void ADQAIChannelGroup::getPatternMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_pattmode;
}

void ADQAIChannelGroup::setChannels(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_channels = pValue;
    m_channelmaskChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getChannels(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_channels;
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
    if (m_nofrecordsChanged)
    {
        unsigned int max_nofsamples;
        success = m_adq_dev->GetMaxNofSamplesFromNofRecords(m_nofrecords, &max_nofsamples);
        if (success)
        {
            m_maxsamples = max_nofsamples;
            *pValue = m_maxsamples;
            ndsInfoStream(m_node) << "SUCCESS:" << "MaxSamples" << m_maxsamples << std::endl;
        }
        else
        {
            ndsWarningStream(m_node) << "FAILURE: " << "GetMaxNofSamplesFromNofRecords" << std::endl;
        }
    }
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

    if (!calledFromAcquisitionThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) {
        return;
    }

    if (m_trigmodeChanged)
    {
        m_trigmodeChanged = false;
        success = m_adq_dev->SetTriggerMode(m_trigmode);
        if (success)
        {
 //           m_trigmodePV.read(&now, &m_trigmode);
            m_trigmodePV.push(now, m_trigmode);
            ndsInfoStream(m_node) << "SUCCESS:" << "Trigger Mode is set to" << m_trigmode << std::endl;
        }
    }
    
    if (m_biasChanged)
    {
        m_biasChanged = false;
        const char chan[5] = "ABCD";
        unsigned int i = 0;
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
                    ndsInfoStream(m_node) << "SUCCESS:" << "Adjustable Bias for " << chan[ch] << " is set." << std::endl;
                }
            }
            sleep(1000);

            if (i == nofchan)
            {
                ndsInfoStream(m_node) << "SUCCESS:" << "Adjustable Bias for all channels is set." << std::endl;
            }
        }
    }

    if (m_dbsChanged)
    {
        m_dbsChanged = false;
        success = m_adq_dev->GetNofDBSInstances(&dbs_n_of_inst);
        if (success)
        {
            for (dbs_inst = 0; dbs_inst < dbs_n_of_inst; ++dbs_inst)
            {
                success = m_adq_dev->SetupDBS(dbs_inst, m_dbs_settings[0], m_dbs_settings[1], m_dbs_settings[2], m_dbs_settings[3]);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: " << "Failed setting up DBS instance " << dbs_inst << std::endl;
                }
                else
                {
                    ndsInfoStream(m_node) << "SUCCES: " << "DBS settings: " << &m_dbs_settings[0] << " " << &m_dbs_settings[1] << " " 
                                                                            << &m_dbs_settings[2] << " " << &m_dbs_settings[3] << std::endl;
                }
            }
            sleep(1000);
        }
    }

    if (m_pattmodeChanged)
    {
        m_pattmodeChanged = false;
        success = m_adq_dev->SetTestPatternMode(m_pattmode);
        if (!success)
        {
            ndsWarningStream(m_node) << "FAILURE: " << "Failed setting up pattern mode." << std::endl;
        }
        else
        {
            m_pattmodePV.read(&now, &m_pattmode);
            m_pattmodePV.push(now, m_pattmode);
            ndsInfoStream(m_node) << "SUCCESS:" << "Pattern Mode is set to " << m_pattmode << std::endl;
        }
    }

    if (m_channelmaskChanged)
    {
        m_channelmaskChanged = false;
        m_channelmask = 0x00;
        if (nofchan > 0)
        {
            if (m_channels == 1000)
            {
                m_channelmask |= 0x01;
            }
            if (m_channels == 1100)
            {
                m_channelmask |= 0x02;
            }
            if (m_channels == 1110)
            {
                m_channelmask |= 0x04;
            }
            if (m_channels == 1111)
            {
                m_channelmask |= 0x08;
            }
            ndsInfoStream(m_node) << "Channelmask is set to " << m_channelmask << std::endl;
        }
        else
        {
            ndsWarningStream(m_node) << "FAILURE: " << "No channels are found." << std::endl;
        }
    }

    if (m_nofsamplesChanged || m_nofrecordsChanged || m_daqmodeChanged)
    {
        m_nofsamplesChanged = false;
        m_nofrecordsChanged = false;
        m_daqmodeChanged = false;

        if (m_nofsamples > m_maxsamples)
        {
            ndsErrorStream(m_node) << "ERROR: " << "Chosen number of samples is higher than the MAX: " << m_nofsamples << ">" << m_maxsamples << std::endl;
        }
        else
        {
            switch (m_daqmode)
            {
            case 0:   // Multi-Record
                int trigged;
                success = m_adq_dev->MultiRecordSetChannelMask(m_channels);
                if (success)
                {
                    success = m_adq_dev->MultiRecordSetup(m_nofrecords, m_nofsamples);
                    if (success)
                    {
                        switch (m_trigmode)
                        {
                        case 0:        // SW trigger
                            ndsInfoStream(m_node) << "Issuing Software trigger. " << std::endl;
                            success = m_adq_dev->DisarmTrigger();
                            if (success)
                            {
                                success = m_adq_dev->ArmTrigger();
                                if (success)
                                {
                                    success = m_adq_dev->SWTrig();
                                    if (success)
                                    {
                                        do
                                        {
                                            trigged = m_adq_dev->GetAcquiredAll();
                                        } while (trigged == 0);

                                        ndsInfoStream(m_node) << "All records are triggered." << m_channelmask << std::endl;
                                    }
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
                break;
            case 1:   // Continuous streaming
                success = m_adq_dev->ContinuousStreamingSetup(m_channels);
                break;
            case 2:   // Triggered streaming
                break;
            }
        }


    }
}


void ADQAIChannelGroup::onSwitchOn()
{
    // Enable all channels ------------------------> should be changed to "Enable chosen/needed channels"
    for (auto const& channel : m_AIChannels) {
        channel->setState(nds::state_t::on);
    }
}

void ADQAIChannelGroup::onSwitchOff()
{
    // Disable all channels 
    for (auto const& channel : m_AIChannels) {
        channel->setState(nds::state_t::off);
    }
}

void ADQAIChannelGroup::onStart()
{

}

void ADQAIChannelGroup::onStop()
{

}

void ADQAIChannelGroup::recover()
{
    throw nds::StateMachineRollBack("Cannot recover");
}

bool ADQAIChannelGroup::allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal)
{
    return true;
}