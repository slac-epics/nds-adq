#ifndef ADQFOURTEEN_H
#define ADQFOURTEEN_H

#include <nds3/nds.h>
#include "ADQAIChannelGroup.h"

class ADQFourteen : public ADQAIChannelGroup
{
public:
    ADQFourteen(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev);

    nds::StateMachine m_stateMachine;

    unsigned int m_chanCnt;

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

    void commitChanges(bool calledFromDaqThread = false);

private:
    ADQInterface * m_adqDevPtr;
    nds::Port m_node;

    ADQAIChannelGroup* aiChanGrp;

    int32_t m_chanActive;
    bool m_chanActiveChanged;
    int32_t m_chanBits; 
    std::string m_chanMask; 
    bool m_chanMaskChanged;

    int32_t m_trigLvl;
    bool m_trigLvlChanged;
    int32_t m_trigEdge;
    bool m_trigEdgeChanged;
    int32_t m_trigChan;
    int32_t m_trigChanInt;
    bool m_trigChanChanged;

    nds::PVDelegateIn<std::int32_t> m_chanActivePV;
    nds::PVDelegateIn<std::string> m_chanMaskPV;
    nds::PVDelegateIn<std::int32_t> m_trigLvlPV;
    nds::PVDelegateIn<std::int32_t> m_trigEdgePV;
    nds::PVDelegateIn<std::int32_t> m_trigChanPV;

    /*void switchOn();
    void switchOff();
    void start();
    void stop();
    void recover();
    bool allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal); */
};

#endif /* ADQFOURTEEN_H */