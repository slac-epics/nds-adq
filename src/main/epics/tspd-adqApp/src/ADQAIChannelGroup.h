#ifndef ADQAICHANNELGROUP_H
#define ADQAICHANNELGROUP_H

#include <nds3/nds.h>
#include <time.h>


class ADQAIChannelGroup
{
public:
    ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev);

    nds::Port m_node;
    nds::StateMachine m_stateMachine;

    uint32_t m_numChannels;
    std::vector<std::shared_ptr<ADQAIChannel> > m_AIChannels;

    void getProductName(timespec* pTimestamp, std::string* pValue);
    void getSerialNumber(timespec* pTimestamp, std::string* pValue);
    void getProductID(timespec* pTimestamp, std::int32_t* pValue);
    void getADQType(timespec* pTimestamp, std::int32_t* pValue);
    void getCardOption(timespec* pTimestamp, std::string* pValue);
    
private:
    // pointer to certain ADQ device
    ADQInterface * m_adq_dev;

    // PVs connected to EPICS records
    nds::PVDelegateIn<std::string> m_productNamePV;
    nds::PVDelegateIn<std::string> m_serialNumberPV;
    nds::PVDelegateIn<std::int32_t> m_productIDPV;
    nds::PVDelegateIn<std::int32_t> m_adqTypePV;
    nds::PVDelegateIn<std::string> m_cardOptionPV;

};

#endif /* ADQAICHANNELGROUP_H */
