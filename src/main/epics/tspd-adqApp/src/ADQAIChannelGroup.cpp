#include <cstdlib>
#include <string>
#include <sstream>
#include <iostream>

#include <ADQAPI.h>
#include <nds3/nds.h>

#include "ADQDevice.h"
#include "ADQAIChannelGroup.h"
#include "ADQAIChannel.h"

ADQAIChannelGroup::ADQAIChannelGroup(const std::string& name, nds::Node& parentNode, ADQInterface *& m_adq_dev) :
    m_node(nds::Port(name, nds::nodeType_t::generic)),
    m_productName(nds::PVVariableIn<std::string>("ProductName")),
    m_serialNumber(nds::PVVariableIn<std::string>("SerialNumber")),
    m_productID(nds::PVVariableIn<std::string>("ProductID")),
    m_adqType(nds::PVVariableIn<std::string>("ADQType")),
    m_cardOption(nds::PVVariableIn<std::string>("CardOption"))

{
    /*
    // Set PVs for device info
    m_productName.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_productName);

    m_serialNumber.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_serialNumber);

    m_productID.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_productID);

    m_adqType.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_adqType);

    m_cardOption.setScanType(nds::scanType_t::interrupt);
    m_node.addChild(m_cardOption);

    // Get device info
    char* adq_pn = m_adq_dev->GetBoardProductName();
    unsigned int adq_pid = m_adq_dev->GetProductID();
    int adq_type = m_adq_dev->GetADQType();
    const char* adq_opt = m_adq_dev->GetCardOption();
    char* adq_sn = m_adq_dev->GetBoardSerialNumber();

    // Send device info to according EPICS records
    m_productName.push(m_productName.getTimestamp(), adq_pn);
    m_serialNumber.push(m_serialNumber.getTimestamp(), adq_sn);
    m_productID.push(m_productID.getTimestamp(), adq_pid);
    m_adqType.push(m_adqType.getTimestamp(), adq_type);
    m_cardOption.push(m_cardOption.getTimestamp(), adq_opt);

    // Device information is returned
    ndsInfoStream(m_node) << "Device started:\nADQ" << adq_type << adq_opt << "\nProduct name: " << adq_pn << \
        "\nSerial number: " << adq_sn << "\nProduct ID: " << adq_pid << std::endl;
*/
}
