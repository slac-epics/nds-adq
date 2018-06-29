#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>
#include <time.h>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQFourteen.h"
#include "ADQDevice.h"
#include "ADQDefinition.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQFourteen::ADQFourteen(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev) : 
    ADQAIChannelGroup(name + GROUP_CHAN_DEVICE, parentNode, adqDev),
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
    m_trigChanPV(nds::PVDelegateIn<std::int32_t>("TrigChan-RB", std::bind(&ADQFourteen::getTrigChan,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_overVoltProtectPV(nds::PVDelegateIn<std::int32_t>("OverVoltProtect-RB", std::bind(&ADQFourteen::getOverVoltProtect,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);

    // PVs for setting active channels
    nds::enumerationStrings_t chanMaskList;
    if (m_chanCnt == 4)
    {
        chanMaskList = { "A", "B", "C", "D", "A+B", "C+D", "A+B+C+D" };
    }
    if (m_chanCnt == 2)
    {
        chanMaskList = { "A", "B", "A+B" };
    }
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

    m_chanMaskPV.setScanType(nds::scanType_t::interrupt);
    m_chanMaskPV.setMaxElements(4);
    m_node.addChild(m_chanMaskPV);

    // PVs for trigger channel  
    nds::enumerationStrings_t trigChanList = { "None", "A", "B", "C", "D", "A+B", "C+D", "A+B+C+D" };
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

    // PVs for input range for channels (ADQ14-VG only)
    node = nds::PVDelegateOut<std::int32_t>("OverVoltProtect", std::bind(&ADQFourteen::setOverVoltProtect,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2),
                                                        std::bind(&ADQFourteen::getOverVoltProtect,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));
    m_node.addChild(node);

    m_overVoltProtectPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_overVoltProtectPV);

    commitChangesSpec();
}

void ADQFourteen::setChanActive(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_chanActive = pValue;
    m_chanActiveChanged = true;
    commitChangesSpec();
}

void ADQFourteen::getChanActive(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_chanActive;
}

void ADQFourteen::getChanMask(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_chanMask;
}

void ADQFourteen::setTrigChan(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigChan = pValue;
    m_trigChanChanged = true;
    commitChangesSpec();
}

void ADQFourteen::getTrigChan(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigChan;
}

void ADQFourteen::setOverVoltProtect(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_overVoltProtect = pValue;
    m_overVoltProtectChanged = true;
    commitChangesSpec();
}

void ADQFourteen::getOverVoltProtect(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_overVoltProtect;
}

void ADQFourteen::commitChangesSpec(bool calledFromDaqThread)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);

    if (!calledFromDaqThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) 
    {
        return;
    }

    if (m_chanActiveChanged)
    {
        m_chanActiveChanged = false;

        if (!m_chanCnt)
        {
            int success = 0;
            ADQNDS_MSG_WARNLOG(success, "WARNING: No channels are found.");
        }
        else
        {
            if (m_chanCnt == 2)
            {
                switch (m_chanActive)
                {
                case 0: // ch A
                    m_chanMask = 0x1;
                    m_chanInt = 1;
                    break;
                case 1: // ch B
                    m_chanMask = 0x2;
                    m_chanInt = 2;
                    break;
                case 2: // ch A+B
                    m_chanMask = 0x3;
                    m_chanInt = 3;
                    break;
                }
            }

            if (m_chanCnt == 4)
            {
                switch (m_chanActive)
                {
                case 0: // ch A
                    m_chanMask = 0x1;
                    m_chanInt = 1;
                    break;
                case 1: // ch B
                    m_chanMask = 0x2;
                    m_chanInt = 2;
                    break;
                case 2: // ch C
                    m_chanMask = 0x4;
                    m_chanInt = 4;
                    break;
                case 3: // ch D
                    m_chanMask = 0x8;
                    m_chanInt = 8;
                    break;
                case 4: // ch A+B
                    m_chanMask = 0x3;
                    m_chanInt = 3;
                    break;
                case 5: // ch C+D
                    m_chanMask = 0xC;
                    m_chanInt = 12;
                    break;
                case 6: // ch A+B+C+D
                    m_chanMask = 0xF;
                    m_chanInt = 15;
                    break;
                }
            }   
        }

        m_chanMaskPV.push(now, m_chanMask);
        m_chanActivePV.push(now, m_chanActive);
    }

    if (m_trigChanChanged && (m_trigMode == 2))
    {
        m_trigChanChanged = false;

        if (m_chanCnt == 2)
        {
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
            case 3: // ch A+B
                m_trigChanInt = 3;
                break;
            }
        }

        if (m_chanCnt == 4)
        {
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
            case 3: // ch C
                m_trigChanInt = 4;
                break;
            case 4: // ch D
                m_trigChanInt = 8;
                break;
            case 5: // ch A+B
                m_trigChanInt = 3;
                break;
            case 6: // ch C+D
                m_trigChanInt = 12;
                break;
            case 7: // ch A+B+C+D
                m_trigChanInt = 15;
                break;
            }
        }

        m_trigChanPV.push(now, m_trigChan);
    }

    if (m_overVoltProtectChanged)
    {
        m_overVoltProtectChanged = false;

        if (m_overVoltProtect > 1)
            m_overVoltProtect = 1;
        if (m_overVoltProtect < 0)
            m_overVoltProtect = 0;

        m_overVoltProtectPV.push(now, m_overVoltProtect);
    }
}

ADQFourteen::~ADQFourteen()
{

}