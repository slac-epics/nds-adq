#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <time.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQFourteen.h"
#include "ADQSeven.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h" // ADQAIChannelGroup(COMMON_DEVICE, parentNode, adqDev),

ADQSeven::ADQSeven(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adqDevPtr(adqDev),
    m_chanActivePV(nds::PVDelegateIn<std::int32_t>("ChanActive-RB", std::bind(&ADQFourteen::getChanActive,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_chanMaskPV(nds::PVDelegateIn<std::string>("ChanMask-RB", std::bind(&ADQFourteen::getChanMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigLvlPV(nds::PVDelegateIn<std::int32_t>("TrigLevel-RB", std::bind(&ADQFourteen::getTrigLvl,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigEdgePV(nds::PVDelegateIn<std::int32_t>("TrigEdge-RB", std::bind(&ADQFourteen::getTrigEdge,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigChanPV(nds::PVDelegateIn<std::int32_t>("TrigChan-RB", std::bind(&ADQFourteen::getTrigChan,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    ADQAIChannelGroup(COMMON_DEVICE, parentNode, adqDev)
{
    parentNode.addChild(m_node);

    //std::shared_ptr<ADQAIChannelGroup> aiChanGrp = std::make_shared<ADQAIChannelGroup>("DAQ-COM", m_node, m_adqDevPtr);
    //m_AIChannelGroupPtr.push_back(aiChanGrp);

    // PVs for setting active channels
    nds::enumerationStrings_t chanMaskList = { "A", "B", "A+B" };
    nds::PVDelegateOut<std::int32_t> node(nds::PVDelegateOut<std::int32_t>("ChanActive", std::bind(&ADQFourteen::setChanActive,
                                                                                                    this,
                                                                                                    std::placeholders::_1,
                                                                                                    std::placeholders::_2),
                                                                                         std::bind(&ADQFourteen::getChanActive,
                                                                                                    this,
                                                                                                    std::placeholders::_1,
                                                                                                    std::placeholders::_2)));
    node.setEnumeration(chanMaskList);
    m_node.addChild(node);

    m_chanActivePV.setScanType(nds::scanType_t::interrupt);
    m_chanActivePV.setEnumeration(chanMaskList);
    m_node.addChild(m_chanActivePV);

    nds::PVDelegateOut<std::string> nodeStr("ChanMask", std::bind(&ADQFourteen::setChanMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                        std::bind(&ADQFourteen::getChanMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));

    m_node.addChild(nodeStr);

    m_chanMaskPV.setScanType(nds::scanType_t::interrupt);
    m_chanMaskPV.setMaxElements(4);
    m_node.addChild(m_chanMaskPV);

    //PVs for trigger level
    node = nds::PVDelegateOut<std::int32_t>("TrigLevel", std::bind(&ADQFourteen::setTrigLvl,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                            std::bind(&ADQFourteen::getTrigLvl,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));
    m_node.addChild(node);


    m_trigLvlPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_trigLvlPV);

    // PVs for trigger edge
    nds::enumerationStrings_t triggerEdgeList = { "Falling edge", "Rising edge" };
    node = nds::PVDelegateOut<std::int32_t>("TrigEdge", std::bind(&ADQFourteen::setTrigEdge,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2),
                                                           std::bind(&ADQFourteen::getTrigEdge,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));
    node.setEnumeration(triggerEdgeList);
    m_node.addChild(node);


    m_trigEdgePV.setScanType(nds::scanType_t::interrupt);
    m_trigEdgePV.setEnumeration(triggerEdgeList);
    m_node.addChild(m_trigEdgePV);

    // PVs for trigger channel  
    nds::enumerationStrings_t trigChanList = { "None", "A", "B" };
    node = nds::PVDelegateOut<std::int32_t>("TrigChan", std::bind(&ADQFourteen::setTrigChan,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2),
                                                              std::bind(&ADQFourteen::getTrigChan,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));
    node.setEnumeration(trigChanList);
    m_node.addChild(node);


    m_trigChanPV.setScanType(nds::scanType_t::interrupt);
    m_trigChanPV.setEnumeration(trigChanList);
    m_node.addChild(m_trigChanPV);

    commitChangesSpec();
}

void ADQSeven::setChanActive(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_chanActive = pValue;
    m_chanActiveChanged = true;
    commitChangesSpec();
}

void ADQSeven::getChanActive(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_chanActive;
}

void ADQSeven::setChanMask(const timespec &pTimestamp, const std::string &pValue)
{
    m_chanMask = pValue;
    commitChangesSpec();
}

void ADQSeven::getChanMask(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_chanMask;
}

void ADQSeven::setTrigLvl(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigLvl = pValue;
    m_trigLvlChanged = true;
    commitChangesSpec();
}

void ADQSeven::getTrigLvl(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigLvl;
}

void ADQSeven::setTrigEdge(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigEdge = pValue;
    m_trigEdgeChanged = true;
    commitChangesSpec();
}

void ADQSeven::getTrigEdge(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigEdge;
}

void ADQSeven::setTrigChan(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigChan = pValue;
    m_trigChanChanged = true;
    commitChangesSpec();
}

void ADQSeven::getTrigChan(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigChan;
}

void ADQSeven::commitChangesSpec(bool calledFromDaqThread)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    ADQAIChannelGroup* adqGrpPtr;

    if (!calledFromDaqThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) {
        return;
    }

    if (m_chanActiveChanged)     // Needs to be moved to ADQ classes (7 and 14 have different options)
    {
        if (!m_chanCnt)
        {
            ndsWarningStream(m_node) << "FAILURE: No channels are found." << std::endl;
        }
        else
        {
            switch (m_chanActive)
            {
            case 0: // ch A
                m_chanBits = 1000;
                m_chanMask = 0x1;
                break;
            case 1: // ch B
                m_chanBits = 0100;
                m_chanMask = 0x2;
                break;
            case 2: // ch A+B
                m_chanBits = 1100;
                m_chanMask = 0x3;
                break;
            }
        }

        m_chanMaskPV.push(now, m_chanMask);
        m_chanActivePV.push(now, m_chanActive);
    }

    if (m_trigEdgeChanged)
    {
        m_trigEdgeChanged = false;
        m_trigEdgePV.push(now, m_trigEdge);
    }

    if (m_trigLvlChanged)
    {
        m_trigLvlChanged = false;
        m_trigLvlPV.push(now, m_trigLvl);
    }

    if (m_trigChanChanged)
    {
        m_trigChanChanged = false;

        switch (m_trigChan)
        {
        case 0: // None
            m_trigChanInt = 0;
            break;
        case 1: // ch A
            m_trigChanInt = 1;
            break;
        case 2: // ch B
            m_trigChanInt = 2;
            break;
        }

        m_trigChanPV.push(now, m_trigChan);
    }

    commitChanges();
}