#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

class ADQAIChannel
{
public:
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum);

    int32_t m_channelNum;

    void setState(nds::state_t newState);

    //// urojec L3: maybe make these methods a bit more descriptive. It is
    ////            a little bit hard to know what they do just from looking at
    ////            the name
    void readData(short* rawData, std::int32_t sampleCnt);
    void getDataPV(timespec* pTimestamp, std::vector<double>* pValue);

    void commitChanges(bool calledFromDaqThread = false);

private:
    nds::Node m_node;
    nds::StateMachine m_stateMachine;
    std::vector<double> m_data;

    bool m_firstReadout;

    void switchOn();
    void switchOff();
    void start();
    void stop();
    void recover();
    bool allowChange(const nds::state_t, const nds::state_t, const nds::state_t);

    //nds::PVDelegateIn<std::vector<uint8_t>> m_dataPV;
    nds::PVDelegateIn<std::vector<double>> m_dataPV;
};

#endif /* ADQAICHANNEL_H; */
