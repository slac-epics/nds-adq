#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>
#include <ctime>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQInfo.h"
#include "ADQDevice.h"
#include "ADQDefinition.h"
#include "ADQFourteen.h"
#include "ADQSeven.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQInfo::ADQInfo(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev) :
    m_node(nds::Port(name + INFO_DEVICE, nds::nodeType_t::generic)),
    m_adqDevPtr(adqDev),
    m_productNamePV(nds::PVDelegateIn<std::string>("ProdName", std::bind(&ADQInfo::getProductName,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_serialNumberPV(nds::PVDelegateIn<std::string>("ProdSerial", std::bind(&ADQInfo::getSerialNumber,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_productIDPV(nds::PVDelegateIn<std::int32_t>("ProdID", std::bind(&ADQInfo::getProductID,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_adqTypePV(nds::PVDelegateIn<std::int32_t>("ProdType", std::bind(&ADQInfo::getADQType,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_cardOptionPV(nds::PVDelegateIn<std::string>("ProdOpt", std::bind(&ADQInfo::getCardOption,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempLocalPV(nds::PVDelegateIn<std::int32_t>("TempLocal", std::bind(&ADQInfo::getTempLocal,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempAdcOnePV(nds::PVDelegateIn<std::int32_t>("TempADC-1", std::bind(&ADQInfo::getTempADCone,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempAdcTwoPV(nds::PVDelegateIn<std::int32_t>("TempADC-2", std::bind(&ADQInfo::getTempADCtwo,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempFpgaPV(nds::PVDelegateIn<std::int32_t>("TempFPGA", std::bind(&ADQInfo::getTempFPGA,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempDiodPV(nds::PVDelegateIn<std::int32_t>("TempDiod", std::bind(&ADQInfo::getTempDd,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_sampRatePV(nds::PVDelegateIn<double>("SampRate", std::bind(&ADQInfo::getSampRate,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_busAddrPV(nds::PVDelegateIn<std::int32_t>("BusAddr", std::bind(&ADQInfo::getBusAddr,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_busTypePV(nds::PVDelegateIn<std::int32_t>("BusType", std::bind(&ADQInfo::getBusType,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_pcieLinkRatePV(nds::PVDelegateIn<std::int32_t>("PCIeLinkRate", std::bind(&ADQInfo::getPCIeLinkRate,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_pcieLinkWidPV(nds::PVDelegateIn<std::int32_t>("PCIeLinkWid", std::bind(&ADQInfo::getPCIeLinkWid,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2)))
{
    parentNode.addChild(m_node);

    // PVs for device info
    m_productNamePV.setScanType(nds::scanType_t::interrupt);
    m_productNamePV.setMaxElements(STRING_ENUM);
    m_productNamePV.processAtInit(PINI);
    m_node.addChild(m_productNamePV);   

    m_serialNumberPV.setScanType(nds::scanType_t::interrupt);
    m_serialNumberPV.setMaxElements(STRING_ENUM);
    m_serialNumberPV.processAtInit(PINI);
    m_node.addChild(m_serialNumberPV);

    m_productIDPV.setScanType(nds::scanType_t::interrupt);
    m_productIDPV.processAtInit(PINI);
    m_node.addChild(m_productIDPV);

    m_adqTypePV.setScanType(nds::scanType_t::interrupt);
    m_adqTypePV.processAtInit(PINI);
    m_node.addChild(m_adqTypePV);

    m_cardOptionPV.setScanType(nds::scanType_t::interrupt);
    m_cardOptionPV.setMaxElements(STRING_ENUM);
    m_cardOptionPV.processAtInit(PINI);
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

    // PV for sample rate 
    m_sampRatePV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_sampRatePV);

    // PV for Bus connection
    m_busAddrPV.setScanType(nds::scanType_t::interrupt);
    m_busAddrPV.processAtInit(PINI);
    m_node.addChild(m_busAddrPV);

    m_busTypePV.setScanType(nds::scanType_t::interrupt);
    m_busTypePV.processAtInit(PINI);
    m_node.addChild(m_busTypePV);

    m_pcieLinkRatePV.setScanType(nds::scanType_t::interrupt);
    m_pcieLinkRatePV.processAtInit(PINI);
    m_node.addChild(m_pcieLinkRatePV);

    m_pcieLinkWidPV.setScanType(nds::scanType_t::interrupt);
    m_pcieLinkWidPV.processAtInit(PINI);
    m_node.addChild(m_pcieLinkWidPV);
    
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

void ADQInfo::getSampRate(timespec* pTimestamp, double* pValue)
{
    double* sampRate;
    m_adqDevPtr->GetSampleRate(0, sampRate);
    pValue = sampRate;
}

void ADQInfo::getBusAddr(timespec* pTimestamp, std::int32_t* pValue)
{
    if ((m_adqDevPtr->IsPCIeDevice()) || (m_adqDevPtr->IsPCIeLiteDevice()))
    {
        *pValue = m_adqDevPtr->GetPCIeAddress();
    }
    
    if ((m_adqDevPtr->IsUSBDevice()) || (m_adqDevPtr->IsUSB3Device()))
    {
        *pValue = m_adqDevPtr->GetUSBAddress();
    }
}

void ADQInfo::getBusType(timespec* pTimestamp, std::int32_t* pValue)
{
    if (m_adqDevPtr->IsPCIeDevice())
    {
        *pValue = 0;
    }

    if (m_adqDevPtr->IsPCIeLiteDevice())
    {
        *pValue = 1;
    }

    if (m_adqDevPtr->IsUSBDevice())
    {
        *pValue = 2;
    }

    if (m_adqDevPtr->IsUSB3Device())
    {
        *pValue = 3;
    }
}

void ADQInfo::getPCIeLinkRate(timespec* pTimestamp, std::int32_t* pValue)
{
    if ((m_adqDevPtr->IsPCIeDevice()) || (m_adqDevPtr->IsPCIeLiteDevice()))
    {
        *pValue = m_adqDevPtr->GetPCIeLinkRate();
    }
}

void ADQInfo::getPCIeLinkWid(timespec* pTimestamp, std::int32_t* pValue)
{
    if ((m_adqDevPtr->IsUSBDevice()) || (m_adqDevPtr->IsUSB3Device()))
    {
        *pValue = m_adqDevPtr->GetPCIeLinkWidth();
    }
}