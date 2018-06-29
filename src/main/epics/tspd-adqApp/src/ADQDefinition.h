#ifndef ADQDEFINITION_H
#define ADQDEFINITION_H

#include <nds3/nds.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

//#define PRINT_RECORD_INFO

#define PINI true
#define CELSIUS_CONVERT 1/256
#define TEMP_LOCAL 0
#define TEMPADC_ONE 1
#define TEMPADC_TWO 2
#define TEMP_FPGA 3
#define TEMP_DIOD 4

#define DATA_MAX_ELEMENTS (4*1024*1024)
#define CHANNEL_NUMBER_MAX 8
#define STRING_ENUM 32
#define ROOT_NODE_DEVICE "ADQ"
#define GROUP_CHAN_DEVICE ":COM"
#define INFO_DEVICE ":INFO"

#define sleep(interval) usleep(1000*interval) // usleep - microsecond interval

#define MIN(a,b) ((a) > (b) ? (b) : (a))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

#define ADQNDS_MSG_ERRLOG(status, text)                               \
    do {                                                              \
        if (!status) {                                                \
            struct timespec now = { 0, 0 };                           \
            clock_gettime(CLOCK_REALTIME, &now);                      \
            m_logMsgPV.push(now, std::string(text));                  \
            ndsErrorStream(m_node) << std::string(text) << std::endl; \
            goto finish;                                              \
        }                                                             \
    } while (0)

#define ADQNDS_MSG_WARNLOG(status, text)                                \
    do {                                                                \
        if (!status) {                                                  \
            struct timespec now = { 0, 0 };                             \
            clock_gettime(CLOCK_REALTIME, &now);                        \
            m_logMsgPV.push(now, std::string(text));                    \
            ndsWarningStream(m_node) << std::string(text) << std::endl; \
        }                                                               \
    } while (0)

#define ADQNDS_MSG_INFOLOG(text)                                 \
    do {                                                         \
        struct timespec now = { 0, 0 };                          \
        clock_gettime(CLOCK_REALTIME, &now);                     \
        m_logMsgPV.push(now, std::string(text));                 \
        ndsInfoStream(m_node) << std::string(text) << std::endl; \
    } while (0)


#endif /* ADQDEFINITION_H */