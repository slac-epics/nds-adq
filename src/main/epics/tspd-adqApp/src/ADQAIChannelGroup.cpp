#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adq_dev(adq_dev),
    m_trigmode(0),
    m_trigmodeChanged(true),
    m_adjustBias(0),
    m_channelmask(0x00),
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
    m_channelmaskPV(nds::PVDelegateIn<std::string>("ChannelMask", std::bind(&ADQAIChannelGroup::getChanMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{ 
    parentNode.addChild(m_node);

    n_of_chan = m_adq_dev->GetNofChannels();

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
    nds::PVDelegateOut<std::string> node_vec(nds::PVDelegateOut<std::string>("ChannelMask", std::bind(&ADQAIChannelGroup::setChanMask,
                                                                                                                         this,
                                                                                                                         std::placeholders::_1,
                                                                                                                         std::placeholders::_2),
                                                                                                                        std::bind(&ADQAIChannelGroup::getChanMask,
                                                                                                                         this,
                                                                                                                         std::placeholders::_1,
                                                                                                                         std::placeholders::_2)));
  //  node_vect.setMaxElements(4);
    m_node.addChild(node_vect);

    m_channelmaskPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_channelmaskPV);

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
    // Don't use dbpf TriggerMode.PROC 1 after putting a value to TRIGMODE record, because it has SCAN=PASSIVE like TRIGMODE itself
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);

    m_trigmode = pValue + 1;
    success = m_adq_dev->SetTriggerMode(m_trigmode);
    if (success)
    {
        int tmp;
        m_trigmodePV.read(&now, &tmp);
        m_trigmodePV.push(now, tmp);
        m_trigmodeChanged = true;
            ndsInfoStream(m_node) << "Trigger Mode is set to " << m_trigmode << std::endl;
    }
}

void ADQAIChannelGroup::getTriggerMode(timespec* pTimestamp, std::int32_t* pValue)
{
    int tmp;
    tmp = m_adq_dev->GetTriggerMode();
    *pValue = tmp - 1;
}

void ADQAIChannelGroup::setAdjustBias(const timespec &pTimestamp, const std::int32_t &pValue) // Setting ADC offset requires Sleep(1000) for proper setting time
{
    m_adjustBias = pValue;
    success = m_adq_dev->HasAdjustableBias();
    if (success)
    {
        for (ch = 0; ch < n_of_chan; ++ch)
        {
            success = m_adq_dev->SetAdjustableBias(ch+1, m_adjustBias);
            if (!success)
            {
                const char tmp[5] = "ABCD";
                ndsWarningStream(m_node) << "FAILURE: " << "Failed setting adjustable bias for channel " << tmp[ch] << std::endl;
            }
            else
            {
                m_biasChanged = true;
            }
        }
    }
}

void ADQAIChannelGroup::getAdjustBias(timespec* pTimestamp, std::int32_t* pValue)    
{
    success = m_adq_dev->HasAdjustableBias();
    if (success)
    {
        for (ch = 0; ch < n_of_chan; ++ch)
        {
            int tmp;
            success = m_adq_dev->GetAdjustableBias(ch+1, &tmp);
            *pValue = tmp;
        }
    }
}

void ADQAIChannelGroup::setDBS(const timespec &pTimestamp, const std::vector<std::int32_t> &pValue)
{
    m_dbs_settings = pValue;
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
        }
    }
}

void ADQAIChannelGroup::getDBS(timespec* pTimestamp, std::vector<std::int32_t>* pValue)
{
    *pValue = m_dbs_settings;
    ndsInfoStream(m_node) << "DBS settings: " << &pValue[0] << " " << &pValue[1] << " " << &pValue[2] << " " << &pValue[3] << std::endl;
}

void ADQAIChannelGroup::setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_pattmode = pValue;
    success = m_adq_dev->SetTestPatternMode(m_pattmode);
    if (!success)
    {
        ndsWarningStream(m_node) << "FAILURE: " << "Failed setting up pattern mode." << std::endl;
    }
}

void ADQAIChannelGroup::getPatternMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_pattmode;
    ndsInfoStream(m_node) << "Pattern Mode is set to " << m_pattmode << std::endl;
}

void ADQAIChannelGroup::setChanMask(const timespec &pTimestamp, const std::string &pValue)
{
    m_channels = pValue;
    if (n_of_chan > 0)
    {
        if (m_channels[0] == 1)
        {
            m_channelmask |= 0x01;

            if (m_channels[1] == 1)
            {
                m_channelmask |= 0x02;
                
                if (m_channels[2] == 1)
                {
                    m_channelmask |= 0x04;

                    if (m_channels[3] == 1)
                    {
                        m_channelmask |= 0x08;
                    }
                }
            }
        }
        m_channelmaskChanged = true;
    }
    else
    {
        ndsWarningStream(m_node) << "FAILURE: " << "No channels are found." << std::endl;
    }
}

void ADQAIChannelGroup::getChanMask(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_channelmask;
    ndsInfoStream(m_node) << "Channelmask is set to " << pValue << std::endl;
}

void ADQAIChannelGroup::commitChanges()
{

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