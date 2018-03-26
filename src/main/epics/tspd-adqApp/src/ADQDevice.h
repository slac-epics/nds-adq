#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <ADQAPI.h>
#include <nds3/nds.h>

class ADQDevice 
{
public: 
    ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters);
    ~ADQDevice();
    
protected:

private:
    // function ADQControlUnit_ListDevices requires pointers to a list pointer (retList) and a length integer (retLen)
    unsigned int* retLen = 4;
    nds::Node m_node;
};


#endif /* ADQDEVICE_H */