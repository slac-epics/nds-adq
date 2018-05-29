#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

class ADQAIChannel
{
public:
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum, ADQInterface *& adq_dev);

    int32_t m_channelNum;

    void setState(nds::state_t newState);

    //// urojec L3: maybe make these methods a bit more descriptive. It is
    ////            a little bit hard to know what they do just from looking at
    ////            the name
    void read_trigstr(short* rawdata, std::int32_t total_samples);
    void read_multirec(void* rawdata, std::int32_t total_samples);
    void read_contstr(void* rawdata, std::int32_t total_samples);
    void readback(timespec* pTimestamp, std::vector<double>* pValue);
    void commitChanges(bool calledFromAcquisitionThread = false);

private:
    //// urojec L2: why does the channel need this? From code it does not look like it is used anywhere?
    ADQInterface * m_adq_dev;

    nds::Node m_node;
    nds::StateMachine m_stateMachine;
    std::vector<double> m_data;

    bool m_firstReadout;

    int channel;

    //// urojec L3: camelCase
    unsigned char channelsmask;

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
