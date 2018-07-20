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
#include "ADQInit.h"

ADQAIChannel::ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum) :
    m_channelNum(channelNum),
    m_inputRangePV(nds::PVDelegateIn<double>("InputRange-RB", std::bind(&ADQAIChannel::getInputRange, this,
                                                                        std::placeholders::_1, std::placeholders::_2))),
    m_dcBiasPV(nds::PVDelegateIn<int32_t>("DCBias-RB", std::bind(&ADQAIChannel::getDcBias, this, std::placeholders::_1,
                                                                 std::placeholders::_2))),
    m_dataPV(nds::PVDelegateIn<std::vector<int32_t>>("Data", std::bind(&ADQAIChannel::getDataPV, this,
                                                                       std::placeholders::_1, std::placeholders::_2)))
{
    m_node = parentNode.addChild(nds::Node(name));

    // PV for input range
    nds::PVDelegateOut<double> nodeFloat(nds::PVDelegateOut<double>(
                                       "InputRange",
                                       std::bind(&ADQAIChannel::setInputRange, this, std::placeholders::_1, std::placeholders::_2),
                                       std::bind(&ADQAIChannel::getInputRange, this, std::placeholders::_1, std::placeholders::_2)));
    m_node.addChild(nodeFloat);
    m_inputRangePV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_inputRangePV);

    // PVs for Adjustable Bias
    nds::PVDelegateOut<int32_t> node(nds::PVDelegateOut<int32_t>(
                                       "DCBias",
                                       std::bind(&ADQAIChannel::setDcBias, this, std::placeholders::_1, std::placeholders::_2),
                                       std::bind(&ADQAIChannel::getDcBias, this, std::placeholders::_1, std::placeholders::_2)));

    m_node.addChild(node);
    m_dcBiasPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dcBiasPV);

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
}

void ADQAIChannel::setInputRange(const timespec& pTimestamp, const double& pValue)
{
    m_inputRange = pValue;
    m_inputRangePV.getTimestamp() = pTimestamp;
    m_inputRangeChanged = true;
}

void ADQAIChannel::getInputRange(timespec* pTimestamp, double* pValue)
{
    *pValue = m_inputRange;
    *pTimestamp = m_inputRangePV.getTimestamp();
}

void ADQAIChannel::setDcBias(const timespec& pTimestamp, const int32_t& pValue)
{
    m_dcBias = pValue;
    m_dcBiasPV.getTimestamp() = pTimestamp;
    m_dcBiasChanged = true;
}

void ADQAIChannel::getDcBias(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_dcBias;
    *pTimestamp = m_dcBiasPV.getTimestamp();
}

/* This function updates readback PVs according to changed each channel's parameter
 * and applies them to the device with ADQAPI functions.
 */
void ADQAIChannel::commitChanges(bool calledFromDaqThread, ADQInterface*& adqInterface)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    unsigned int status = 0;

    if (!calledFromDaqThread &&
        (m_stateMachine.getLocalState() != nds::state_t::on && m_stateMachine.getLocalState() != nds::state_t::stopping &&
         m_stateMachine.getLocalState() != nds::state_t::initializing))
    {
        return;
    }

    if (m_inputRangeChanged)
    {
        float inputRangeRb = 0;
        m_inputRangeChanged = false;

        int adqType = adqInterface->GetADQType();
        std::string adqOption = adqInterface->GetCardOption();

        if ((adqType == 714 || adqType == 14) && (adqOption.find("-VG") != std::string::npos))
        {
            status = adqInterface->HasAdjustableInputRange();
            if (status)
            {
                if (m_inputRange <= 0)
                {
                    m_inputRange = 500;
                    ndsInfoStream(m_node) << "INFO: Input range is set to 0.5 Vpp by default, CH" << m_channelNum << std::endl;
                }

                if ((m_inputRange > 0) || (m_inputRange < 0.5))
                    m_inputRange = 200;
                if ((m_inputRange > 0.5) || (m_inputRange < 1))
                    m_inputRange = 1000;
                if ((m_inputRange > 1) || (m_inputRange < 2))
                    m_inputRange = 2000;
                if ((m_inputRange > 2) || (m_inputRange < 5) || (m_inputRange > 5))
                    m_inputRange = 5000;

                status = adqInterface->SetInputRange(m_channelNum + 1, m_inputRange, &inputRangeRb);
                if (status)
                {
                    status = adqInterface->GetInputRange(m_channelNum + 1, &inputRangeRb);
                    if (status)
                        m_inputRangePV.push(now, (double)inputRangeRb);
                }
                else
                {
                    ndsWarningStream(m_node) << "WARNING: SetInputRange failed, CH" << m_channelNum << std::endl;
                }
            }
            else
            {
                ndsWarningStream(m_node) << "WARNING: Device doesn't support adjustable input range." << std::endl;
            }
        }
    }

    if (m_dcBiasChanged)
    {
        m_dcBiasChanged = false;

        if (adqInterface->HasAdjustableBias())
        {
            status = adqInterface->SetAdjustableBias(m_channelNum + 1, m_dcBias);
            sleep(1000);

            if (!status)
            {
                ndsWarningStream(m_node) << "WARNING: SetAdjustableBias failed on CH" << m_channelNum << std::endl;
            }
            else
            {
                int dcBias = 0;
                status = adqInterface->GetAdjustableBias(m_channelNum + 1, &dcBias);
                m_dcBias = dcBias;
                m_dcBiasPV.push(now, m_dcBias);
            }
        }
        else
        {
            ndsInfoStream(m_node) << "INFO: Device doesn't support adjustable bias." << std::endl;
        }
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
}

void ADQAIChannel::recover()
{
    throw nds::StateMachineRollBack("INFO: Cannot recover");
}

bool ADQAIChannel::allowChange(const nds::state_t, const nds::state_t, const nds::state_t)
{
    return true;
}

/* @brief Dummy method for the m_dataPV. The readData method pushes data to m_dataPV. @ref readData
 */
void ADQAIChannel::getDataPV(timespec* pTimestamp, std::vector<int32_t>* pValue)
{
    UNUSED(pTimestamp);
    UNUSED(pValue);
}

/* @brief This method creates a vector (waveform) with acquired data and pushes it to m_dataPV of each channel.
 * @param 
 */
void ADQAIChannel::readData(short* rawData, int32_t sampleCnt)
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

    m_data.clear();
    m_data.resize(sampleCnt);
    std::vector<int32_t>::iterator target = m_data.begin();

    for (int i = 0; i < sampleCnt; ++i, ++target)
    {
        *target = rawData[i];
    }

    m_dataPV.push(now, m_data);
}
