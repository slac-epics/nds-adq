//
// Copyright (c) 2018 Cosylab d.d.
// This software is distributed under the terms found
// in file LICENSE.txt that is included with this distribution.
//

#ifndef ADQDEFINITION_H
#define ADQDEFINITION_H

#include <iostream>
#include <nds3/nds.h>
#include <sstream>
#include <unistd.h>

/** @file ADQDefinition.h
 * @brief This file contains global objects (constants and macros).
 * They are used in classes that include this file.
 */

/** @def PINI
 * @brief Enable PVs to process at the device initialization.
 */
#define PINI true

/** @def CELSIUS_CONVERT
 * @brief Convert received temperature value into Celsius.
 */
#define CELSIUS_CONVERT 1 / 256

 /** @def TEMP_LOCAL
 * @brief Digitizer's device address of PCB.
 */
#define TEMP_LOCAL 0

 /** @def TEMP_ADC_ONE
 * @brief Digitizer's device address of ADC1.
 */
#define TEMP_ADC_ONE 1

 /** @def TEMP_ADC_TWO
 * @brief Digitizer's device address of ADC2.
 */
#define TEMP_ADC_TWO 2

 /** @def TEMP_FPGA
 * @brief Digitizer's device address of FPGA.
 */
#define TEMP_FPGA 3

 /** @def TEMP_DIOD
 * @brief Digitizer's device address of DCDC2A.
 */
#define TEMP_DIOD 4

/** @def DATA_MAX_ELEMENTS
 * @brief Maximum number of elements for data PV.
 */
#define DATA_MAX_ELEMENTS (4 * 1024 * 1024)

 /** @def BUFFERSIZE_ADQ14
 * @brief Buffersize for data acquisition (ADQ14).
 */
#define BUFFERSIZE_ADQ14 (512 * 1024)

 /** @def BUFFERSIZE_ADQ7
 * @brief Buffersize for data acquisition (ADQ7).
 */
#define BUFFERSIZE_ADQ7 (256 * 1024)

/** @def CHANNEL_COUNT_MAX
 * @brief Maximum allowed amount of channels.
 */
#define CHANNEL_COUNT_MAX 8

/** @def EXTERN_TRIG_COUNT
 * @brief Amount of inputs for external triggering in each device.
 */
#define EXTERN_TRIG_COUNT 1

/** @def STRING_ENUM
 * @brief Number of elements for some digitizer's information PVs.
 */
#define STRING_ENUM 32

/** @def GROUP_CHAN_DEVICE
 * @brief Append the string to ADQAIChannelGroup node name.
 */
#define GROUP_CHAN_DEVICE "-ChGrp"

/** @def INFO_DEVICE
 * @brief Append the string to ADQDevice node name.
 */
#define INFO_DEVICE "-Info"

/** @def SLEEP(interval)
 * @brief Macro for sleeping for \a 1000*interval microseconds.
 * @param interval value that will be multiplied by 1000 microseconds.
 */
#define SLEEP(interval) usleep(1000 * interval)

/** @def MIN(a, b)
 * @brief A macro that returns the minumim of \a a and \a b.
 */
#define MIN(a, b) ((a) > (b) ? (b) : (a))

/** @def UNUSED
 * @brief Macro for busying unused parameters in methods.
 */
#define UNUSED(x) (void)x

/** @def ADQNDS_MSG_INFOLOG_PV(text)
 * @brief Macro for pushing log messages to PV. Used in ADQAIChannelGroup methods.
 * @param text input information message.
 */
#define ADQNDS_MSG_INFOLOG_PV(text)                              \
    do                                                           \
    {                                                            \
        struct timespec now = { 0, 0 };                          \
        clock_gettime(CLOCK_REALTIME, &now);                     \
        m_logMsgPV.push(now, std::string(text));                 \
        ndsInfoStream(m_node) << std::string(text) << std::endl; \
    } while (0)

/** @def ADQNDS_MSG_ERRLOG_PV
 * @brief Macro for informing the user about occured major failures and
 * stopping data acquisition. Used in ADQAIChannelGroup methods.
 * @param status status of the function that calls this macro.
 * @param text input information message.
 */
#define ADQNDS_MSG_ERRLOG_PV(status, text)                            \
    do                                                                \
    {                                                                 \
        if (!status)                                                  \
        {                                                             \
            struct timespec now = { 0, 0 };                           \
            clock_gettime(CLOCK_REALTIME, &now);                      \
            m_logMsgPV.push(now, std::string(text));                  \
            ndsErrorStream(m_node) << std::string(text) << std::endl; \
            goto finish;                                              \
        }                                                             \
    } while (0)

/** @def ADQNDS_MSG_WARNLOG_PV
 * @brief Macro for warning information in case of minor failures.
 * Used in ADQAIChannelGroup methods.
 * @param status status of the function that calls this macro.
 * @param text input information message.
 */
#define ADQNDS_MSG_WARNLOG_PV(status, text)                             \
    do                                                                  \
    {                                                                   \
        if (!status)                                                    \
        {                                                               \
            struct timespec now = { 0, 0 };                             \
            clock_gettime(CLOCK_REALTIME, &now);                        \
            m_logMsgPV.push(now, std::string(text));                    \
            ndsWarningStream(m_node) << std::string(text) << std::endl; \
        }                                                               \
    } while (0)

#endif /* ADQDEFINITION_H */
