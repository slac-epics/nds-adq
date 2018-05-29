#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <cstring>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

#define sleep(interval) usleep(1000*interval) // usleep - microsecond interval
#define PRINT_RECORD_INFO

//// urojec L3: camelCase
//// urojec L2:

static struct timespec tsref;
void timer_start(void)
{
    if (clock_gettime(CLOCK_REALTIME, &tsref) < 0) {
        printf("\nFailed to start timer.");
        return;
    }
}
unsigned int timer_time_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
        printf("\nFailed to get system time.");
        return -1;
    }
    return (unsigned int)((int)(ts.tv_sec - tsref.tv_sec) * 1000 +
        (int)(ts.tv_nsec - tsref.tv_nsec) / 1000000);
}


//// urojec L2: hardcoading values is depreciated
ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adq_dev(adq_dev),
    m_trigmode(0),
    m_adjustBias(0),
    m_daqmode(2),
    m_pattmode(0),
    m_channels(3),
    m_channelmask("0x01"),
    m_nofrecords(10),
    m_nofsamples(12),
    m_dbs_bypass(0),
    m_dbs_dctarget(0),
    m_dbs_lowsat(0),
    m_dbs_upsat(0),
    m_trigmodeChanged(true),
    m_biasChanged(true),
    m_dbsbypassChanged(true),
    m_dbsdcChanged(true),
    m_dbslowsatChanged(true),
    m_dbsupsatChanged(true),
    m_pattmodeChanged(true),
    m_nofrecordsChanged(true),
    m_nofsamplesChanged(true),
    m_daqmodeChanged(true),
    m_channelsChanged(true),
    m_channelmaskChanged(true),
    m_daqmodePV(nds::PVDelegateIn<std::int32_t>("DAQMode-RB", std::bind(&ADQAIChannelGroup::getDAQMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigmodePV(nds::PVDelegateIn<std::int32_t>("TriggerMode-RB", std::bind(&ADQAIChannelGroup::getTriggerMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_pattmodePV(nds::PVDelegateIn<std::int32_t>("PatternMode-RB", std::bind(&ADQAIChannelGroup::getPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_adjustBiasPV(nds::PVDelegateIn<std::int32_t>("AdjustBias-RB", std::bind(&ADQAIChannelGroup::getAdjustBias,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_bypassPV(nds::PVDelegateIn<std::int32_t>("DBS-Bypass-RB", std::bind(&ADQAIChannelGroup::getDBSbypass,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_dctargetPV(nds::PVDelegateIn<std::int32_t>("DBS-DC-RB", std::bind(&ADQAIChannelGroup::getDBSdc,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_lowsatPV(nds::PVDelegateIn<std::int32_t>("DBS-LowSat-RB", std::bind(&ADQAIChannelGroup::getDBSlowsat,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_dbs_upsatPV(nds::PVDelegateIn<std::int32_t>("DBS-UpSat-RB", std::bind(&ADQAIChannelGroup::getDBSupsat,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_channelsPV(nds::PVDelegateIn<std::int32_t>("Channels-RB", std::bind(&ADQAIChannelGroup::getChannels,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_channelmaskPV(nds::PVDelegateIn<std::string>("ChannelMask-RB", std::bind(&ADQAIChannelGroup::getChannelMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_nofrecordsPV(nds::PVDelegateIn<std::int32_t>("NofRecords-RB", std::bind(&ADQAIChannelGroup::getNofRecords,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_collect_recordsPV(nds::PVDelegateIn<std::int32_t>("CollectRecords-RB", std::bind(&ADQAIChannelGroup::getCollectRecords,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_maxsamplesPV(nds::PVDelegateIn<std::int32_t>("MaxSamples-RB", std::bind(&ADQAIChannelGroup::getMaxSamples,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_nofsamplesPV(nds::PVDelegateIn<std::int32_t>("NofSamples-RB", std::bind(&ADQAIChannelGroup::getNofSamples,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_totalsamplesPV(nds::PVDelegateIn<std::int32_t>("TotalSamples-RB", std::bind(&ADQAIChannelGroup::getTotalSamples,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_triglvlPV(nds::PVDelegateIn<std::int32_t>("TriggerLevel-RB", std::bind(&ADQAIChannelGroup::getTriggerLvl,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigedgePV(nds::PVDelegateIn<std::int32_t>("TriggerEdge-RB", std::bind(&ADQAIChannelGroup::getTriggerEdge,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_trigchanPV(nds::PVDelegateIn<std::int32_t>("TriggerChannel-RB", std::bind(&ADQAIChannelGroup::getTriggerChan,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);

    nofchan = m_adq_dev->GetNofChannels();

    for (size_t numChannel(0); numChannel != nofchan; ++numChannel)
    {
        std::ostringstream channelName;
        channelName << "CH" << numChannel;
        m_AIChannels.push_back(std::make_shared<ADQAIChannel>(channelName.str(), m_node, numChannel, m_adq_dev));
    }

    // PVs for data acquisition modes
    //// urojec L2: why all this push back, why not just fix the list. It is not something that dynamically changes
    nds::enumerationStrings_t daqModeList;
    daqModeList.push_back("Multi-Record");
    daqModeList.push_back("Continuous streaming");
    daqModeList.push_back("Triggered streaming");
    nds::PVDelegateOut<std::int32_t> node(nds::PVDelegateOut<std::int32_t>("DAQMode", std::bind(&ADQAIChannelGroup::setDAQMode,
                                                                                                  this,
                                                                                                  std::placeholders::_1,
                                                                                                  std::placeholders::_2),
                                                                                      std::bind(&ADQAIChannelGroup::getDAQMode,
                                                                                                  this,
                                                                                                  std::placeholders::_1,
                                                                                                  std::placeholders::_2)));
    node.setEnumeration(daqModeList);
    m_node.addChild(node);

    m_daqmodePV.setScanType(nds::scanType_t::interrupt);
    m_daqmodePV.setEnumeration(daqModeList);
    m_node.addChild(m_daqmodePV);

    // PVs for Trigger Mode
    nds::enumerationStrings_t triggerModeList;
    triggerModeList.push_back("SW trigger");
    triggerModeList.push_back("External trigger");
    triggerModeList.push_back("Level trigger");
    triggerModeList.push_back("Internal trigger");

    //// urojec L1: is this reusage of nodes safe? Are you sure that a copy is made in the addChild function,
    //// or are there references?
    node = nds::PVDelegateOut<std::int32_t>("TriggerMode", std::bind(&ADQAIChannelGroup::setTriggerMode,
                                                                       this,
                                                                       std::placeholders::_1,
                                                                       std::placeholders::_2),
                                                           std::bind(&ADQAIChannelGroup::getTriggerMode,
                                                                       this,
                                                                       std::placeholders::_1,
                                                                       std::placeholders::_2));
    node.setEnumeration(triggerModeList);
    m_node.addChild(node);

    m_trigmodePV.setScanType(nds::scanType_t::interrupt);
    m_trigmodePV.setEnumeration(triggerModeList);
    m_node.addChild(m_trigmodePV);

    // PVs for Adjustable Bias
    m_node.addChild(nds::PVDelegateOut<std::int32_t>("AdjustBias",
                                                   std::bind(&ADQAIChannelGroup::setAdjustBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getAdjustBias,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_adjustBiasPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_adjustBiasPV);

    // PV for DBS setup
    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-Bypass",
                                                   std::bind(&ADQAIChannelGroup::setDBSbypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSbypass,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_bypassPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_bypassPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-DC",
                                                   std::bind(&ADQAIChannelGroup::setDBSdc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSdc,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_dctargetPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_dctargetPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-LowSat",
                                                   std::bind(&ADQAIChannelGroup::setDBSlowsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSlowsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_lowsatPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_lowsatPV);

    m_node.addChild(nds::PVDelegateOut<std::int32_t>("DBS-UpSat",
                                                   std::bind(&ADQAIChannelGroup::setDBSupsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2),
                                                   std::bind(&ADQAIChannelGroup::getDBSupsat,
                                                             this,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2)));

    m_dbs_upsatPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_dbs_upsatPV);

    // PV for Pattern Mode
    nds::enumerationStrings_t patternModeList;
    patternModeList.push_back("Normal");
    patternModeList.push_back("Test (x)");
    patternModeList.push_back("Count upwards");
    patternModeList.push_back("Count downwards");
    patternModeList.push_back("Alternating ups and downs");
    node = nds::PVDelegateOut<std::int32_t>("PatternMode", std::bind(&ADQAIChannelGroup::setPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                           std::bind(&ADQAIChannelGroup::getPatternMode,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));
    node.setEnumeration(patternModeList);
    m_node.addChild(node);

    m_pattmodePV.setScanType(nds::scanType_t::interrupt);
    m_pattmodePV.setEnumeration(patternModeList);
    m_node.addChild(m_pattmodePV);

    //// urojec L1: as far as I can tell, the ADQ14 is a bit special with respect to what combinations are allowed.
    ////            this is typically something that has to be moved to ADQ14 specifric class
    // PV for channel enabling
    nds::enumerationStrings_t channelMaskList;
    channelMaskList.push_back("A+B+C+D");
    channelMaskList.push_back("A+B");
    channelMaskList.push_back("C+D");
    channelMaskList.push_back("A");
    channelMaskList.push_back("B");
    channelMaskList.push_back("C");
    channelMaskList.push_back("D");
    node = nds::PVDelegateOut<std::int32_t>("Channels", std::bind(&ADQAIChannelGroup::setChannels,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                        std::bind(&ADQAIChannelGroup::getChannels,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));
    node.setEnumeration(channelMaskList);
    m_node.addChild(node);

    m_channelsPV.setScanType(nds::scanType_t::interrupt);
    m_channelsPV.setEnumeration(channelMaskList);
    m_node.addChild(m_channelsPV);

    nds::PVDelegateOut<std::string> node_str("ChannelMask", std::bind(&ADQAIChannelGroup::setChannelMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                            std::bind(&ADQAIChannelGroup::getChannelMask,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2));

    m_node.addChild(node_str);

    m_channelmaskPV.setScanType(nds::scanType_t::interrupt);
    m_channelmaskPV.setMaxElements(4);
    m_node.addChild(m_channelmaskPV);

    // PVs for records
    node = nds::PVDelegateOut<std::int32_t>("NofRecords", std::bind(&ADQAIChannelGroup::setNofRecords,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getNofRecords,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_nofrecordsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_nofrecordsPV);

    m_totalsamplesPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_totalsamplesPV);

    node = nds::PVDelegateOut<std::int32_t>("CollectRecords", std::bind(&ADQAIChannelGroup::setCollectRecords,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                              std::bind(&ADQAIChannelGroup::getCollectRecords,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_collect_recordsPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_collect_recordsPV);

    //PVs for samples
    m_maxsamplesPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_maxsamplesPV);

    node = nds::PVDelegateOut<std::int32_t>("NofSamples", std::bind(&ADQAIChannelGroup::setNofSamples,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2),
                                                          std::bind(&ADQAIChannelGroup::getNofSamples,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2));
    m_node.addChild(node);
    m_nofsamplesPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_nofsamplesPV);

    //PVs for trigger level
    node = nds::PVDelegateOut<std::int32_t>("TriggerLevel", std::bind(&ADQAIChannelGroup::setTriggerLvl,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2),
                                                            std::bind(&ADQAIChannelGroup::getTriggerLvl,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                       std::placeholders::_2));
    m_node.addChild(node);
    m_triglvlPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_triglvlPV);

    // PVs for trigger edge
    nds::enumerationStrings_t triggerEdgeList;
    triggerEdgeList.push_back("Falling edge");
    triggerEdgeList.push_back("Rising edge");
    node = nds::PVDelegateOut<std::int32_t>("TriggerEdge", std::bind(&ADQAIChannelGroup::setTriggerEdge,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2),
                                                           std::bind(&ADQAIChannelGroup::getTriggerEdge,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));
    node.setEnumeration(triggerEdgeList);
    m_node.addChild(node);

    m_trigedgePV.setScanType(nds::scanType_t::interrupt);
    m_trigedgePV.setEnumeration(triggerEdgeList);
    m_node.addChild(m_trigedgePV);

    // PVs for trigger channel
    /* On ADQ7 only a single bit in the mask can be set, only one channel is allowed to at once generate the trigger.
     * On ADQ14 only masks 1, 2, 4, 8 (single bit) and masks 3 (A + B), 12 (C + D), 15 (A + B + C + D) are allowed.
     */
    nds::enumerationStrings_t triggerChannelList;
    triggerChannelList.push_back("None");
    triggerChannelList.push_back("A");
    triggerChannelList.push_back("B");
    triggerChannelList.push_back("C");
    triggerChannelList.push_back("D");
    triggerChannelList.push_back("A+B");
    triggerChannelList.push_back("C+D");
    triggerChannelList.push_back("A+B+C+D");
    node = nds::PVDelegateOut<std::int32_t>("TriggerChannel", std::bind(&ADQAIChannelGroup::setTriggerChan,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2),
                                                              std::bind(&ADQAIChannelGroup::getTriggerChan,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));
    node.setEnumeration(triggerChannelList);
    m_node.addChild(node);

    m_trigchanPV.setScanType(nds::scanType_t::interrupt);
    m_trigchanPV.setEnumeration(triggerChannelList);
    m_node.addChild(m_trigchanPV);

    // PV for state machine
    m_stateMachine = m_node.addChild(nds::StateMachine(true, std::bind(&ADQAIChannelGroup::onSwitchOn, this),
                                                             std::bind(&ADQAIChannelGroup::onSwitchOff, this),
                                                             std::bind(&ADQAIChannelGroup::onStart, this),
                                                             std::bind(&ADQAIChannelGroup::onStop, this),
                                                             std::bind(&ADQAIChannelGroup::recover, this),
                                                             std::bind(&ADQAIChannelGroup::allowChange,
                                                                     this,
                                                                     std::placeholders::_1,
                                                                     std::placeholders::_2,
                                                                     std::placeholders::_3)));

    commitChanges();
}

void ADQAIChannelGroup::setDAQMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_daqmode = pValue;
    m_daqmodeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDAQMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_daqmode;
}

void ADQAIChannelGroup::setTriggerMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigmode = pValue;
    m_trigmodeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTriggerMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigmode;
}

void ADQAIChannelGroup::setPatternMode(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_pattmode = pValue;
    m_pattmodeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getPatternMode(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_pattmode;
}

void ADQAIChannelGroup::setAdjustBias(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_adjustBias = pValue;
    m_biasChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getAdjustBias(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adjustBias;
}

void ADQAIChannelGroup::setDBSbypass(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_bypass = pValue;
    m_dbsbypassChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSbypass(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_bypass;
}

void ADQAIChannelGroup::setDBSdc(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_dctarget = pValue;
    m_dbsdcChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSdc(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_dctarget;
}

void ADQAIChannelGroup::setDBSlowsat(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_lowsat = pValue;
    m_dbslowsatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSlowsat(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_lowsat;
}

void ADQAIChannelGroup::setDBSupsat(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_dbs_upsat = pValue;
    m_dbsupsatChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getDBSupsat(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_dbs_upsat;
}

void ADQAIChannelGroup::setChannels(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_channels = pValue;
    m_channelsChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getChannels(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_channels;
}

void ADQAIChannelGroup::setChannelMask(const timespec &pTimestamp, const std::string &pValue)
{
    m_channelmask = pValue;
    commitChanges();
}

void ADQAIChannelGroup::getChannelMask(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_channelmask;
}

void ADQAIChannelGroup::setNofRecords(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_nofrecords = pValue;
    m_nofrecordsChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getNofRecords(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_nofrecords;
}

void ADQAIChannelGroup::setCollectRecords(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_collect_records = pValue;
    m_collect_recordsChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getCollectRecords(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_collect_records;
}

void ADQAIChannelGroup::getMaxSamples(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_maxsamples;
}

void ADQAIChannelGroup::setNofSamples(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_nofsamples = pValue;
    m_nofsamplesChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getNofSamples(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_nofsamples;
}

void ADQAIChannelGroup::getTotalSamples(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_totalsamples;
}

void ADQAIChannelGroup::setTriggerLvl(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_triglvl = pValue;
    m_triglvlChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTriggerLvl(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_triglvl;
}

void ADQAIChannelGroup::setTriggerEdge(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigedge = pValue;
    m_trigedgeChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTriggerEdge(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigedge;
}

void ADQAIChannelGroup::setTriggerChan(const timespec &pTimestamp, const std::int32_t &pValue)
{
    m_trigchan = pValue;
    m_trigchanChanged = true;
    commitChanges();
}

void ADQAIChannelGroup::getTriggerChan(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_trigchan;
}

void ADQAIChannelGroup::commitChanges(bool calledFromAcquisitionThread)
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);
    int i;

    /* Allow changes to parameters when device is ON/STOPPING/INITIALISING states.
     * Do not apply changes when device is on acquisition state.
     */
    if (!calledFromAcquisitionThread && (
        m_stateMachine.getLocalState() != nds::state_t::on &&
        m_stateMachine.getLocalState() != nds::state_t::stopping  &&
        m_stateMachine.getLocalState() != nds::state_t::initializing)) {
        return;
    }

    if (m_daqmodeChanged)
    {
        m_daqmodeChanged = false;
        m_daqmodePV.push(now, m_daqmode);
    }

    if (m_trigmodeChanged)
    {
        m_trigmodeChanged = false;

        success = m_adq_dev->SetTriggerMode(m_trigmode + 1);

        if (!success)
        {
            ndsWarningStream(m_node) << "FAILURE: Trigger Mode was not set." << std::endl;
        }
        else
        {
            ndsInfoStream(m_node) << "SUCCESS: Trigger Mode is set to " << m_trigmode << std::endl;
            m_trigmodePV.push(now, m_trigmode);
        }
    }

    if (m_pattmodeChanged)
    {
        m_pattmodeChanged = false;
        success = m_adq_dev->SetTestPatternMode(m_pattmode);
        if (!success)
        {
            ndsWarningStream(m_node) << "FAILURE: Failed setting up pattern mode." << std::endl;
        }
        else
        {
            ndsInfoStream(m_node) << "SUCCESS: Pattern Mode is set to " << m_pattmode << std::endl;
            m_pattmodePV.push(now, m_pattmode);
        }
    }

    if (m_biasChanged)
    {
        m_biasChanged = false;
        const char chan[5] = "ABCD";
        int* adc_code;
        success = m_adq_dev->HasAdjustableBias();
        if (success)
        {
            for (ch = 0; ch < nofchan; ++ch)
            {
                success = m_adq_dev->SetAdjustableBias(ch + 1, m_adjustBias);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: " << "Failed setting adjustable bias for channel " << chan[ch] << std::endl;
                }
                else
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == nofchan)
            {
                ndsInfoStream(m_node) << "SUCCESS:" << "Adjustable Bias for all channels is set." << std::endl;
                m_adjustBiasPV.push(now, m_adjustBias);
            }
        }
    }

    if (m_dbsbypassChanged || m_dbsdcChanged || m_dbslowsatChanged || m_dbsupsatChanged)
    {
        m_dbsbypassChanged = false;
        m_dbsdcChanged = false;
        m_dbslowsatChanged = false;
        m_dbsupsatChanged = false;

        success = m_adq_dev->GetNofDBSInstances(&dbs_n_of_inst);
        if (success)
        {
            ndsInfoStream(m_node) << "Number of DBS instances: " << dbs_n_of_inst << std::endl;
            i = 0;
            for (dbs_inst = 0; dbs_inst < dbs_n_of_inst; ++dbs_inst)
            {
                success = m_adq_dev->SetupDBS(dbs_inst, m_dbs_bypass, m_dbs_dctarget, m_dbs_lowsat, m_dbs_upsat);
                if (!success)
                {
                    ndsWarningStream(m_node) << "FAILURE: Failed setting up DBS instance " << dbs_inst << std::endl;
                }
                else
                {
                    ++i;
                }
            }
            sleep(1000);

            if (i == dbs_n_of_inst)
            {
                ndsInfoStream(m_node) << "SUCCESS: All " << dbs_n_of_inst << " DBS instances are set up." << std::endl;
                m_dbs_bypassPV.push(now, m_dbs_bypass);
                m_dbs_dctargetPV.push(now, m_dbs_dctarget);
                m_dbs_lowsatPV.push(now, m_dbs_lowsat);
                m_dbs_upsatPV.push(now, m_dbs_upsat);
            }
        }
    }

    if (m_channelsChanged)
    {
        m_channelsChanged = false;
        m_channelbits = 0000;

        if (!nofchan)
        {
            ndsWarningStream(m_node) << "FAILURE: No channels are found." << std::endl;
        }
        else
        {
            switch (m_channels)
            {
            case 0: // ch A+B+C+D
                m_channelbits |= 1111;
                m_channelmask = 0xF;
                break;
            case 1: // ch A+B
                m_channelbits |= 1100;
                m_channelmask = 0x3;
                break;
            case 2: // ch C+D
                m_channelbits |= 0011;
                m_channelmask = 0xC;
                break;
            case 3: // ch A
                m_channelbits |= 1000;
                m_channelmask = 0x1;
                break;
            case 4: // ch B
                m_channelbits |= 0100;
                m_channelmask = 0x2;
                break;
            case 5: // ch C
                m_channelbits |= 0010;
                m_channelmask = 0x4;
                break;
            case 6: // ch D
                m_channelbits |= 0001;
                m_channelmask = 0x8;
                break;
            }
        }

        channelmask_char = *m_channelmask.c_str();

        if (m_daqmode == 0) // Multi-Record
        {
            success = m_adq_dev->MultiRecordSetChannelMask(m_channelbits);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at MultiRecordSetChannelMask." << std::endl;
            }
        }
        if (m_daqmode == 1) // Continuous Streaming
        {
            success = m_adq_dev->ContinuousStreamingSetup(m_channelbits);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at ContinuousStreamingSetup." << std::endl;
            }

        }

        m_channelmaskPV.push(now, m_channelmask);
        m_channelsPV.push(now, m_channelbits);
    }

    if (m_nofrecordsChanged || m_nofsamplesChanged)
    {
        m_nofrecordsChanged = false;
        m_nofsamplesChanged = false;

        if (m_nofrecords != -1)
        {
            m_nofrecordsPV.push(now, m_nofrecords);
            success = m_adq_dev->GetMaxNofSamplesFromNofRecords(m_nofrecords, &max_nofsamples);
            if (!success)
            {
                ndsWarningStream(m_node) << "FAILURE: Couldn't get the MAX number of samples (set number of records)." << std::endl;
            }
            else
            {
                m_maxsamples = max_nofsamples;
                m_maxsamplesPV.push(now, m_maxsamples);
                ndsInfoStream(m_node) << "SUCCESS: Maximum number of samples is " << m_maxsamples << std::endl;

                if (m_nofsamples > m_maxsamples)
                {
                    ndsErrorStream(m_node) << "ERROR: Chosen number of samples is higher than the MAX: " << m_nofsamples << ">" << m_maxsamples << std::endl;
                }
                else
                {
                    m_nofsamplesPV.push(now, m_nofsamples);

                    m_totalsamples = m_nofsamples * m_nofrecords;
                    m_totalsamplesPV.push(now, m_totalsamples);
                }
            }
        }
        else
        {
            m_totalsamples = m_nofsamples * m_nofrecords;

            m_nofrecordsPV.push(now, m_nofrecords);
            m_nofsamplesPV.push(now, m_nofsamples);
            m_totalsamplesPV.push(now, m_totalsamples);
        }
    }

    if (m_collect_recordsChanged)
    {
        m_collect_recordsChanged = false;

        if (m_collect_records > m_nofrecords)
            m_collect_records = m_nofrecords;

        m_collect_recordsPV.push(now, m_collect_records);
    }

    if (m_trigedgeChanged)
    {
        m_trigedgeChanged = false;
        m_trigedgePV.push(now, m_trigedge);

        if (m_trigmode == 2)
        {
            success = m_adq_dev->SetLvlTrigEdge(m_trigedge);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigEdge." << std::endl;
            }
        }
    }

    if (m_trigchanChanged)
    {
        m_trigchanChanged = false;

        switch (m_trigchan)
        {
        case 0: // None
            trigchan_int = 0;
            break;
        case 1: // ch A
            trigchan_int = 1;
            break;
        case 2: // ch B
            trigchan_int = 2;
            break;
        case 3: // ch C
            trigchan_int = 4;
            break;
        case 4: // ch D
            trigchan_int = 8;
            break;
        case 5: // ch A+B
            trigchan_int = 3;
            break;
        case 6: // ch C+D
            trigchan_int = 12;
            break;
        case 7: // ch A+B+C+D
            trigchan_int = 15;
            break;
        }

        m_trigchanPV.push(now, m_trigchan);

        if (m_trigmode == 2)
        {
            success = m_adq_dev->SetLvlTrigChannel(trigchan_int);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigChannel." << std::endl;
            }
        }
    }

    if (m_triglvlChanged)
    {
        m_triglvlChanged = false;
        m_triglvlPV.push(now, m_triglvl);
        if (m_trigmode == 2)
        {
            success = m_adq_dev->SetLvlTrigLevel(m_triglvl);
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at SetLvlTrigLevel." << std::endl;
            }
        }
    }
}


void ADQAIChannelGroup::onSwitchOn()
{
    // Enable all channels
    for (auto const& channel : m_AIChannels)
    {
        channel->setState(nds::state_t::on);
    }
    commitChanges(true);
}

void ADQAIChannelGroup::onSwitchOff()
{
    // Disable all channels
    for (auto const& channel : m_AIChannels)
    {
        channel->setState(nds::state_t::off);
    }
    commitChanges();
}

void ADQAIChannelGroup::onStart()
{
    // Start all Channels
    for (auto const& channel : m_AIChannels)
    {
        channel->setState(nds::state_t::running);
    }

    // Start data acquisition
    m_stop = false;

    if (m_daqmode == 0)
        m_acquisitionThread = m_node.runInThread("Acquisition-MultiRec", std::bind(&ADQAIChannelGroup::acquisition_multirec, this));
    if (m_daqmode == 1)
        m_acquisitionThread = m_node.runInThread("Acquisition-ContStream", std::bind(&ADQAIChannelGroup::acquisition_contstream, this));
    if (m_daqmode == 2)
        m_acquisitionThread = m_node.runInThread("Acquisition-TrigStream", std::bind(&ADQAIChannelGroup::acquisition_trigstream, this));
}

void ADQAIChannelGroup::onStop()
{
    m_stop = true;
    m_adq_dev->StopStreaming();

    m_acquisitionThread.join();

    // Stop channels
    for (auto const& channel : m_AIChannels)
    {
        channel->setState(nds::state_t::on);
    }

    commitChanges();
}

void ADQAIChannelGroup::recover()
{
    throw nds::StateMachineRollBack("Cannot recover");
}

bool ADQAIChannelGroup::allowChange(const nds::state_t currentLocal, const nds::state_t currentGlobal, const nds::state_t nextLocal)
{
    return true;
}

void ADQAIChannelGroup::acquisition_trigstream()
{
    int i;

    unsigned int m_tr_nofbuf = 8;
    unsigned int m_tr_bufsize = 512 * 1024;

    StreamingHeader_t* tr_headers[4] = { NULL, NULL, NULL, NULL };
    short* tr_buffers[4] = { NULL, NULL, NULL, NULL };
    short* tr_extra[4] = { NULL, NULL, NULL, NULL };
    short* record_data;
    short* all_records_data;
    unsigned int pretrig_samples = 0;
    unsigned int holdoff_samples = 0;
    unsigned int buffers_filled = 0;
    unsigned int records_completed[4] = { 0, 0, 0, 0 };
    unsigned int samples_added[4] = { 0, 0, 0, 0 };
    unsigned int headers_added[4] = { 0, 0, 0, 0 };
    unsigned int samples_extradata[4] = { 0, 0, 0, 0 };
    unsigned int header_status[4] = { 0, 0, 0, 0 };
    unsigned int headers_done = 0;
    unsigned int timeout_ms = 1000;
    unsigned int received_all_records = 0;
    unsigned int samples_remaining;
    unsigned int nof_received_records_sum = 0;
    unsigned int nof_records_sum = 0;

    int total_samples = m_nofrecords * m_nofsamples;

    for (ch = 0; ch < nofchan; ++ch)
    {
        if (!((1 << ch) & channelmask_char))
            continue;
        tr_buffers[ch] = (short int*)malloc((size_t)m_tr_bufsize);
        if (!tr_buffers[ch])
        {
            ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target buffers." << std::endl;
            goto finish;
        }
        else
        {
            tr_headers[ch] = (StreamingHeader_t*)malloc((size_t)m_tr_bufsize);
            if (!tr_headers[ch])
            {
                ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target buffers." << std::endl;
                goto finish;
            }
            else
            {
                tr_extra[ch] = (short int*)malloc((size_t)(sizeof(short)*m_nofsamples));
                if (!tr_extra[ch])
                {
                    ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for target extradata." << std::endl;
                    goto finish;
                }
            }
        }
    }

    // Allocate memory for record data (used for ProcessRecord function template)
    record_data = (short int*)malloc((size_t)(sizeof(short)*m_nofsamples));

    if (!record_data)
    {
        ndsErrorStream(m_node) << "ERROR: Failed to allocate memory for ProcessRecord." << std::endl;
        goto finish;
    }

    // Compute the sum of the number of records specified by the user
    for (ch = 0; ch < nofchan; ++ch)
    {
        nofrecords_sum += m_nofrecords;
    }

    success = m_adq_dev->TriggeredStreamingSetup(m_nofrecords, m_nofsamples, pretrig_samples, holdoff_samples, channelmask_char);
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at TriggeredStreamingSetup." << std::endl;
        goto finish;
    }
    else
    {
        success = m_adq_dev->SetTransferBuffers(m_tr_nofbuf, m_tr_bufsize);
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at SetTransferBuffers." << std::endl;
            goto finish;
        }
        else
        {
            success = m_adq_dev->StopStreaming();
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at StopStreaming." << std::endl;
                goto finish;
            }
            else
            {
                success = m_adq_dev->StartStreaming();
                if (!success)
                {
                    ndsErrorStream(m_node) << "ERROR: Failed at StartStreaming." << std::endl;
                    goto finish;
                }
                else
                {
                    switch (m_trigmode)
                    {
                    case 0: // SW trigger
                        ndsInfoStream(m_node) << "Issuing Software trigger... " << std::endl;
                        success = m_adq_dev->DisarmTrigger();
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
                            goto finish;
                        }
                        else
                        {
                            success = m_adq_dev->ArmTrigger();
                            if (!success)
                            {
                                ndsErrorStream(m_node) << "ERROR: Failed at ArmTrigger." << std::endl;
                                goto finish;
                            }
                            else
                            {
                                for (i = 0; i < m_nofrecords; ++i)
                                {
                                    success = m_adq_dev->SWTrig();
                                    if (!success)
                                    {
                                        ndsErrorStream(m_node) << "ERROR: Failed at SWTrig." << std::endl;
                                        goto finish;
                                    }
                                }

                                do
                                {
                                    buffers_filled = 0;

                                    success = m_adq_dev->GetStreamOverflow();
                                    if (success)
                                    {
                                        ndsErrorStream(m_node) << "ERROR: Streaming overflow detected." << std::endl;
                                        goto finish;
                                    }

                                    success = m_adq_dev->GetTransferBufferStatus(&buffers_filled);
                                    if (!success)
                                    {
                                        ndsErrorStream(m_node) << "ERROR: Failed at GetTransferBufferStatus." << std::endl;
                                        goto finish;
                                    }

                                    // Poll for the transfer buffer status as long as the timeout has not been
                                    // reached and no buffers have been filled.
                                    while (!buffers_filled)
                                    {
                                        // Mark the loop start
                                        timer_start();
                                        while (!buffers_filled && (timer_time_ms() < timeout_ms))
                                        {
                                            success = m_adq_dev->GetTransferBufferStatus(&buffers_filled);
                                            if (!success)
                                            {
                                                ndsErrorStream(m_node) << "ERROR: Failed at GetTransferBufferStatus." << std::endl;
                                                goto finish;
                                            }
                                            // Sleep to avoid loading the processor too much
                                            sleep(10);
                                        }

                                        // Timeout reached, flush the transfer buffer to receive data
                                        if (!buffers_filled)
                                        {
                                            ndsInfoStream(m_node) << "Timeout, flushing DMA..." << std::endl;
                                            success = m_adq_dev->FlushDMA();
                                            if (!success)
                                            {
                                                ndsErrorStream(m_node) << "ERROR: Failed at FlushDMA." << std::endl;
                                                goto finish;
                                            }
                                        }
                                    }

                                    ndsInfoStream(m_node) << "Receiving data..." << std::endl;
                                    success = m_adq_dev->GetDataStreaming((void**)tr_buffers, (void**)tr_headers, channelmask_char, samples_added, headers_added, header_status);
                                    if (!success)
                                    {
                                        ndsErrorStream(m_node) << "ERROR: Failed at GetDataStreaming." << std::endl;
                                        goto finish;
                                    }
                                    else
                                    {
                                        // Parse data
                                        for (ch = 0; ch < 4; ++ch)
                                        {
                                            if (!((1 << ch) & channelmask_char))
                                                continue;

                                            if (headers_added[ch] > 0)
                                            {
                                                if (header_status[ch])
                                                    headers_done = headers_added[ch];
                                                else
                                                    // One incomplete record in the buffer (header is copied to the front
                                                    // of the buffer later)
                                                    headers_done = headers_added[ch] - 1;

                                                // If there is at least one complete header
                                                records_completed[ch] += headers_done;
                                            }

                                            // Parse  added samples
                                            if (samples_added[ch] > 0)
                                            {
                                                samples_remaining = samples_added[ch];

                                                // Handle incomplete record at the start of the buffer
                                                if (samples_extradata[ch] > 0)
                                                {
                                                    if (headers_done == 0)
                                                    {
                                                        // There is not enough data in the transfer buffer to complete
                                                        // the record. Add all the samples to the extradata buffer.
                                                        std::memcpy(&tr_extra[ch][samples_extradata[ch]], tr_buffers[ch], sizeof(short)*samples_added[ch]);
                                                        samples_remaining -= samples_added[ch];
                                                        samples_extradata[ch] += samples_added[ch];
                                                    }
                                                    else
                                                    {
                                                        // Move data to record_data
                                                        std::memcpy((void*)record_data, tr_extra[ch], sizeof(short)*samples_extradata[ch]);
                                                        std::memcpy((void*)(record_data + samples_extradata[ch]), tr_buffers[ch], sizeof(short)*(tr_headers[ch][0].RecordLength - samples_extradata[ch]));

                                                        samples_remaining -= tr_headers[ch][0].RecordLength - samples_extradata[ch];
                                                        samples_extradata[ch] = 0;

                                                        adq14_triggered_streaming_process_record(record_data, &tr_headers[ch][0]);
                                                        ndsInfoStream(m_node) << "Completed record " << tr_headers[ch][0].RecordNumber <<
                                                            " on channel " << ch << ", " << tr_headers[ch][0].RecordLength << " samples." << std::endl;
                                                    }
                                                }
                                                else
                                                {
                                                    if (headers_done == 0)
                                                    {
                                                        // The samples in the transfer buffer begin a new record, this
                                                        // record is incomplete.
                                                        std::memcpy(tr_extra[ch], tr_buffers[ch], sizeof(short)*samples_added[ch]);
                                                        samples_remaining -= samples_added[ch];
                                                        samples_extradata[ch] = samples_added[ch];
                                                    }
                                                    else
                                                    {

                                                        // Copy data to record buffer
                                                        std::memcpy((void*)record_data, tr_buffers[ch], sizeof(short)*tr_headers[ch][0].RecordLength);
                                                        samples_remaining -= tr_headers[ch][0].RecordLength;

                                                        adq14_triggered_streaming_process_record(record_data, &tr_headers[ch][0]);
                                                        ndsInfoStream(m_node) << "Completed record " << tr_headers[ch][0].RecordNumber <<
                                                            " on channel " << ch << ", " << tr_headers[ch][0].RecordLength << " samples." << std::endl;
                                                    }
                                                }
                                                // At this point: the first record in the transfer buffer or the entire
                                                // transfer buffer has been parsed.

                                                // Loop through complete records fully inside the buffer
                                                for (i = 1; i < headers_done; ++i)
                                                {
                                                    // Copy data to record buffer
                                                    std::memcpy((void*)record_data, (&tr_buffers[ch][samples_added[ch] - samples_remaining]), sizeof(short)*tr_headers[ch][i].RecordLength);

                                                    samples_remaining -= tr_headers[ch][i].RecordLength;

                                                    adq14_triggered_streaming_process_record(record_data, &tr_headers[ch][i]);
                                                    ndsInfoStream(m_node) << "Completed record " << tr_headers[ch][i].RecordNumber <<
                                                        " on channel " << ch << ", " << tr_headers[ch][i].RecordLength << " samples." << std::endl;
                                                }

                                                if (samples_remaining > 0)
                                                {
                                                    // There is an incomplete record at the end of the transfer buffer
                                                    // Copy the incomplete header to the start of the target_headers buffer
                                                    std::memcpy(tr_headers[ch], &tr_headers[ch][headers_done], sizeof(StreamingHeader_t));

                                                    // Copy any remaining samples to the target_buffers_extradata buffer,
                                                    // they belong to the incomplete record
                                                    std::memcpy(tr_extra[ch], &tr_buffers[ch][samples_added[ch] - samples_remaining], sizeof(short)*samples_remaining);
                                                    // printf("Incomplete at end of transfer buffer. %u samples.\n", samples_remaining);
                                                    // printf("Copying %u samples to the extradata buffer\n", samples_remaining);
                                                    samples_extradata[ch] = samples_remaining;
                                                    samples_remaining = 0;
                                                }
                                            }

                                            // Read buffers by each channel and send them to DATA PVs
                                            //tr_buffers_ptr = (double*)tr_buffers[ch];
                                            m_AIChannels[ch]->read_trigstr(tr_buffers[ch], total_samples);
                                        }

                                        // Update received_all_records
                                        nof_received_records_sum = 0;
                                        for (ch = 0; ch < 4; ++ch)
                                        {
                                            nof_received_records_sum += records_completed[ch];
                                        }

                                        // Determine if collection is completed
                                        received_all_records = (nof_received_records_sum >= nof_records_sum);

                                    }
                                } while (!received_all_records);
                            }
                        }
                        break;
                    case 1: // External trigger  ----- need to investigate the API doc to develop this method
                        break;
                    case 2: // Level trigger ----- need to investigate the API doc to develop this method
                        ndsInfoStream(m_node) << "Issuing Level trigger... " << std::endl;
                        break;
                    case 3: // Internal trigger ----- need to investigate the API doc to develop this method
                        ndsInfoStream(m_node) << "Issuing Internal trigger... " << std::endl;
                        success = m_adq_dev->SetInternalTriggerPeriod(m_triglvl); // it uses FREQ/Period variable, not lvl
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at SetInternalTriggerPeriod." << std::endl;
                            goto finish;
                        }
                        else
                        {

                        }
                        break;
                    }
                }
            }
        }
    }

finish:
    ndsInfoStream(m_node) << "Acquisition finished." << std::endl;
    commitChanges(true);
    m_adq_dev->StopStreaming();

    for (ch = 0; ch < 4; ch++) {
        if (tr_buffers[ch])
            free(tr_buffers[ch]);
        if (tr_headers[ch])
            free(tr_headers[ch]);
        if (tr_extra[ch])
            free(tr_extra[ch]);
    }

    try {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) {
        /* We are probably already in "stopping", no need to panic... */
    }
}

void ADQAIChannelGroup::adq14_triggered_streaming_process_record(short* record_data, StreamingHeader_t* record_header)  // Need to create PV for average samples, probably it is important, since they have this method
{
    // You may process a full record here (will be called once with every completed record)
#ifdef PRINT_RECORD_INFO
    // Calculate average over all samples (as an example of processing a record)
    int64_t acc_result;
    double  acc_result_d;
    unsigned int k;
    unsigned int m_tr_bufsize = 512 * 1024;

    acc_result = 0;
    for (k = 0; k < record_header->RecordLength; k++)
    {
        acc_result += record_data[k];
    }
    acc_result_d = (double)acc_result / (double)record_header->RecordLength;

    // Print record info
    ndsInfoStream(m_node) << "--------------------------------------" << std::endl;
    ndsInfoStream(m_node) << " Device (SPD-" << (int)record_header->SerialNumber  << "), Channel " << (int)record_header->Channel << ", Record " << record_header->RecordNumber << std::endl;
    ndsInfoStream(m_node) << " Samples         = " << record_header->RecordLength << std::endl;
    ndsInfoStream(m_node) << " Status          = " << (int)record_header->RecordStatus << std::endl;
    ndsInfoStream(m_node) << " Timestamp       = " << record_header->Timestamp << std::endl;
    ndsInfoStream(m_node) << " Average samples = " << acc_result_d << std::endl;
    ndsInfoStream(m_node) << "--------------------------------------" << std::endl;
#endif
}

void ADQAIChannelGroup::acquisition_multirec()  // doesn't work yet, fails at GetData
{
    int trigged;
    unsigned int total_samples;
    short* buf_a = NULL;
    short* buf_b = NULL;
    short* buf_c = NULL;
    short* buf_d = NULL;
    void* tr_buffers[8]; // GetData allows for a digitizer with max 8 channels, the unused pointers should be null pointers

    short* tr_buffers_ptr;

    success = m_adq_dev->MultiRecordSetup(m_nofrecords, m_nofsamples);
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at MultiRecordSetup." << std::endl;
        goto finish;
    }

    if (m_trigmode == 0) // SW trigger
    {
        ndsInfoStream(m_node) << "Issuing Software trigger... " << std::endl;
        success = m_adq_dev->DisarmTrigger();
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
            goto finish;
        }
        else
        {
            success = m_adq_dev->ArmTrigger();
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at ArmTrigger." << std::endl;
                goto finish;
            }
            else
            {
                success = m_adq_dev->SWTrig();
                if (!success)
                {
                    ndsErrorStream(m_node) << "ERROR: Failed at SWTrig." << std::endl;
                    goto finish;
                }
                else
                {
                    do
                    {
                        trigged = m_adq_dev->GetAcquiredAll();
                        success = m_adq_dev->SWTrig();
                        if (!success)
                        {
                            ndsErrorStream(m_node) << "ERROR: Failed at SWTrig." << std::endl;
                            goto finish;
                        }
                    } while (trigged == 0);
                }
            }
        }
    }

    if (m_trigmode == 1 || m_trigmode == 2 || m_trigmode == 3) // External or Level or Internal trigger
    {
        ndsWarningStream(m_node) << "Trigger the device to collect data." << std::endl;
        sleep(10);

        success = m_adq_dev->DisarmTrigger();
        if (!success)
        {
            ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
            goto finish;
        }
        else
        {
            success = m_adq_dev->ArmTrigger();
            if (!success)
            {
                ndsErrorStream(m_node) << "ERROR: Failed at ArmTrigger." << std::endl;
                goto finish;
            }
            else
            {
                do
                {
                    trigged = m_adq_dev->GetAcquiredAll();
                } while (trigged == 0);
            }
        }
    }
    ndsInfoStream(m_node) << "All records are triggered." << std::endl;

    success = m_adq_dev->GetStreamOverflow();
    if (success)
    {
        ndsErrorStream(m_node) << "ERROR: Streaming overflow detected." << std::endl;
        goto finish;
    }

    total_samples = m_collect_records * m_nofsamples;

    buf_a = (short*)calloc(total_samples, sizeof(short));
    buf_b = (short*)calloc(total_samples, sizeof(short));
    buf_c = (short*)calloc(total_samples, sizeof(short));
    buf_d = (short*)calloc(total_samples, sizeof(short));

    // Create a pointer array containing the data buffer pointers
    tr_buffers[0] = (void*)buf_a;
    tr_buffers[1] = (void*)buf_b;
    tr_buffers[2] = (void*)buf_c;
    tr_buffers[3] = (void*)buf_d;

    if (buf_a == NULL || buf_b == NULL || buf_c == NULL || buf_d == NULL)
    {
        ndsErrorStream(m_node) << "ERROR: Buffer(s) == NULL" << std::endl;
        goto finish;
    }

    success = m_adq_dev->GetData(tr_buffers, total_samples, sizeof(short), 0, m_collect_records, channelmask_char, 0, m_nofsamples, ADQ_TRANSFER_MODE_NORMAL);
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at GetData." << std::endl;
        goto finish;
    }

    success = m_adq_dev->DisarmTrigger();
    if (!success)
    {
        ndsErrorStream(m_node) << "ERROR: Failed at DisarmTrigger." << std::endl;
        goto finish;
    }

    // Read buffers by each channel and send them to DATA PVs

    for (ch = 0; ch < nofchan; ++ch)
    {
        // Read buffers by each channel and send them to DATA PVs
        tr_buffers_ptr = ((short*)tr_buffers[ch]);
        //m_AIChannels[ch]->read_multirec(tr_buffers[ch], total_samples);
        m_AIChannels[ch]->read_trigstr(tr_buffers_ptr, total_samples);
    }


finish:
    ndsInfoStream(m_node) << "Acquisition finished." << std::endl;
    commitChanges(true);
    m_adq_dev->StopStreaming();

    if (buf_a != NULL)
        free(buf_a);
    if (buf_b != NULL)
        free(buf_b);
    if (buf_c != NULL)
        free(buf_c);
    if (buf_d != NULL)
        free(buf_d);

    try {
        m_stateMachine.setState(nds::state_t::on);
    }
    catch (nds::StateMachineNoSuchTransition error) {
        /* We are probably already in "stopping", no need to panic... */
    }
}

void ADQAIChannelGroup::acquisition_contstream() // ----- need to investigate the API doc to develop this method
{

}
