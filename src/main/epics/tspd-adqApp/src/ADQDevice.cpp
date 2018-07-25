//
// Copyright (c) 2018 Cosylab d.d.
// This software is distributed under the terms found
// in file LICENSE.txt that is included with this distribution.
//

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQAIChannel.h"
#include "ADQAIChannelGroup.h"
#include "ADQDefinition.h"
#include "ADQDevice.h"
#include "ADQInit.h"

ADQDevice::ADQDevice(const std::string& name, nds::Node& parentNode, ADQInterface*& adqInterface) :
    m_node(nds::Port(name + INFO_DEVICE, nds::nodeType_t::generic)), 
    m_adqInterface(adqInterface),
    m_productNamePV(nds::PVDelegateIn<std::string>("ProdName", std::bind(&ADQDevice::getProductName, this,
                                                                         std::placeholders::_1, std::placeholders::_2))),
    m_serialNumberPV(nds::PVDelegateIn<std::string>("ProdSerial", std::bind(&ADQDevice::getSerialNumber, this,
                                                                            std::placeholders::_1, std::placeholders::_2))),
    m_productIDPV(nds::PVDelegateIn<int32_t>("ProdID", std::bind(&ADQDevice::getProductID, this, std::placeholders::_1,
                                                                 std::placeholders::_2))),
    m_adqTypePV(nds::PVDelegateIn<int32_t>("ProdType", std::bind(&ADQDevice::getADQType, this, std::placeholders::_1,
                                                                 std::placeholders::_2))),
    m_cardOptionPV(nds::PVDelegateIn<std::string>("ProdOpt", std::bind(&ADQDevice::getCardOption, this,
                                                                       std::placeholders::_1, std::placeholders::_2))),
    m_tempLocalPV(nds::PVDelegateIn<int32_t>("TempLocal", std::bind(&ADQDevice::getTempLocal, this,
                                                                    std::placeholders::_1, std::placeholders::_2))),
    m_tempAdcOnePV(nds::PVDelegateIn<int32_t>("TempADC-1", std::bind(&ADQDevice::getTempADCone, this,
                                                                     std::placeholders::_1, std::placeholders::_2))),
    m_tempAdcTwoPV(nds::PVDelegateIn<int32_t>("TempADC-2", std::bind(&ADQDevice::getTempADCtwo, this,
                                                                     std::placeholders::_1, std::placeholders::_2))),
    m_tempFpgaPV(nds::PVDelegateIn<int32_t>("TempFPGA", std::bind(&ADQDevice::getTempFPGA, this, std::placeholders::_1,
                                                                  std::placeholders::_2))),
    m_tempDiodPV(nds::PVDelegateIn<int32_t>("TempDiod", std::bind(&ADQDevice::getTempDd, this, std::placeholders::_1,
                                                                  std::placeholders::_2))),
    m_sampRatePV(nds::PVDelegateIn<double>("SampRate", std::bind(&ADQDevice::getSampRate, this, std::placeholders::_1,
                                                                 std::placeholders::_2))),
    m_bytesPerSampPV(nds::PVDelegateIn<int32_t>("BytesPerSample", std::bind(&ADQDevice::getBytesPerSample, this,
                                                                            std::placeholders::_1, std::placeholders::_2))),
    m_busTypePV(nds::PVDelegateIn<int32_t>("BusType", std::bind(&ADQDevice::getBusType, this, std::placeholders::_1,
                                                                std::placeholders::_2))),
    m_busAddrPV(nds::PVDelegateIn<int32_t>("BusAddr", std::bind(&ADQDevice::getBusAddr, this, std::placeholders::_1,
                                                                std::placeholders::_2))),
    m_pcieLinkRatePV(nds::PVDelegateIn<int32_t>("PCIeLinkRate", std::bind(&ADQDevice::getPCIeLinkRate, this,
                                                                          std::placeholders::_1, std::placeholders::_2))),
    m_pcieLinkWidPV(nds::PVDelegateIn<int32_t>("PCIeLinkWid", std::bind(&ADQDevice::getPCIeLinkWid, this,
                                                                        std::placeholders::_1, std::placeholders::_2))),
    m_sampRateDecPV(nds::PVDelegateIn<double>("SampRateDec", std::bind(&ADQDevice::getSampRateDec, this,
                                                                       std::placeholders::_1, std::placeholders::_2)))
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
    m_cardOptionPV.processAtInit(PINI);
    m_cardOptionPV.setMaxElements(STRING_ENUM);
    m_node.addChild(m_cardOptionPV);

    // PVs for temperatures
    m_tempLocalPV.setScanType(nds::scanType_t::periodic, 1);
    m_tempLocalPV.processAtInit(PINI);
    m_node.addChild(m_tempLocalPV);

    m_tempAdcOnePV.setScanType(nds::scanType_t::periodic, 1);
    m_tempAdcOnePV.processAtInit(PINI);
    m_node.addChild(m_tempAdcOnePV);

    m_tempAdcTwoPV.setScanType(nds::scanType_t::periodic, 1);
    m_tempAdcTwoPV.processAtInit(PINI);
    m_node.addChild(m_tempAdcTwoPV);

    m_tempFpgaPV.setScanType(nds::scanType_t::periodic, 1);
    m_tempFpgaPV.processAtInit(PINI);
    m_node.addChild(m_tempFpgaPV);

    m_tempDiodPV.setScanType(nds::scanType_t::periodic, 1);
    m_tempDiodPV.processAtInit(PINI);
    m_node.addChild(m_tempDiodPV);

    // PV for sample rate
    m_sampRatePV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_sampRatePV);

    m_sampRateDecPV.setScanType(nds::scanType_t::interrupt);
    m_sampRatePV.processAtInit(PINI);
    m_node.addChild(m_sampRateDecPV);

    // PV for number of bytes per sample
    m_bytesPerSampPV.setScanType(nds::scanType_t::interrupt);
    m_bytesPerSampPV.processAtInit(PINI);
    m_node.addChild(m_bytesPerSampPV);

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

