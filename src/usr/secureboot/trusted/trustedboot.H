/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/secureboot/trustedboot.H $                            */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2015                             */
/* [+] International Business Machines Corp.                              */
/*                                                                        */
/*                                                                        */
/* Licensed under the Apache License, Version 2.0 (the "License");        */
/* you may not use this file except in compliance with the License.       */
/* You may obtain a copy of the License at                                */
/*                                                                        */
/*     http://www.apache.org/licenses/LICENSE-2.0                         */
/*                                                                        */
/* Unless required by applicable law or agreed to in writing, software    */
/* distributed under the License is distributed on an "AS IS" BASIS,      */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or        */
/* implied. See the License for the specific language governing           */
/* permissions and limitations under the License.                         */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */
/**
 * @file trustedboot.H
 *
 * @brief Trustedboot TPM interfaces
 *
 */
#ifndef __TRUSTEDBOOT_H
#define __TRUSTEDBOOT_H
// -----------------------------------------------
// Includes
// -----------------------------------------------

namespace TRUSTEDBOOT
{

enum
{
    MAX_SYSTEM_TPMS = 2,
};


#ifdef CONFIG_TPMDD_1_2
// Command structures taken from TPM Main - Part3 commands v 1.2 rev116

/// Base of all incoming messages
struct TPM_BaseIn {
    uint16_t      tag;       ///< Type TPM_TAG
    uint32_t      paramSize; ///< Total # output bytes incl paramSize and tag
    uint32_t      ordinal;   ///< Type TpmOrdinal : Command ordinal
} PACKED;

/// Base of all outgoing messages
struct TPM_BaseOut {
    uint16_t      tag;       ///< Type TPM_TAG
    uint32_t      paramSize; ///< Total # output bytes incl paramSize and tag
    uint32_t      returnCode;///< The return code of the operation
} PACKED;

/// Generic TPM Input Command structure with a 2 byte parm
struct TPM_2ByteIn {
    TPM_BaseIn  base;
    uint16_t    param;
} PACKED;

/// Generic TPM Output Command structure with a 4 byte return data
struct TPM_4ByteOut {
    TPM_BaseOut base;
    uint32_t    resp;
} PACKED;

/// Incoming TPM_GetCapability structure
struct TPM_GetCapabilityIn {
    TPM_BaseIn  base;
    uint32_t    capArea;    ///< Partition of capabilites to be interrogated
    uint32_t    subCapSize; ///< Size of subCap parameter
    uint8_t     subCap[0];   ///< Further definition of information
} PACKED;

/// Outgoing TPM_GetCapability structure
struct TPM_GetCapabilityOut {
    TPM_BaseOut base;
    uint32_t    respSize;   ///< The length of the returned capability response
    uint8_t     resp[0];     ///< The capability response
} PACKED;

/// Various static values
enum {
    // Command structure tags
    TPM_TAG_RQU_COMMAND       = 0x00C1, ///< A command with no authentication
    TPM_TAG_RQU_AUTH1_COMMAND = 0x00C2, ///< A command with one auth handle
    TPM_TAG_RQU_AUTH2_COMMAND = 0x00C3, ///< A command with two auth handles
    TPM_TAG_RSP_COMMAND       = 0x00C4, ///< A response from cmd with no auth
    TPM_TAG_RSP_AUTH1_COMMAND = 0x00C5, ///< An auth response w/one auth handle
    TPM_TAG_RSP_AUTH2_COMMAND = 0x00C6, ///< An auth response w/two auth handles


    // Ordinals
    TPM_ORD_Extend            = 0x00000014, ///< Extend a PCR valud
    TPM_ORD_PcrRead           = 0x00000015, ///< Read a PCR Value
    TPM_ORD_GetCapability     = 0x00000065, ///< Get Capability command
    TPM_ORD_Init              = 0x00000097, ///< Init the TPM
    TPM_ORD_Startup           = 0x00000099, ///< Initialize the TPM settings

    // Capability areas
    TPM_CAP_PROPERTY          = 0x00000005, ///< Pull TPM properties

    // Capability subcaps
    TPM_CAP_PROP_MANUFACTURER = 0x00000103, ///< returns the vendor ID
    TPM_CAP_PROP_INPUT_BUFFER = 0x00000124, ///< The max size of TPM i/o buf(B)

    // TPM_STARTUP_TYPE's
    TPM_ST_CLEAR              = 0x0001,     ///< Start TPM from a clean state
    TPM_ST_STATE              = 0x0002,     ///< Start TPM from a saved state
    TPM_ST_DEACTIVATED        = 0x0003,     ///< Start TPM in deactivated state


    // TPM Return Codes
    TPM_SUCCESS               = 0x00000000,

};

#endif // CONFIG_TPMDD_1_2


} // end TRUSTEDBOOT namespace
#endif
