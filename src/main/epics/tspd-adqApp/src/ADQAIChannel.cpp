#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQFourteen.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"


//// urojec L3: most editors provide a visual line that you can set so that you                                              !!!!!!!!!!!!!!!!
////         go into newline and make the code more readable
ADQAIChannel::ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum) :
    m_channelNum(channelNum),
    m_dataPV(nds::PVDelegateIn<std::vector<double>>("Data", std::bind(&ADQAIChannel::getDataPV,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    m_node = parentNode.addChild(nds::Node(name));

    // PV for data
    m_dataPV.setScanType(nds::scanType_t::interrupt);
    //// urojec L2: no hardcoading of values, either use a macro or a static const variable                                 ??????????????????
    ////            defined in the class
    m_dataPV.setMaxElements(DATA_MAX_ELEMENTS);
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
}

void ADQAIChannel::commitChanges(bool calledFromDaqThread)
{
    if (!calledFromDaqThread && (
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

void ADQAIChannel::getDataPV(timespec* pTimestamp, std::vector<double>* pValue)
{
    /* Dummy method for the m_dataPV;
     * methods starting with "read" are actual methods that push received data to PV
     */
}

void ADQAIChannel::readTrigStream(short* rawData, std::int32_t sampleCntTotal)
{
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    if (m_stateMachine.getLocalState() != nds::state_t::running)
    {
        // Print the warning message the first time this function is inappropriately called                            !!!!!!!!!!!!!!!!!!!
        //// urojec L3: this comment is a little bit confusing. It can mean more things 
        if (m_firstReadout) {
            m_firstReadout = false;
            ndsInfoStream(m_node) << "Channel " << m_channelNum << " is not running." << std::endl;
        }
        return;
    }

    //double* data_ch;
    m_data.clear();
    m_data.reserve(sampleCntTotal);
    std::vector<double>::iterator target = m_data.begin();

    //// urojec L1: why is both reserve and resize needed here
    m_data.resize(sampleCntTotal);

    //// urojec L2: it this ++ intentionally positioned before i? Do you know what is the difference with before and after?
    ////// ppipp: there is a difference between postfix and prefix, but it doesn't affect the process of for-loops; 
    //////        also i've read it is safer to use prefix and in general it is a good tone
    for (int i = 0; i < sampleCntTotal; ++i, ++target)
    {
        *target = ((double)rawData[i]);
    }

    ////
    m_dataPV.push(now, m_data);

    commitChanges(true);
}


void ADQAIChannel::readMultiRecord(void* rawData, std::int32_t sampleCntTotal)
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

    data_ch = *(double*)rawData;
    m_data.clear();
    m_data.reserve(sampleCntTotal);
    std::vector<double>::iterator target = m_data.begin();

    m_data.resize(sampleCntTotal);

    for (i = 0; i < sampleCntTotal; ++i, ++target)
    {
        *target = data_ch[i];
    }

    m_dataPV.push(now, m_data);

    commitChanges(true); */
}

void ADQAIChannel::readContinStream(void* rawData, std::int32_t sampleCntTotal)
{

}
