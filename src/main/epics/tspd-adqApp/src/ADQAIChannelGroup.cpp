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
    m_dbs_bypass(0),
    m_dbs_dctarget(m_adjustBias),
    m_dbs_lowsat(0),
    m_dbs_upsat(0),
    m_trigmodePV(nds::PVDelegateIn<std::int32_t>("TriggerMode-RB", std::bind(&ADQAIChannelGroup::getTriggerMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_adjustBiasPV(nds::PVDelegateIn<std::int32_t>("AdjustBias-RB", std::bind(&ADQAIChannelGroup::getAdjustBias,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbsPV(nds::PVDelegateIn<std::int32_t>("DBS-RB", std::bind(&ADQAIChannelGroup::setDBS,
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
        std::cout << "Trigger Mode is set to " << m_trigmode << std::endl;
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
                ndsInfoStream(m_node) << "FAILURE: " << "Failed setting adjustable bias for channel " << tmp[ch] << std::endl;
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

void ADQAIChannelGroup::setDBS(const timespec &pTimestamp, const std::int32_t &pValue) // check pValue and decide how to connect all four m_ variables (gui access)
{
    success = m_adq_dev->GetNofDBSInstances(&dbs_n_of_inst);
    if (success)
    {
        for (dbs_inst = 0; dbs_inst < dbs_n_of_inst; ++dbs_inst)
        {
            success = m_adq_dev->SetupDBS(dbs_inst, m_dbs_bypass, m_dbs_dctarget, m_dbs_lowsat, m_dbs_upsat);
            if (!success)
            {
                ndsInfoStream(m_node) << "FAILURE: " << "Failed setting up DBS instance " << dbs_inst << std::endl;
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