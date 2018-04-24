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

    setDeviceInfo();
}

void ADQInfo::setDeviceInfo()
{
    std::int32_t tmp_int;
    std::string tmp_str;
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);

    m_productNamePV.read(&now, &tmp_str);
    m_productNamePV.push(now, tmp_str);

    m_serialNumberPV.read(&now, &tmp_str);
    m_serialNumberPV.push(now, tmp_str);

    m_productIDPV.read(&now, &tmp_int);
    m_productIDPV.push(now, tmp_int);

    m_adqTypePV.read(&now, &tmp_int);
    m_adqTypePV.push(now, tmp_int);

    m_cardOptionPV.read(&now, &tmp_str);
    m_cardOptionPV.push(now, tmp_str);

    std::cout << "Device info is received." << std::endl;

}

void ADQInfo::getProductName(timespec* pTimestamp, std::string* pValue)
{
    char* adq_pn = m_adq_dev->GetBoardProductName();
    *pValue = std::string(adq_pn);
}

void ADQInfo::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    char* adq_sn = m_adq_dev->GetBoardSerialNumber();
    *pValue = std::string(adq_sn);
}

void ADQInfo::getProductID(timespec* pTimestamp, std::int32_t* pValue)
{
    unsigned int adq_pid = m_adq_dev->GetProductID();
    *pValue = std::int32_t(adq_pid);
}

void ADQInfo::getADQType(timespec* pTimestamp, std::int32_t* pValue)
{
    int adq_type = m_adq_dev->GetADQType();
    *pValue = std::int32_t(adq_type);
}

void ADQInfo::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    const char* adq_opt = m_adq_dev->GetCardOption();
    *pValue = std::string(adq_opt);
}