void ADQDevice::getProductName(timespec* pTimestamp, std::string* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetBoardProductName();
    *pTimestamp = m_productNamePV.getTimestamp();
}

void ADQDevice::getSerialNumber(timespec* pTimestamp, std::string* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetBoardSerialNumber();
    *pTimestamp = m_serialNumberPV.getTimestamp();
}

void ADQDevice::getProductID(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetProductID();
    *pTimestamp = m_productIDPV.getTimestamp();
}

void ADQDevice::getADQType(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetADQType();
    *pTimestamp = m_adqTypePV.getTimestamp();
}

void ADQDevice::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetCardOption();
    *pTimestamp = m_cardOptionPV.getTimestamp();
}

void ADQDevice::getTempLocal(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetTemperature(TEMP_LOCAL) * CELSIUS_CONVERT;
    *pTimestamp = m_tempLocalPV.getTimestamp();
}

void ADQDevice::getTempADCone(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetTemperature(TEMP_ADC_ONE) * CELSIUS_CONVERT;
    *pTimestamp = m_tempAdcOnePV.getTimestamp();
}

void ADQDevice::getTempADCtwo(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetTemperature(TEMP_ADC_TWO) * CELSIUS_CONVERT;
    *pTimestamp = m_tempAdcTwoPV.getTimestamp();
}

void ADQDevice::getTempFPGA(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetTemperature(TEMP_FPGA) * CELSIUS_CONVERT;
    *pTimestamp = m_tempFpgaPV.getTimestamp();
}

void ADQDevice::getTempDd(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    *pValue = m_adqInterface->GetTemperature(TEMP_DIOD) * CELSIUS_CONVERT;
    *pTimestamp = m_tempDiodPV.getTimestamp();
}

void ADQDevice::getSampRate(timespec* pTimestamp, double* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    double sampRate = 0;
    m_adqInterface->GetSampleRate(0, &sampRate);
    *pValue = sampRate;
    *pTimestamp = m_sampRatePV.getTimestamp();
}

void ADQDevice::getSampRateDec(timespec* pTimestamp, double* pValue)
{
    double sampRateDec = 0;
    m_adqInterface->GetSampleRate(1, &sampRateDec);
    *pValue = sampRateDec;
    *pTimestamp = m_sampRateDecPV.getTimestamp();
}

void ADQDevice::getBytesPerSample(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    unsigned int bytes = 0;
    m_adqInterface->GetNofBytesPerSample(&bytes);
    *pValue = bytes;
    *pTimestamp = m_bytesPerSampPV.getTimestamp();
}

void ADQDevice::getBusAddr(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    if ((m_adqInterface->IsPCIeDevice()) || (m_adqInterface->IsPCIeLiteDevice()))
    {
        *pValue = m_adqInterface->GetPCIeAddress();
    }

    if ((m_adqInterface->IsUSBDevice()) || (m_adqInterface->IsUSB3Device()))
    {
        *pValue = m_adqInterface->GetUSBAddress();
    }

    *pTimestamp = m_busAddrPV.getTimestamp();
}

void ADQDevice::getBusType(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    if (m_adqInterface->IsPCIeDevice())
    {
        *pValue = 0;
    }

    if (m_adqInterface->IsPCIeLiteDevice())
    {
        *pValue = 1;
    }

    if (m_adqInterface->IsUSBDevice())
    {
        *pValue = 2;
    }

    if (m_adqInterface->IsUSB3Device())
    {
        *pValue = 3;
    }

    *pTimestamp = m_busTypePV.getTimestamp();
}

void ADQDevice::getPCIeLinkRate(timespec* pTimestamp, int32_t* pValue)
{
    std::lock_guard<std::mutex> lock(m_adqDevMutex);
    if ((m_adqInterface->IsPCIeDevice()) || (m_adqInterface->IsPCIeLiteDevice()))
    {
        *pValue = m_adqInterface->GetPCIeLinkRate();
    }

    *pTimestamp = m_pcieLinkRatePV.getTimestamp();
}

void ADQDevice::getPCIeLinkWid(timespec* pTimestamp, int32_t* pValue)
{
    if ((m_adqInterface->IsPCIeDevice()) || (m_adqInterface->IsPCIeLiteDevice()))
    {
        *pValue = m_adqInterface->GetPCIeLinkWidth();
    }

    *pTimestamp = m_pcieLinkWidPV.getTimestamp();
}

ADQDevice::~ADQDevice()
{
}
