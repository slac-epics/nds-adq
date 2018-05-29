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
                                                                        std::placeholders::_2))),
    m_templocalPV(nds::PVDelegateIn<std::int32_t>("TemperatureLocal", std::bind(&ADQInfo::getTempLocal,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempadcoPV(nds::PVDelegateIn<std::int32_t>("TemperatureADC-1", std::bind(&ADQInfo::getTempADCone,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempadctPV(nds::PVDelegateIn<std::int32_t>("TemperatureADC-2", std::bind(&ADQInfo::getTempADCtwo,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempfpgaPV(nds::PVDelegateIn<std::int32_t>("TemperatureFPGA", std::bind(&ADQInfo::getTempFPGA,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempddPV(nds::PVDelegateIn<std::int32_t>("TempratureDiode", std::bind(&ADQInfo::getTempDd,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);


    //// urojec L3: where does the 32 come from and why is it hardcoded. Espetially if
    ////            used at so many places define a macro or static const variable
    // PVs for device info
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

    // PVs for temperatures
    m_templocalPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_templocalPV);

    m_tempadcoPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempadcoPV);

    m_tempadctPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempadctPV);

    m_tempfpgaPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempfpgaPV);

    m_tempddPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempddPV);

}

void ADQInfo::getProductName(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adq_dev->GetBoardProductName();
}

void ADQInfo::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adq_dev->GetBoardSerialNumber();
}

void ADQInfo::getProductID(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adq_dev->GetProductID();
}

void ADQInfo::getADQType(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adq_dev->GetADQType();;
}

void ADQInfo::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adq_dev->GetCardOption();
}



//// urojec L3: Hardocoding, please define macros or static const or enum for all this
void ADQInfo::getTempLocal(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adq_dev->GetTemperature(0) / 256;
}

void ADQInfo::getTempADCone(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adq_dev->GetTemperature(1) / 256;
}

void ADQInfo::getTempADCtwo(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adq_dev->GetTemperature(2) / 256;
}

void ADQInfo::getTempFPGA(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adq_dev->GetTemperature(3) / 256;
}

void ADQInfo::getTempDd(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adq_dev->GetTemperature(4) / 256;
}
