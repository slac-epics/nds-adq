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
    m_trigmodeChanged(true),
    m_biasChanged(true),
    m_dbsChanged(true),
    m_pattmodeChanged(true),
    m_nofrecordsChanged(true),
    m_nofsamplesChanged(true),
    m_daqmodeChanged(true),
    m_channelmaskChanged(true),
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
    m_channelbitsPV(nds::PVDelegateIn<std::int32_t>("ChannelBits-RB", std::bind(&ADQAIChannelGroup::getChannels,
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
    node = nds::PVDelegateOut<std::int32_t>("ChannelBits", std::bind(&ADQAIChannelGroup::setChannels,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                       std::bind(&ADQAIChannelGroup::getChannels,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);

    m_channelbitsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_channelbitsPV);

    m_channelmaskPV.setScanType(nds::scanType_t::interrupt);
    m_channelmaskPV.setMaxElements(8);
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
    m_channelbits = pValue;
    m_channelmaskChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getChannels(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_channelbits;
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
    unsigned int i;

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
            i = 0;
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

        //m_adjustBiasPV.push(now, m_adjustBias);
    }

    if (m_dbsChanged)
    {
        m_dbsChanged = false;
        success = m_adq_dev->GetNofDBSInstances(&dbs_n_of_inst);
        if (success)
        {
            i = 0;
            for (dbs_inst = 0; dbs_inst < dbs_n_of_inst; ++dbs_inst)
            {
                success = m_adq_dev->SetupDBS(dbs_inst, m_dbs_settings[0], m_dbs_settings[1], m_dbs_settings[2], m_dbs_settings[3]);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: " << "Failed setting up DBS instance " << dbs_inst << std::endl;
                }
                else
                {
                    ++i;
                    ndsInfoStream(m_node) << "SUCCES: " << "DBS instance: " << dbs_inst << std::endl;
                }
            }
            sleep(1000);

            if (i == dbs_n_of_inst)
            {
                ndsInfoStream(m_node) << "SUCCESS: " << "All " << dbs_n_of_inst << " DBS instances are set up." << std::endl;
            }
        }

        //m_dbs_settingsPV.push(now, m_dbs_settings);
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
            ndsInfoStream(m_node) << "SUCCESS:" << "Pattern Mode is set to " << m_pattmode << std::endl;
        }

        //m_pattmodePV.push(now, m_pattmode);
    }

    if (m_channelmaskChanged)
    {
        m_channelmaskChanged = false;
        m_channelmask = 0x00;

        if (!nofchan)
        {
            ndsWarningStream(m_node) << "FAILURE: " << "No channels are found." << std::endl;
        }
        else
        {
            switch (m_channelbits)
            {
            case 1000: // ch A
                m_channelmask |= 0x01;
                break;
            case 0100: // ch B
                m_channelmask |= 0x02;
                break;
            case 0010: // ch C
                m_channelmask |= 0x04;
                break;
            case 0001: // ch D
                m_channelmask |= 0x08;
                break;
            case 1100: // ch A+B
                m_channelmask |= 0x03;
                break;
            case 0011: // ch C+D
                m_channelmask |= 0x12;
                break;
            case 1111: // ch A+B+C+D
                m_channelmask |= 0x15;
                break;
            }
            ndsInfoStream(m_node) << "Channelmask is set to " << m_channelmask << std::endl;
        }

        //m_channelmaskPV.push(now, m_channelmask);
        //m_channelbitsPV.push(now, m_channelbits);
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
            ndsWarningStream(m_node) << "FAILURE: " << "Trigger Mode was not set." << std::endl;
        }
        else
        {
            ndsInfoStream(m_node) << "SUCCESS:" << "Trigger Mode is set to" << m_trigmode << std::endl;

            success = m_adq_dev->GetMaxNofSamplesFromNofRecords(m_nofrecords, &max_nofsamples);
            if (!success)
            {
                ndsWarningStream(m_node) << "FAILURE: " << "Couldn't get the MAX number of samples (set number of records)." << std::endl;                
            }
            else
            {
                m_maxsamples = max_nofsamples;
                //m_maxsamplesPV.push(now, m_maxsamples);
                ndsInfoStream(m_node) << "SUCCESS:" << "Maximum number of samples is " << m_maxsamples << std::endl;

                if (m_nofsamples > m_maxsamples)
                {
                    ndsErrorStream(m_node) << "ERROR: " << "Chosen number of samples is higher than the MAX: " << m_nofsamples << ">" << m_maxsamples << std::endl;
                }
                else
                {
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
            }           
        }
        
        //m_nofrecordsPV.push(now, m_nofrecords);
        //m_nofsamplesPV.push(now, m_nofsamples);
       // m_daqmodePV.push(now, m_daqmode);
       // m_trigmodePV.push(now, m_trigmode);
    }
}


void ADQAIChannelGroup::onSwitchOn()
{
    // Enable all channels ------------------------> should be changed to "Enable chosen/needed channels"
    for (auto const& channel : m_AIChannels) 
    {
        channel->setState(nds::state_t::on);
    }
}

void ADQAIChannelGroup::onSwitchOff()
{
    // Disable all channels 
    for (auto const& channel : m_AIChannels) 
    {
        channel->setState(nds::state_t::off);
    }
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
    do
    {
        success = m_adq_dev->DisarmTrigger();
    } while (success == 0);

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
    int trigged;

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
                success = m_adq_dev->SWTrig();
                if (success)
                {
                    do
                    {
                        trigged = m_adq_dev->GetAcquiredAll();
                    } while (trigged == 0);

                    ndsInfoStream(m_node) << "All records are triggered." << std::endl;

                    ndsInfoStream(m_node) << "Creating buffers..." << std::endl;

                    m_buffersize = m_nofrecords * m_nofsamples;
                    
                    switch (m_channelbits)
                    {
                    case 1000: // ch A
                        buf_a = (short*)calloc(m_buffersize, sizeof(short));
                        m_target_buf[0] = (void*)buf_a;
                        break;
                    case 0100: // ch B
                        buf_b = (short*)calloc(m_buffersize, sizeof(short));
                        m_target_buf[1] = (void*)buf_b;
                        break;
                    case 0010: // ch C
                        buf_c = (short*)calloc(m_buffersize, sizeof(short));
                        m_target_buf[2] = (void*)buf_c;
                        break;
                    case 0001: // ch D
                        buf_d = (short*)calloc(m_buffersize, sizeof(short));
                        m_target_buf[3] = (void*)buf_d;
                        break;
                    case 1100: // ch A+B
                        buf_a = (short*)calloc(m_buffersize, sizeof(short));
                        buf_b = (short*)calloc(m_buffersize, sizeof(short));
                        m_target_buf[0] = (void*)buf_a;
                        m_target_buf[1] = (void*)buf_b;
                        break;
                    case 0011: // ch C+D
                        buf_c = (short*)calloc(m_buffersize, sizeof(short));
                        buf_d = (short*)calloc(m_buffersize, sizeof(short));
                        m_target_buf[2] = (void*)buf_c;
                        m_target_buf[3] = (void*)buf_d;
                        break;
                    case 1111: // ch A+B+C+D
                        buf_a = (short*)calloc(m_buffersize, sizeof(short));
                        buf_b = (short*)calloc(m_buffersize, sizeof(short));
                        buf_c = (short*)calloc(m_buffersize, sizeof(short));
                        buf_d = (short*)calloc(m_buffersize, sizeof(short));
                        m_target_buf[0] = (void*)buf_a;
                        m_target_buf[1] = (void*)buf_b;
                        m_target_buf[2] = (void*)buf_c;
                        m_target_buf[3] = (void*)buf_d;
                        break;
                    }

                    success = m_adq_dev->GetData(m_target_buf, m_buffersize, sizeof(short), 0, m_nofrecords, m_channelmask, 0, m_nofsamples, 0x00);
                    if (!success)
                    {
                        ndsErrorStream(m_node) << "ERROR: " << "Failed to transfer data (GetData)." << std::endl;
                    }
                    else
                    {
                        for (auto const& channel : m_AIChannels) {
                            channel->read(m_target_buf + channel->m_channelNum, m_nofsamples);
                        }
                    }

                    
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