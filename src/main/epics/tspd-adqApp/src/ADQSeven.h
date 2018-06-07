#ifndef ADQSEVEN_H
#define ADQSEVEN_H

#include <nds3/nds.h>
#include "ADQAIChannelGroup.h" // : public ADQAIChannelGroup

class ADQSeven : public ADQAIChannelGroup
{
public:
    ADQSeven(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev);

    //nds::StateMachine m_stateMachine;

    // Pointer to channel group class
    std::vector<std::shared_ptr<ADQAIChannelGroup> > m_AIChannelGroupPtr;

    void setChanActive(const timespec &pTimestamp, const std::int32_t &pValue);
    void getChanActive(timespec* pTimestamp, std::int32_t* pValue);
    void setChanMask(const timespec &pTimestamp, const std::string &pValue);
    void getChanMask(timespec* pTimestamp, std::string* pValue);

    void setTrigLvl(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTrigLvl(timespec* pTimestamp, std::int32_t* pValue);
    void setTrigEdge(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTrigEdge(timespec* pTimestamp, std::int32_t* pValue);
    void setTrigChan(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTrigChan(timespec* pTimestamp, std::int32_t* pValue);

    void commitChangesSpec(bool calledFromDaqThread = false);

private:
    ADQInterface * m_adqDevPtr;
    nds::Port m_node;

    nds::PVDelegateIn<std::int32_t> m_chanActivePV;
    nds::PVDelegateIn<std::string> m_chanMaskPV;
    nds::PVDelegateIn<std::int32_t> m_trigLvlPV;
    nds::PVDelegateIn<std::int32_t> m_trigEdgePV;
    nds::PVDelegateIn<std::int32_t> m_trigChanPV;
};

#endif /* ADQSEVEN_H */