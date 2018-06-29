#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <ADQAPI.h>
#include "ADQDefinition.h"
#include "ADQInfo.h"
#include "ADQFourteen.h"
#include "ADQSeven.h"
#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include <nds3/nds.h>

class ADQDevice
{
public:
    ADQDevice(nds::Factory &factory, const std::string &deviceName, const nds::namedParameters_t &parameters);
    ~ADQDevice();

private:
    // pointer to certain ADQ device
    ADQInterface * m_adqDevPtr;

    // Pointer to ADQ Control Unit
    void* m_adqCtrlUnitPtr;

    //// urojec L2: both of these are vectors of pointers, there is a difference.
    ////            The vectors themselves are std::vectors

    // Pointer to device information class
    std::vector<std::shared_ptr<ADQInfo> > m_adqInfoPtr;

    // Pointers to device specific class
    std::vector<std::shared_ptr<ADQFourteen> > m_adqFrtnPtr;
    std::vector<std::shared_ptr<ADQSeven> > m_adqSvnPtr;

    nds::Node m_node;
};


#endif /* ADQDEVICE_H */
