//
// Copyright (c) 2018 Cosylab d.d.
// This software is distributed under the terms found
// in file LICENSE.txt that is included with this distribution.
//

#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

/** @file ADQAIChannel.h
 * @brief This file defines ADQAIChannel class.
 */

/** @class ADQAIChannel
 * @brief This class handles channel specific parameters and pushes acquired data to appropriate data PVs.
 */
class ADQAIChannel
{
public:
    /** @fn ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum);
     * @brief ADQAIChannel class constructor.
     * @param name a name with which this class will register its child node.
     * @param parentNode a name of a parent node to which this class' node is a child.
     * @param channelNum a number of channel which a constructed class represents.
     */
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum);

    /** @var m_channelNum
     * @brief Number of channel.
     */
    int32_t m_channelNum;

    /** @fn setInputRange
    * @brief Sets the channel's input range.
    */
    void setInputRange(const timespec& pTimestamp, const double& pValue);

    /** @fn getInputRange
     * @brief Gets the channel's input range.
     */
    void getInputRange(timespec* pTimestamp, double* pValue);

    /** @fn setDcBias
     * @brief Sets the channel's DC bias.
     */
    void setDcBias(const timespec& pTimestamp, const int32_t& pValue);

    /** @fn getDcBias
     * @brief Gets the channel's DC bias.
     */
    void getDcBias(timespec* pTimestamp, int32_t* pValue);

    /** @fn setChanDec
    * @brief Sets the channel's sample decimation.
    */
    void setChanDec(const timespec& pTimestamp, const int32_t& pValue);

    /** @fn getChanDec
    * @brief Gets the channel's sample decimation.
    */
    void getChanDec(timespec* pTimestamp, int32_t* pValue);

    /** @fn setState
     * @brief Sets a new state to the ADQAIChannel class' state machine.
     */
    void setState(nds::state_t newState);

    /** @fn readData
     * @brief This method passes the acquired data to appropriate data PV.
     */
    void readData(short* rawData, int32_t sampleCnt);

    /** @fn getDataPV
     * @brief This is a dummy method held by the data PV for appropriate work in NDS3. 
     */
    void getDataPV(timespec* pTimestamp, std::vector<int32_t>* pValue);

    /** @fn commitChanges
     * @brief This method processes changes are applied to channel specific parameters.
     * @param calledFromDaqThread a flag that prevents this function to be called when set to false.
     * @param adqInterface a pointer to the ADQ API interface created in the ADQInit class.
     * @param m_logMsgPV a PV from ADQAIGroupChannel that receives any log messages.
     */
    void commitChanges(bool calledFromDaqThread, ADQInterface*& adqInterface, nds::PVDelegateIn<std::string> m_logMsgPV);

private:
    nds::Node m_node;
    nds::StateMachine m_stateMachine;

    ADQInterface* m_adqInterface;

    double m_inputRange;
    bool m_inputRangeChanged;

    int32_t m_dcBias;
    bool m_dcBiasChanged;

    int32_t m_chanDec;
    bool m_chanDecChanged;

    std::vector<int32_t> m_data;

    bool m_firstReadout;

    void switchOn();
    void switchOff();
    void start();
    void stop();
    void recover();
    bool allowChange(const nds::state_t, const nds::state_t, const nds::state_t);

    nds::PVDelegateIn<double> m_inputRangePV;
    nds::PVDelegateIn<int32_t> m_dcBiasPV;
    nds::PVDelegateIn<int32_t> m_chanDecPV;
    nds::PVDelegateIn<std::vector<int32_t>> m_dataPV;
};

#endif /* ADQAICHANNEL_H; */
