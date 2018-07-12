#include <cstdlib>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"
#include "ADQInfo.h"

ADQInfo::ADQInfo(const std::string& name, nds::Node& parentNode, ADQInterface*& adqDev) :
    m_node(nds::Port(name + INFO_DEVICE, nds::nodeType_t::generic)), 
    m_adqDevPtr(adqDev),
    m_productNamePV(nds::PVDelegateIn<std::string>("ProdName", std::bind(&ADQInfo::getProductName, this,
                                                                         std::placeholders::_1, std::placeholders::_2))),
    m_serialNumberPV(nds::PVDelegateIn<std::string>("ProdSerial", std::bind(&ADQInfo::getSerialNumber, this,
                                                                            std::placeholders::_1, std::placeholders::_2))),
    m_productIDPV(nds::PVDelegateIn<int32_t>("ProdID", std::bind(&ADQInfo::getProductID, this, std::placeholders::_1,
                                                                 std::placeholders::_2))),
    m_adqTypePV(nds::PVDelegateIn<int32_t>("ProdType", std::bind(&ADQInfo::getADQType, this, std::placeholders::_1,
                                                                 std::placeholders::_2))),
    m_cardOptionPV(nds::PVDelegateIn<std::string>("ProdOpt", std::bind(&ADQInfo::getCardOption, this,
                                                                       std::placeholders::_1, std::placeholders::_2))),
    m_tempLocalPV(nds::PVDelegateIn<int32_t>("TempLocal", std::bind(&ADQInfo::getTempLocal, this, std::placeholders::_1,
                                                                    std::placeholders::_2))),
    m_tempAdcOnePV(nds::PVDelegateIn<int32_t>("TempADC-1", std::bind(&ADQInfo::getTempADCone, this,
                                                                     std::placeholders::_1, std::placeholders::_2))),
    m_tempAdcTwoPV(nds::PVDelegateIn<int32_t>("TempADC-2", std::bind(&ADQInfo::getTempADCtwo, this,
                                                                     std::placeholders::_1, std::placeholders::_2))),
    m_tempFpgaPV(nds::PVDelegateIn<int32_t>("TempFPGA", std::bind(&ADQInfo::getTempFPGA, this, std::placeholders::_1,
                                                                  std::placeholders::_2))),
    m_tempDiodPV(nds::PVDelegateIn<int32_t>("TempDiod", std::bind(&ADQInfo::getTempDd, this, std::placeholders::_1,
                                                                  std::placeholders::_2))),
    m_sampRatePV(nds::PVDelegateIn<double>("SampRate", std::bind(&ADQInfo::getSampRate, this, std::placeholders::_1,
                                                                 std::placeholders::_2))),
    m_busTypePV(nds::PVDelegateIn<int32_t>("BusType", std::bind(&ADQInfo::getBusType, this, std::placeholders::_1,
                                                                std::placeholders::_2))),
    m_busAddrPV(nds::PVDelegateIn<int32_t>("BusAddr", std::bind(&ADQInfo::getBusAddr, this, std::placeholders::_1,
                                                                std::placeholders::_2))),
    m_pcieLinkRatePV(nds::PVDelegateIn<int32_t>("PCIeLinkRate", std::bind(&ADQInfo::getPCIeLinkRate, this,
                                                                          std::placeholders::_1, std::placeholders::_2))),
    m_pcieLinkWidPV(nds::PVDelegateIn<int32_t>("PCIeLinkWid", std::bind(&ADQInfo::getPCIeLinkWid, this,
                                                                        std::placeholders::_1, std::placeholders::_2)))
{
    parentNode.addChild(m_node);

    // PVs for device info
    m_productNamePV.setScanType(nds::scanType_t::interrupt);
    m_productNamePV.setMaxElements(STRING_ENUM);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_productNamePV);

    m_serialNumberPV.setScanType(nds::scanType_t::interrupt);
    m_serialNumberPV.setMaxElements(STRING_ENUM);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_serialNumberPV);

    m_productIDPV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_productIDPV);

    m_adqTypePV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_adqTypePV);

    m_cardOptionPV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_cardOptionPV.setMaxElements(STRING_ENUM);
    m_node.addChild(m_cardOptionPV);

    // PVs for temperatures
    m_tempLocalPV.setScanType(nds::scanType_t::periodic, 1);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_tempLocalPV);

    m_tempAdcOnePV.setScanType(nds::scanType_t::periodic, 1);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_tempAdcOnePV);

    m_tempAdcTwoPV.setScanType(nds::scanType_t::periodic, 1);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_tempAdcTwoPV);

    m_tempFpgaPV.setScanType(nds::scanType_t::periodic, 1);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_tempFpgaPV);

    m_tempDiodPV.setScanType(nds::scanType_t::periodic, 1);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_tempDiodPV);

    // PV for sample rate
    m_sampRatePV.setScanType(nds::scanType_t::periodic, 1);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_sampRatePV);

    // PV for Bus connection
    m_busAddrPV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_busAddrPV);

    m_busTypePV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_busTypePV);

    m_pcieLinkRatePV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_pcieLinkRatePV);

    m_pcieLinkWidPV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_pcieLinkWidPV);
}

