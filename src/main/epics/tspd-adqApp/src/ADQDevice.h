#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <ADQAPI.h>
#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include <nds3/nds.h>

class ADQDevice 
{
public: 
    ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters);
    ~ADQDevice();
    
protected:

private:
    // ADQCU
    void* m_adq_cu;
    // ADQ device number
    int m_adq_nr = 1;
    // pointer to certain ADQ device
    ADQInterface* m_adq_ptr = NULL;
    // serial number
    char* m_adq_sn;
 // std::vector<std::shared_ptr<ADQAIChannelGroup> > m_AIChannelGroups;
    nds::Node m_node;
};


#endif /* ADQDEVICE_H */