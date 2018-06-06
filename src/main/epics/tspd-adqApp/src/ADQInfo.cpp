#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQInfo.h"
#include "ADQFourteen.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQInfo::ADQInfo(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_adqDevPtr(adqDev),
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
    m_tempLocalPV(nds::PVDelegateIn<std::int32_t>("TemperatureLocal", std::bind(&ADQInfo::getTempLocal,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempAdcOnePV(nds::PVDelegateIn<std::int32_t>("TemperatureADC-1", std::bind(&ADQInfo::getTempADCone,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempAdcTwoPV(nds::PVDelegateIn<std::int32_t>("TemperatureADC-2", std::bind(&ADQInfo::getTempADCtwo,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempFpgaPV(nds::PVDelegateIn<std::int32_t>("TemperatureFPGA", std::bind(&ADQInfo::getTempFPGA,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempDiodPV(nds::PVDelegateIn<std::int32_t>("TemperatureDiode", std::bind(&ADQInfo::getTempDd,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);

    // PVs for device info
    m_productNamePV.setScanType(nds::scanType_t::interrupt);
    m_productNamePV.setMaxElements(STRING_ENUM);
    m_node.addChild(m_productNamePV);

    m_serialNumberPV.setScanType(nds::scanType_t::interrupt);
    m_serialNumberPV.setMaxElements(STRING_ENUM);
    m_node.addChild(m_serialNumberPV);

    m_productIDPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_productIDPV);

    m_adqTypePV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_adqTypePV);

    m_cardOptionPV.setScanType(nds::scanType_t::interrupt);
    m_cardOptionPV.setMaxElements(STRING_ENUM);
    m_node.addChild(m_cardOptionPV);

    // PVs for temperatures
    m_tempLocalPV.setScanType(nds::scanType_t::periodic, 1);
    m_node.addChild(m_tempLocalPV);

    m_tempAdcOnePV.setScanType(nds::scanType_t::periodic, 1);
    m_node.addChild(m_tempAdcOnePV);

    m_tempAdcTwoPV.setScanType(nds::scanType_t::periodic, 1);
    m_node.addChild(m_tempAdcTwoPV);

    m_tempFpgaPV.setScanType(nds::scanType_t::periodic, 1);
    m_node.addChild(m_tempFpgaPV);

    m_tempDiodPV.setScanType(nds::scanType_t::periodic, 1);
    m_node.addChild(m_tempDiodPV);
}

void ADQInfo::getProductName(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adqDevPtr->GetBoardProductName();
}

void ADQInfo::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adqDevPtr->GetBoardSerialNumber();
}

void ADQInfo::getProductID(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetProductID();
}

void ADQInfo::getADQType(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetADQType();
}

void ADQInfo::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adqDevPtr->GetCardOption();
}

void ADQInfo::getTempLocal(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(TEMP_LOCAL)*CELSIUS_CONVERT;
}

void ADQInfo::getTempADCone(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(TEMPADC_ONE)*CELSIUS_CONVERT;
}

void ADQInfo::getTempADCtwo(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(TEMPADC_TWO)*CELSIUS_CONVERT;
}

void ADQInfo::getTempFPGA(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(TEMP_FPGA)*CELSIUS_CONVERT;
}

void ADQInfo::getTempDd(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(TEMP_DIOD)*CELSIUS_CONVERT;
}