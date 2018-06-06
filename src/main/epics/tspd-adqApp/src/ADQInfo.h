//// urojec L3: Should be the same as header filename
#ifndef ADQINFO_H
#define ADQINFO_H

#include <nds3/nds.h>

#define CELSIUS_CONVERT 1/256
#define TEMP_LOCAL 0
#define TEMPADC_ONE 1
#define TEMPADC_TWO 2
#define TEMP_FPGA 3
#define TEMP_DIOD 4

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

private:

    ADQInterface * m_adqDevPtr;

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
};

#endif /* ADQINFO_H */
