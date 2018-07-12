#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

class ADQAIChannel
{
public:
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum);

    int32_t m_channelNum;

    void setState(nds::state_t newState);

    void readData(short* rawData, int32_t sampleCnt);
    void getDataPV(timespec* pTimestamp, std::vector<int32_t>* pValue);

    void commitChanges(bool calledFromDaqThread = false);

private:
    nds::Node m_node;
    nds::StateMachine m_stateMachine;
    std::vector<int32_t> m_data;

    bool m_firstReadout;

    void switchOn();
    void switchOff();
    void start();
    void stop();
    void recover();
    bool allowChange(const nds::state_t, const nds::state_t, const nds::state_t);

    nds::PVDelegateIn<std::vector<int32_t>> m_dataPV;
};

#endif /* ADQAICHANNEL_H; */
