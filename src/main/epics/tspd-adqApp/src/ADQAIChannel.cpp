#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"
#include "ADQFourteen.h"
#include "ADQInfo.h"
#include "ADQSeven.h"

ADQAIChannel::ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum) :
    m_channelNum(channelNum),
    m_dataPV(nds::PVDelegateIn<std::vector<int8_t>>("Data", std::bind(&ADQAIChannel::getDataPV, this,
                                                                      std::placeholders::_1, std::placeholders::_2)))
{
    m_node = parentNode.addChild(nds::Node(name));

    // PV for data
    m_dataPV.setScanType(nds::scanType_t::interrupt);
    m_dataPV.setMaxElements(DATA_MAX_ELEMENTS);
    m_node.addChild(m_dataPV);

    // PV for state machine
    m_stateMachine = nds::StateMachine(true,
                                       std::bind(&ADQAIChannel::switchOn, this),
                                       std::bind(&ADQAIChannel::switchOff, this),
                                       std::bind(&ADQAIChannel::start, this),
                                       std::bind(&ADQAIChannel::stop, this),
                                       std::bind(&ADQAIChannel::recover, this),
                                       std::bind(&ADQAIChannel::allowChange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_node.addChild(m_stateMachine);

    commitChanges();
}

void ADQAIChannel::commitChanges(bool calledFromDaqThread)
{
    if (!calledFromDaqThread &&
        (m_stateMachine.getLocalState() != nds::state_t::on && m_stateMachine.getLocalState() != nds::state_t::stopping &&
         m_stateMachine.getLocalState() != nds::state_t::initializing))
    {
        return;
    }
}

void ADQAIChannel::setState(nds::state_t newState)
{
    m_stateMachine.setState(newState);
}

void ADQAIChannel::switchOn()
{
}
void ADQAIChannel::switchOff()
{
}

void ADQAIChannel::start()
{
    m_firstReadout = true;
}

void ADQAIChannel::stop()
{
    commitChanges();
}

void ADQAIChannel::recover()
{
    throw nds::StateMachineRollBack("INFO: Cannot recover");
}

bool ADQAIChannel::allowChange(const nds::state_t, const nds::state_t, const nds::state_t)
{
    return true;
}

void ADQAIChannel::getDataPV(timespec* pTimestamp, std::vector<int8_t>* pValue)
{
    /* Dummy method for the m_dataPV;
     * methods starting with "read" are actual methods that push received data to PV
     */
}

void ADQAIChannel::readData(short* rawData, std::int32_t sampleCnt)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    if (m_stateMachine.getLocalState() != nds::state_t::running)
    {
        // Print the warning message the first time this function is inappropriately called                            !!!!!!!!!!!!!!!!!!!
        //// urojec L3: this comment is a little bit confusing. It can mean more things
        if (m_firstReadout)
        {
            m_firstReadout = false;
            ndsInfoStream(m_node) << "Channel " << m_channelNum << " is not running." << std::endl;
        }
        return;
    }

    //double* data_ch;
    m_data.clear();
    m_data.reserve(sampleCnt);
    std::vector<int8_t>::iterator target = m_data.begin();

    //// urojec L1: why is both reserve and resize needed here
    //m_data.resize(sampleCnt);
    ndsInfoStream(m_node) << "INFO: Sending data to PV..." << std::endl;
    for (int i = 0; i < sampleCnt; ++i, ++target)
    {
        *target = (short)rawData[i];
    }

    m_dataPV.push(now, m_data);

    commitChanges(true);
}
