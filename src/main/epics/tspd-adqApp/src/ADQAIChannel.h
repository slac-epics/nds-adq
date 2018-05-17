#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

class ADQAIChannel
{
public: 
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum, ADQInterface *& adq_dev);

    int32_t m_channelNum;

    void setState(nds::state_t newState);

    void read_trigstr(short* rawdata, std::int32_t total_samples);
    void readback(timespec* pTimestamp, std::vector<double>* pValue);
    void commitChanges(bool calledFromAcquisitionThread = false);

private:
    ADQInterface * m_adq_dev;

    nds::Node m_node;
    nds::StateMachine m_stateMachine;
    std::vector<double> m_data;

    bool m_firstReadout;
     
    int channel;
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