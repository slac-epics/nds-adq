#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQInfo::ADQInfo(const std::string& name, nds::Node& parentNode, ADQInterface *& adq_dev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adq_dev(adq_dev),
    m_productNamePV(nds::PVDelegateIn<std::string>("ProductName", std::bind(&ADQInfo::getProductName,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_serialNumberPV(nds::PVDelegateIn<std::string>("SerialNumber", std::bind(&ADQInfo::getSerialNumber,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_productIDPV(nds::PVDelegateIn<std::int32_t>("ProductID", std::bind(&ADQInfo::getProductID,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_adqTypePV(nds::PVDelegateIn<std::int32_t>("ADQType", std::bind(&ADQInfo::getADQType,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_cardOptionPV(nds::PVDelegateIn<std::string>("CardOption", std::bind(&ADQInfo::getCardOption,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);

    // Set PVs for device info
    m_productNamePV.setScanType(nds::scanType_t::interrupt);
    m_productNamePV.setMaxElements(32);
    m_node.addChild(m_productNamePV);

    m_serialNumberPV.setScanType(nds::scanType_t::interrupt);
    m_serialNumberPV.setMaxElements(32);
    m_node.addChild(m_serialNumberPV);

    m_productIDPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_productIDPV);

    m_adqTypePV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_adqTypePV);

    m_cardOptionPV.setScanType(nds::scanType_t::interrupt);
    m_cardOptionPV.setMaxElements(32);
    m_node.addChild(m_cardOptionPV);

}

void ADQInfo::getProductName(timespec* pTimestamp, std::string* pValue)
{
    char* adq_pn = m_adq_dev->GetBoardProductName();
    *pValue = adq_pn;
}

void ADQInfo::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    char* adq_sn = m_adq_dev->GetBoardSerialNumber();
    *pValue = adq_sn;
}

void ADQInfo::getProductID(timespec* pTimestamp, std::int32_t* pValue)
{
    unsigned int adq_pid = m_adq_dev->GetProductID();
    *pValue = adq_pid;
}

void ADQInfo::getADQType(timespec* pTimestamp, std::int32_t* pValue)
{
    int adq_type = m_adq_dev->GetADQType();
    *pValue = adq_type;
}

void ADQInfo::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    const char* adq_opt = m_adq_dev->GetCardOption();
    *pValue = adq_opt;
}

