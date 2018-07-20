#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

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

    void readData(short* rawData, int32_t sampleCnt);
    void getDataPV(timespec* pTimestamp, std::vector<int32_t>* pValue);

    void commitChanges(bool calledFromDaqThread, ADQInterface*& adqInterface);

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
