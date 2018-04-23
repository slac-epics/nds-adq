#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adq_dev(adq_dev),
    m_trigmode(0),
    m_trigmodeChanged(true),
/*   m_productNamePV(nds::PVDelegateIn<std::string>("ProductName", std::bind(&ADQAIChannelGroup::getProductName,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_serialNumberPV(nds::PVDelegateIn<std::string>("SerialNumber", std::bind(&ADQAIChannelGroup::getSerialNumber,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_productIDPV(nds::PVDelegateIn<std::int32_t>("ProductID", std::bind(&ADQAIChannelGroup::getProductID,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_adqTypePV(nds::PVDelegateIn<std::int32_t>("ADQType", std::bind(&ADQAIChannelGroup::getADQType,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_cardOptionPV(nds::PVDelegateIn<std::string>("CardOption", std::bind(&ADQAIChannelGroup::getCardOption,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))), */
    m_trigmodePV(nds::PVDelegateIn<std::int32_t>("TriggerMode-RB", std::bind(&ADQAIChannelGroup::getTriggerMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))) 

{ 
    parentNode.addChild(m_node);

    n_of_chan = m_adq_dev->GetNofChannels();
/*    
    // Set PVs for device info
    m_productNamePV.setScanType(nds::scanType_t::interrupt);
    m_productNamePV.setMaxElements(32);
    m_node.addChild(m_productNamePV);
    
    m_serialNumberPV.setScanType(nds::scanType_t::interrupt);
    m_serialNumberPV.setMaxElements(32);
    m_node.addChild(m_serialNumberPV);

    m_productIDPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_productIDPV);

    m_adqTypePV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_adqTypePV);

    m_cardOptionPV.setScanType(nds::scanType_t::interrupt);
    m_cardOptionPV.setMaxElements(32);
    m_node.addChild(m_cardOptionPV);
*/
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

    // PVs for state machine.
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
/*
void ADQAIChannelGroup::getProductName(timespec* pTimestamp, std::string* pValue)
{
    char* adq_pn = m_adq_dev->GetBoardProductName();
    *pValue = std::string(adq_pn);
}

void ADQAIChannelGroup::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    char* adq_sn = m_adq_dev->GetBoardSerialNumber();
    *pValue = std::string(adq_sn);
}

void ADQAIChannelGroup::getProductID(timespec* pTimestamp, std::int32_t* pValue)
{
    unsigned int adq_pid = m_adq_dev->GetProductID();
    *pValue = std::int32_t(adq_pid);
}

void ADQAIChannelGroup::getADQType(timespec* pTimestamp, std::int32_t* pValue)
{
    int adq_type = m_adq_dev->GetADQType();
    *pValue = std::int32_t(adq_type);
}

void ADQAIChannelGroup::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    const char* adq_opt = m_adq_dev->GetCardOption();
    *pValue = std::string(adq_opt);
}
*/
void ADQAIChannelGroup::setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    // Don't use dbpf TriggerMode.PROC 1 after putting a value to TRIGMODE record, because it has SCAN=PASSIVE like TRIGMODE itself
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);

    m_trigmode = pValue + std::int32_t(1);
    m_trigmodeChanged = true;
    success = m_adq_dev->SetTriggerMode(m_trigmode);
    if (success)
    {
        std::int32_t tmp;
        m_trigmodePV.read(&now, &tmp);
        m_trigmodePV.push(now, tmp);
        std::cout << "Trigger Mode # " << tmp << std::endl;
    }
    else
    {
        std::cout << "SetTrigerMode failed" << std::endl;
    }
}

void ADQAIChannelGroup::getTriggerMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigmode - std::int32_t(1);
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