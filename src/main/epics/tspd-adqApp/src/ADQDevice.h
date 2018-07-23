#ifndef ADQDEVICE_H
#define ADQDEVICE_H

#include <nds3/nds.h>
#include <mutex>

class ADQDevice
{
public:
    ADQDevice(const std::string& name, nds::Node& parentNode, ADQInterface*& adqInterface);
    virtual ~ADQDevice();

    nds::Port m_node;

    void getProductName(timespec* pTimestamp, std::string* pValue);
    void getSerialNumber(timespec* pTimestamp, std::string* pValue);
    void getProductID(timespec* pTimestamp, int32_t* pValue);
    void getADQType(timespec* pTimestamp, int32_t* pValue);
    void getCardOption(timespec* pTimestamp, std::string* pValue);

    void getTempLocal(timespec* pTimestamp, int32_t* pValue);
    void getTempADCone(timespec* pTimestamp, int32_t* pValue);
    void getTempADCtwo(timespec* pTimestamp, int32_t* pValue);
    void getTempFPGA(timespec* pTimestamp, int32_t* pValue);
    void getTempDd(timespec* pTimestamp, int32_t* pValue);

    void getSampRate(timespec* pTimestamp, double* pValue);
    void getSampRateDec(timespec* pTimestamp, double* pValue);
    void getBytesPerSample(timespec* pTimestamp, int32_t* pValue);

    void getBusAddr(timespec* pTimestamp, int32_t* pValue);
    void getBusType(timespec* pTimestamp, int32_t* pValue);
    void getPCIeLinkRate(timespec* pTimestamp, int32_t* pValue);
    void getPCIeLinkWid(timespec* pTimestamp, int32_t* pValue);

private:
    ADQInterface* m_adqInterface;

    std::string m_productName;
    nds::PVDelegateIn<std::string> m_productNamePV;
    nds::PVDelegateIn<std::string> m_serialNumberPV;
    nds::PVDelegateIn<int32_t> m_productIDPV;
    nds::PVDelegateIn<int32_t> m_adqTypePV;
    nds::PVDelegateIn<std::string> m_cardOptionPV;
    nds::PVDelegateIn<int32_t> m_tempLocalPV;
    nds::PVDelegateIn<int32_t> m_tempAdcOnePV;
    nds::PVDelegateIn<int32_t> m_tempAdcTwoPV;
    nds::PVDelegateIn<int32_t> m_tempFpgaPV;
    nds::PVDelegateIn<int32_t> m_tempDiodPV;
    nds::PVDelegateIn<double> m_sampRatePV;
    //nds::PVDelegateIn<double> m_sampRateDecPV;
    nds::PVDelegateIn<int32_t> m_bytesPerSampPV;
    nds::PVDelegateIn<int32_t> m_busTypePV;
    nds::PVDelegateIn<int32_t> m_busAddrPV;
    nds::PVDelegateIn<int32_t> m_pcieLinkRatePV;
    nds::PVDelegateIn<int32_t> m_pcieLinkWidPV;

protected:
    std::mutex m_adqDevMutex;  // protects adqDev library

    nds::PVDelegateIn<double> m_sampRateDecPV;
};

#endif /* ADQDEVICE_H */
