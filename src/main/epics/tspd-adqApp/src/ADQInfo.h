#ifndef ADQINFO_H
#define ADQINFO_H

#include <nds3/nds.h>

class ADQInfo
{
public:
    ADQInfo(const std::string& name, nds::Node& parentNode, ADQInterface *& adqDev);

    nds::Port m_node;

    void getProductName(timespec* pTimestamp, std::string* pValue);
    void getSerialNumber(timespec* pTimestamp, std::string* pValue);
    void getProductID(timespec* pTimestamp, std::int32_t* pValue);
    void getADQType(timespec* pTimestamp, std::int32_t* pValue);
    void getCardOption(timespec* pTimestamp, std::string* pValue);
    void getTempLocal(timespec* pTimestamp, std::int32_t* pValue);
    void getTempADCone(timespec* pTimestamp, std::int32_t* pValue);
    void getTempADCtwo(timespec* pTimestamp, std::int32_t* pValue);
    void getTempFPGA(timespec* pTimestamp, std::int32_t* pValue);
    void getTempDd(timespec* pTimestamp, std::int32_t* pValue);
    void getSampRate(timespec* pTimestamp, double* pValue);
    void getBusAddr(timespec* pTimestamp, std::int32_t* pValue);
    void getBusType(timespec* pTimestamp, std::int32_t* pValue);
    void getPCIeLinkRate(timespec* pTimestamp, std::int32_t* pValue);
    void getPCIeLinkWid(timespec* pTimestamp, std::int32_t* pValue);

private:

    ADQInterface * m_adqDevPtr;

    std::string m_productName;

    nds::PVDelegateIn<std::string> m_productNamePV;
    nds::PVDelegateIn<std::string> m_serialNumberPV;
    nds::PVDelegateIn<std::int32_t> m_productIDPV;
    nds::PVDelegateIn<std::int32_t> m_adqTypePV;
    nds::PVDelegateIn<std::string> m_cardOptionPV;
    nds::PVDelegateIn<std::int32_t> m_tempLocalPV;
    nds::PVDelegateIn<std::int32_t> m_tempAdcOnePV;
    nds::PVDelegateIn<std::int32_t> m_tempAdcTwoPV;
    nds::PVDelegateIn<std::int32_t> m_tempFpgaPV;
    nds::PVDelegateIn<std::int32_t> m_tempDiodPV;
    nds::PVDelegateIn<double> m_sampRatePV;
    nds::PVDelegateIn<std::int32_t> m_busAddrPV;
    nds::PVDelegateIn<std::int32_t> m_busTypePV;
    nds::PVDelegateIn<std::int32_t> m_pcieLinkRatePV;
    nds::PVDelegateIn<std::int32_t> m_pcieLinkWidPV;
};

#endif /* ADQINFO_H */
