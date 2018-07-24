#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

/*! @brief This class handles channel specific parameters and pushes acquired data to appropriate data PVs.
 *
 * @param name a name with which this class will register its child node.
 * @param parentNode a name of a parent node to which this class' node is a child.
 * @param channelNum a number of channel which a constructed class represents.
 */
class ADQAIChannel
{
public:
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum);

    int32_t m_channelNum;

    void setInputRange(const timespec& pTimestamp, const double& pValue);
    void getInputRange(timespec* pTimestamp, double* pValue);

    void setDcBias(const timespec& pTimestamp, const int32_t& pValue);
    void getDcBias(timespec* pTimestamp, int32_t* pValue);

    void setState(nds::state_t newState);

    /*! @brief This method passes the acquired data to appropriate data PV.
     */
    void readData(short* rawData, int32_t sampleCnt);
    void getDataPV(timespec* pTimestamp, std::vector<int32_t>* pValue);

    /*! @brief This method processes changes applied to channel specific parameters.
     *
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
    nds::PVDelegateIn<std::vector<int32_t>> m_dataPV;
};

#endif /* ADQAICHANNEL_H; */
