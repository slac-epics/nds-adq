#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQAIChannel::ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum, ADQInterface *& adq_dev) :
    m_channelNum(channelNum),
    m_adq_dev(adq_dev),
    m_dataPV(nds::PVDelegateIn<std::vector<uint8_t>>("Data", std::bind(&ADQAIChannel::readback,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    m_node = parentNode.addChild(nds::Node(name));

    // PV for data
    m_dataPV.setScanType(nds::scanType_t::interrupt);
    m_dataPV.setMaxElements(2000);
    m_node.addChild(m_dataPV);

    // PV for state machine
    m_stateMachine = nds::StateMachine(true, std::bind(&ADQAIChannel::switchOn, this),
                                             std::bind(&ADQAIChannel::switchOff, this),
                                             std::bind(&ADQAIChannel::start, this),
                                             std::bind(&ADQAIChannel::stop, this),
                                             std::bind(&ADQAIChannel::recover, this),
                                             std::bind(&ADQAIChannel::allowChange, 
                                                 this,
                                                 std::placeholders::_1,
                                                 std::placeholders::_2,
                                                 std::placeholders::_3));

    m_node.addChild(m_stateMachine);

    commitChanges();

 //  int SetLvlTrigChannel(int channel);

}

void ADQAIChannel::commitChanges(bool calledFromAcquisitionThread)
{
    if (!calledFromAcquisitionThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) {
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
    throw nds::StateMachineRollBack("Cannot recover");
}

bool ADQAIChannel::allowChange(const nds::state_t, const nds::state_t, const nds::state_t)
{
    return true;
}

void ADQAIChannel::read(void* data, int32_t samples)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    if (m_stateMachine.getLocalState() != nds::state_t::running)
    {
        // Print the warning message the first time this function is inappropriately called 
        if (m_firstReadout) {
            m_firstReadout = false;
            ndsInfoStream(m_node) << "Channel " << m_channelNum << " is not running." << std::endl;
        }
        return;
    }

    short data_ch;
    m_data.clear();
    m_data.reserve(sizeof(samples));
    std::vector<uint8_t>::iterator target = m_data.begin();

    data_ch = ((uint8_t*)data)[m_channelNum];

    m_data.resize(sizeof(samples));

    for (target; target != m_data.end(); ++target)
    {
        *target = data_ch;
    }

    m_dataPV.push(now, m_data);

    commitChanges(true);
}

void ADQAIChannel::readback(timespec* pTimestamp, std::vector<uint8_t>* pValue)
{

}