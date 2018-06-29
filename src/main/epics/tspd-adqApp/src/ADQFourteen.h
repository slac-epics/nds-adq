#ifndef ADQFOURTEEN_H
#define ADQFOURTEEN_H

#include <nds3/nds.h>
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"

class ADQFourteen : public ADQAIChannelGroup
{
public:
    ADQFourteen(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev);
    ~ADQFourteen();

    // Pointer to channel group class
    std::vector<std::shared_ptr<ADQAIChannelGroup> > m_AIChannelGroupPtr;

    void setChanActive(const timespec &pTimestamp, const std::int32_t &pValue);
    void getChanActive(timespec* pTimestamp, std::int32_t* pValue);
    void getChanMask(timespec* pTimestamp, std::string* pValue);
    void setTrigChan(const timespec &pTimestamp, const std::int32_t &pValue);
    void getTrigChan(timespec* pTimestamp, std::int32_t* pValue);
    void setOverVoltProtect(const timespec &pTimestamp, const std::int32_t &pValue);
    void getOverVoltProtect(timespec* pTimestamp, std::int32_t* pValue);

    void commitChangesSpec(bool calledFromDaqThread = false);

private:
    nds::Port m_node;
    ADQInterface * m_adqDevPtr;

    nds::PVDelegateIn<std::int32_t> m_chanActivePV;
    nds::PVDelegateIn<std::string> m_chanMaskPV;
    nds::PVDelegateIn<std::int32_t> m_trigChanPV;
    nds::PVDelegateIn<std::int32_t> m_overVoltProtectPV;
};

#endif /* ADQFOURTEEN_H */