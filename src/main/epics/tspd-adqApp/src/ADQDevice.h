#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <ADQAPI.h>
#include "ADQInfo.h"
#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include <nds3/nds.h>


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
    ADQInterface * m_adq_dev;

    // Success status
    //// urojec L3: why is this a class variable. If it is just used in the methods
    ////            for reading the return values of adq api function it is safer
    ////            to define it within each unit, instead of having it as member
    ////            -> also cleaner, this is not a member that has to be shared
    unsigned int success;
    // Pointer to ADQ Control Unit
    void* adq_cu;
    // Pointer to ADQ info structure
    struct ADQInfoListEntry* adq_info_list;
    // Number of ADQ devices connected to the system from ListDevices function
    unsigned int adq_list;
    // Number of ADQ devices from NofADQ function
    int n_of_adq;
    // User input
    //// urojec L3: do not hardcode things, use macro or a static const variable
    //// urojec L3: why are these two members??? Form what I can tell it is only used at the begining to
    ////            parse the serial number. If so then just defined in that scope. Unless it is
    ////            used somewhere else and I missed it
    ////        Please check for other such cases, seems it is simmilar with adq_list and firends
    char input_raw[100];
    const char* input;
    // ADQ device number from adq_list array; indexing starts from 0
    // Please note that the device number when using GetADQ/NofADQ/etc will not have anything to do with the index number used in this function.
    int adq_list_nr;
    // ADQ device number used for getting a pointer m_adq_dev
    int adq_nr;
    // Readback ADQ serial number
    char* adq_sn;
    // Serial number of needed ADQ
    char* specified_sn;
    // Var for for loop
    unsigned int adq_found;


    //// urojec L2: both of these are vectors of pointers, there is a difference.
    ////            The vectors themselves are std::vectors
    // Pointer to channel group of device
    std::vector<std::shared_ptr<ADQAIChannelGroup> > m_AIChannelGroup;

    // Pointer to info part of device
    std::vector<std::shared_ptr<ADQInfo> > m_Info;

    nds::Node m_node;
};


#endif /* ADQDEVICE_H */