void ADQInfo::getProductName(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adqDevPtr->GetBoardProductName();
    *pTimestamp = m_productNamePV.getTimestamp();
}

void ADQInfo::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adqDevPtr->GetBoardSerialNumber();
    *pTimestamp = m_serialNumberPV.getTimestamp();
}

void ADQInfo::getProductID(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetProductID();
    *pTimestamp = m_productIDPV.getTimestamp();
}

void ADQInfo::getADQType(timespec* pTimestamp, int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetADQType();
    *pTimestamp = m_adqTypePV.getTimestamp();
}

void ADQInfo::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adqDevPtr->GetCardOption();
    *pTimestamp = m_cardOptionPV.getTimestamp();
}

void ADQInfo::getTempLocal(timespec* pTimestamp, int32_t* pValue)
{
    {
        std::lock_guard<std::mutex> lock(adqDevMutex);
        *pValue = m_adqDevPtr->GetTemperature(TEMP_LOCAL) * CELSIUS_CONVERT;
    }
    *pTimestamp = m_tempLocalPV.getTimestamp();
}

void ADQInfo::getTempADCone(timespec* pTimestamp, int32_t* pValue)
{
    {
        std::lock_guard<std::mutex> lock(adqDevMutex);
        *pValue = m_adqDevPtr->GetTemperature(TEMPADC_ONE) * CELSIUS_CONVERT;
    }
    *pTimestamp = m_tempAdcOnePV.getTimestamp();
}

void ADQInfo::getTempADCtwo(timespec* pTimestamp, int32_t* pValue)
{
    {
        std::lock_guard<std::mutex> lock(adqDevMutex);
        *pValue = m_adqDevPtr->GetTemperature(TEMPADC_TWO) * CELSIUS_CONVERT;
    }
    *pTimestamp = m_tempAdcTwoPV.getTimestamp();
}

void ADQInfo::getTempFPGA(timespec* pTimestamp, int32_t* pValue)
{
    {
        std::lock_guard<std::mutex> lock(adqDevMutex);
        *pValue = m_adqDevPtr->GetTemperature(TEMP_FPGA) * CELSIUS_CONVERT;
    }
    *pTimestamp = m_tempFpgaPV.getTimestamp();
}

void ADQInfo::getTempDd(timespec* pTimestamp, int32_t* pValue)
{
    {
        std::lock_guard<std::mutex> lock(adqDevMutex);
        *pValue = m_adqDevPtr->GetTemperature(TEMP_DIOD) * CELSIUS_CONVERT;
    }
    *pTimestamp = m_tempDiodPV.getTimestamp();
}

void ADQInfo::getSampRate(timespec* pTimestamp, double* pValue)
{
    double sampRate = 0;
    m_adqDevPtr->GetSampleRate(0, &sampRate);
    *pValue = sampRate;
    *pTimestamp = m_sampRatePV.getTimestamp();
}

void ADQInfo::getBusAddr(timespec* pTimestamp, int32_t* pValue)
{
    if ((m_adqDevPtr->IsPCIeDevice()) || (m_adqDevPtr->IsPCIeLiteDevice()))
    {
        *pValue = m_adqDevPtr->GetPCIeAddress();
    }

    if ((m_adqDevPtr->IsUSBDevice()) || (m_adqDevPtr->IsUSB3Device()))
    {
        *pValue = m_adqDevPtr->GetUSBAddress();
    }

    *pTimestamp = m_busAddrPV.getTimestamp();
}

void ADQInfo::getBusType(timespec* pTimestamp, int32_t* pValue)
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

    *pTimestamp = m_busTypePV.getTimestamp();
}

void ADQInfo::getPCIeLinkRate(timespec* pTimestamp, int32_t* pValue)
{
    if ((m_adqDevPtr->IsPCIeDevice()) || (m_adqDevPtr->IsPCIeLiteDevice()))
    {
        *pValue = m_adqDevPtr->GetPCIeLinkRate();
    }

    *pTimestamp = m_pcieLinkRatePV.getTimestamp();
}

void ADQInfo::getPCIeLinkWid(timespec* pTimestamp, int32_t* pValue)
{
    if ((m_adqDevPtr->IsUSBDevice()) || (m_adqDevPtr->IsUSB3Device()))
    {
        *pValue = m_adqDevPtr->GetPCIeLinkWidth();
    }

    *pTimestamp = m_pcieLinkWidPV.getTimestamp();
}
