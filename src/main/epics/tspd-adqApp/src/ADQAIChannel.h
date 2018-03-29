#ifndef ADQAICHANNEL_H
#define ADQAICHANNEL_H

#include <nds3/nds.h>

class ADQAIChannel
{
    ADQAIChannel(const std::string& name, nds::Node& parentNode, int32_t channelNum, struct ifcdaqdrv_usr &deviceUser, int32_t *rawData, double linConvFactor);
};

#endif ADQAICHANNEL_H