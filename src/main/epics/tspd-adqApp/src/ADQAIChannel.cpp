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


//// urojec L3: most editors provide a visual line that you can set so that you
////         go into newline and make the code more readable
ADQAIChannel::ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum, ADQInterface *& adq_dev) :
    m_channelNum(channelNum),
    m_adq_dev(adq_dev),
    m_dataPV(nds::PVDelegateIn<std::vector<double>>("Data", std::bind(&ADQAIChannel::readback,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    m_node = parentNode.addChild(nds::Node(name));

    // PV for data
    m_dataPV.setScanType(nds::scanType_t::interrupt);
    //// urojec L2: no hardcoading of values, either use a macro or a static const variable
    ////            defined in the class
    m_dataPV.setMaxElements(30000000);
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

void ADQAIChannel::readback(timespec* pTimestamp, std::vector<double>* pValue)    // Dummy method for the DATA PVs; methods eith name "read_***" are actual methods that push received data to PVs
{

}

void ADQAIChannel::read_trigstr(short* rawdata, std::int32_t total_samples)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    if (m_stateMachine.getLocalState() != nds::state_t::running)
    {
        // Print the warning message the first time this function is inappropriately called
        //// urojec L3: this comment is a little bit confusing. It can mean more things 
        if (m_firstReadout) {
            m_firstReadout = false;
            ndsInfoStream(m_node) << "Channel " << m_channelNum << " is not running." << std::endl;
        }
        return;
    }

    //double* data_ch;
    int i; //// urojec L2: when using this just as a for loop iterator define it in the loop statement
    m_data.clear();
    m_data.reserve(total_samples);
    std::vector<double>::iterator target = m_data.begin();

    //// urojec L1: why is both reserve and resize needed here
    m_data.resize(total_samples);

    //// urojec L2: it this ++ intentionally positioned before i? Do you know what is the difference with before and after?
    for (i = 0; i < total_samples; ++i, ++target)
    {
        *target = ((double)rawdata[i]);
    }

    ////
    m_dataPV.push(now, m_data);

    commitChanges(true);
}


void ADQAIChannel::read_multirec(void* rawdata, std::int32_t total_samples)
{/*
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

    double data_ch;
    int i;

    data_ch = *(double*)rawdata;
    m_data.clear();
    m_data.reserve(total_samples);
    std::vector<double>::iterator target = m_data.begin();

    m_data.resize(total_samples);

    for (i = 0; i < total_samples; ++i, ++target)
    {
        *target = data_ch[i];
    }

    m_dataPV.push(now, m_data);

    commitChanges(true); */
}

void ADQAIChannel::read_contstr(void* rawdata, std::int32_t total_samples)
{

}
