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
    m_adqDevPtr(adq_dev),
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
    m_tempLocalPV(nds::PVDelegateIn<std::int32_t>("TemperatureLocal", std::bind(&ADQInfo::getTempPV,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempAdcOnePV(nds::PVDelegateIn<std::int32_t>("TemperatureADC-1", std::bind(&ADQInfo::getTempPV,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempAdcTwoPV(nds::PVDelegateIn<std::int32_t>("TemperatureADC-2", std::bind(&ADQInfo::getTempPV,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempFpgaPV(nds::PVDelegateIn<std::int32_t>("TemperatureFPGA", std::bind(&ADQInfo::getTempPV,
                                                                        this,
                                                                        std::placeholders::_1,
                                                                        std::placeholders::_2))),
    m_tempDiodPV(nds::PVDelegateIn<std::int32_t>("TemperatureDiode", std::bind(&ADQInfo::getTempPV,
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
    m_tempLocalPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempLocalPV);

    m_tempAdcOnePV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempAdcOnePV);

    m_tempAdcTwoPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempAdcTwoPV);

    m_tempFpgaPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempFpgaPV);

    m_tempDiodPV.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_tempDiodPV);

    // Ask for temperature values; probably need to set a refreshing period for this method (periodic processing)
    // or it could be a separate thread like DAQ
    getTemp();
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
    *pValue = m_adqDevPtr->GetADQType();;
}

void ADQInfo::getCardOption(timespec* pTimestamp, std::string* pValue)
{
    *pValue = m_adqDevPtr->GetCardOption();
}



//// urojec L3: Hardocoding, please define macros or static const or enum for all this
//// ppipp: Need to try with only one method for all temperatures (getTempPV and getTemp methods)

void ADQInfo::getTempPV(timespec* pTimestamp, std::int32_t* pValue)
{
    /* Dummy method for pushing different temperatures to different PVs
     */
}

void ADQInfo::getTemp()
{
    struct timespec now = { 0, 0 };
    clock_gettime(CLOCK_REALTIME, &now);

    std::int32_t tempVal;
    std::list<nds::PVDelegateIn<std::int32_t>> tempPvList = { m_tempLocalPV, m_tempAdcOnePV, m_tempAdcTwoPV, m_tempFpgaPV, m_tempDiodPV };
    int i = 0;

    for (nds::PVDelegateIn<std::int32_t> tempPV : tempPvList)
    {
        tempVal = m_adqDevPtr->GetTemperature(i)*CELSIUS_CONVERT;
        // I think tempPV should turn into needed PV name -- needs testing
        // After getting a needed value of temperature, it should be pushed to correct PV
        tempPV.push(now, tempVal);
        ++i;
    }
}

/*
void ADQInfo::getTempLocal(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(0) / 256;
}

void ADQInfo::getTempADCone(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(1) / 256;
}

void ADQInfo::getTempADCtwo(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(2) / 256;
}

void ADQInfo::getTempFPGA(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(3) / 256;
}

void ADQInfo::getTempDd(timespec* pTimestamp, std::int32_t* pValue)
{
    *pValue = m_adqDevPtr->GetTemperature(4) / 256;
}
*/