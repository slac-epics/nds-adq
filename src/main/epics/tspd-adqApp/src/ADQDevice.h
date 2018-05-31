#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <ADQAPI.h>
#include "ADQInfo.h"
#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include <nds3/nds.h>

#define CHANNEL_NUMBER_MAX 8
#define CELSIUS_CONVERT 1/256

class ADQDevice
{
public:
    ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters);
    ~ADQDevice();


    //// urojec L3: a lot of mixing between camel case and underscore notatiojn, please use one everywhere
    //// urojec L3: the m_ prefix for member variables is a good choice, but it MUST be used for all member variables
    //// urojec L3: it ouwld be nice if htere was sone conistency as to where the adq appears in variables
    //// urojec L3: some of them could be more descriptive
    //// examples:
    ////    m_adq_dev       -> m_adqDev
    ////    adq_cu          -> m_adqCu or m_adqCtrlU
    ////    adq_info_list   -> m_adqInfoList
    ////    n_of_adq        -> m_adqTotalDevCnt (total device count, or just device count)
    ////    adq_nr          -> m_adqDevNr
    //// urojec L3: for variables that are request and readback this should be immediately clear,
    ////    adq_sn          -> m_adqSnRdbk
    ////    specified_sn    -> m_adqSnReq

private:
    //// urojec L3: group the members togther when you declare them based on what
    ////            they represent

    // pointer to certain ADQ device
    //// urojec L3: maybe gie Ptr at the end, since you mostly operate with regular
    ////            variables, this will avoid confusion (m_adqDevPtr)
    ADQInterface * m_adqDevPtr;

    // Pointer to ADQ Control Unit
    void* m_adqCtrlUnitPtr;

    //// urojec L3: do not hardcode things, use macro or a static const variable
    //// urojec L3: why are these two members??? Form what I can tell it is only used at the begining to
    ////            parse the serial number. If so then just defined in that scope. Unless it is
    ////            used somewhere else and I missed it
    ////        Please check for other such cases, seems it is simmilar with adq_list and firends


    //// urojec L2: both of these are vectors of pointers, there is a difference.
    ////            The vectors themselves are std::vectors
    // Pointer to channel group of device
    std::vector<std::shared_ptr<ADQAIChannelGroup> > m_AIChannelGroupPtr;

    // Pointer to info part of device
    std::vector<std::shared_ptr<ADQInfo> > m_infoPtr;

    nds::Node m_node;
};


#endif /* ADQDEVICE_H */
