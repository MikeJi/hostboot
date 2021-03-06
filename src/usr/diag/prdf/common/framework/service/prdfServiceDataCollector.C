/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/common/framework/service/prdfServiceDataCollector.C $ */
/*                                                                        */
/* OpenPOWER HostBoot Project                                             */
/*                                                                        */
/* Contributors Listed Below - COPYRIGHT 2012,2015                        */
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
    @file prdfServiceDataCollector.C
    @brief ServiceDataCollector definition
*/
//------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------
#include <string.h>                // for memcpy
#define prdfServiceDataCollector_C
#include <iipServiceDataCollector.h>
#include <prdfPlatServices.H>
#include <prdfTrace.H>
#undef prdfServiceDataCollector_C

using namespace TARGETING;

//------------------------------------------------------------------------------
//  User Types, Constants, macros, prototypes, globals
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Member Function Specifications
//------------------------------------------------------------------------------

namespace PRDF
{

   std::list<ExtensibleChip*> ServiceDataCollector::cv_ruleChipStack;

#ifndef __HOSTBOOT_MODULE

inline void buffer_append(uint8_t *&ptr, uint32_t value)
{
    uint32_t l_tmp32 = htonl(value);
    memcpy(ptr, &l_tmp32, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
}

inline void buffer_append(uint8_t *&ptr, uint16_t value)
{
    uint16_t l_tmp16 = htons(value);
    memcpy(ptr, &l_tmp16, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
}

inline void buffer_append(uint8_t *&ptr, uint8_t value)
{
    memcpy(ptr, &value, sizeof(value));
    ptr += sizeof(uint8_t);
}

inline uint32_t buffer_get32(const uint8_t * &ptr)
{
    uint32_t l_tmp32 = 0;
    memcpy(&l_tmp32, ptr, sizeof(uint32_t));
    ptr += sizeof(uint32_t);
    return ntohl(l_tmp32);
}

inline uint16_t buffer_get16(const uint8_t * &ptr)
{
    uint16_t l_tmp16 = 0;
    memcpy(&l_tmp16, ptr, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    return ntohs(l_tmp16);
}

inline uint8_t buffer_get8(const uint8_t * &ptr)
{
    uint8_t l_tmp8 = 0;
    memcpy(&l_tmp8, ptr, sizeof(uint8_t));
    ptr += sizeof(uint8_t);
    return l_tmp8;
}

inline void buffer_append( uint8_t *&ptr, const TARGETING::TargetHandle_t i_pGivenHandle )
{
    HUID l_targetHuid = PlatServices::getHuid( i_pGivenHandle );
    l_targetHuid = htonl( l_targetHuid );
    uint32_t l_size = sizeof( l_targetHuid );
    memcpy( ptr, &l_targetHuid, l_size );
    ptr += l_size;
}

inline TARGETING::TargetHandle_t buffer_getTarget( const uint8_t *&ptr )
{
    HUID l_chipHuid = INVALID_HUID;
    uint32_t l_size = sizeof( l_chipHuid );
    memcpy( &l_chipHuid, ptr, l_size );
    l_chipHuid = ntohl( l_chipHuid );
    TARGETING::TargetHandle_t l_tempChipHandle = PlatServices::getTarget( l_chipHuid );
    ptr += l_size;

    return l_tempChipHandle;
}

#endif // ifndef __HOSTBOOT_MODULE

//------------------------------------------------------------------------------

void ServiceDataCollector::SetCallout( PRDcallout mru,
                                       PRDpriority priority,
                                       GARD_POLICY i_gardState )
{
    bool found = false;

    if ( PRDcalloutData::TYPE_TARGET == mru.getType() )
    {
        // Ensuring target is not NULL
        if ( NULL == mru.getTarget() )
        {
            PRDF_ERR( "[ServiceDataCollector::SetCallout] "
                      "skipping NULL callout" );
            return;
        }
    }

    for ( SDC_MRU_LIST::iterator i = xMruList.begin();
          i != xMruList.end() && found == false; ++i )
    {
        if ( i->callout == mru )
        {
            found = true;
            if ( priority > i->priority )
            {
                i->priority = priority;
            }

            if( i_gardState >  i->gardState )
            {
                i->gardState = i_gardState;
            }
        }
    }

    if ( found == false )
    {
        xMruList.push_back( SdcCallout(mru, priority, i_gardState) );
    }
}

//------------------------------------------------------------------------------

void ServiceDataCollector::Gard( GardAction::ErrorType i_et )
{
    GARD_POLICY gardPolicy = GARD;

    if( GardAction::NoGard == i_et )
    {
        gardPolicy = NO_GARD;
    }

    for ( SDC_MRU_LIST::iterator i = xMruList.begin();
          i != xMruList.end(); ++i )
    {
        i->gardState = gardPolicy;
    }
}

//------------------------------------------------------------------------------

bool ServiceDataCollector::isGardRequested()
{
    bool gardRecordExit = false;

    for ( SDC_MRU_LIST::iterator i = xMruList.begin();
          i != xMruList.end(); ++i )
    {
        if( GARD == i->gardState )
        {
            gardRecordExit = true;
            break;
        }
    }

    return gardRecordExit;
}

//------------------------------------------------------------------------------

void ServiceDataCollector::AddSignatureList( TargetHandle_t i_target,
                                             uint32_t i_signature )
{
    #define PRDF_FUNC "[ServiceDataCollector::AddSignatureList] "

    do
    {
        if ( NULL == i_target )
        {
            PRDF_ERR( PRDF_FUNC "Given target is NULL" );
            break;
        }

        bool found = false;
        for ( PRDF_SIGNATURES::iterator i = iv_SignatureList.begin();
              i != iv_SignatureList.end(); i++ )
        {
            if ( (i->target == i_target) && (i->signature == i_signature) )
            {
                found = true;
                break;
            }
        }

        if ( !found )
        {
            iv_SignatureList.push_back( SignatureList(i_target, i_signature) );
        }

    } while (0);

    #undef PRDF_FUNC
}

//------------------------------------------------------------------------------

void ServiceDataCollector::AddSignatureList( ErrorSignature & i_sig )
{
    #define PRDF_FUNC "[ServiceDataCollector::AddSignatureList] "

     TARGETING::TargetHandle_t tgt = PlatServices::getTarget(
                                                  i_sig.getChipId());

     if ( NULL != tgt )
     {
        AddSignatureList( tgt, i_sig.getSigId() );
     }
     else
     {
        PRDF_ERR( PRDF_FUNC "Failed to get target Handle for "
                          "chip:0x%08X", i_sig.getChipId() );
     }
    #undef PRDF_FUNC
}
//------------------------------------------------------------------------------

TARGETING::TargetHandle_t ServiceDataCollector::getTargetAnalyzed( )
{
    ExtensibleChip * l_pChipAnalyzed = getChipAnalyzed();
    TARGETING::TargetHandle_t l_pTargetAnalyzed = NULL;
    if( NULL != l_pChipAnalyzed )
    {
         l_pTargetAnalyzed = l_pChipAnalyzed->GetChipHandle( );
    }
    return  l_pTargetAnalyzed ;
}
//------------------------------------------------------------------------------

#ifndef __HOSTBOOT_MODULE

uint32_t ServiceDataCollector::Flatten(uint8_t * i_buffer, uint32_t & io_size) const
{
    uint32_t max_size = io_size;
    uint32_t rc = SUCCESS;
    //getting the actual size of SignatureList that gets saved in memory.
    uint32_t l_sizeSignList   = iv_SignatureList.size()  * sizeof(SignatureList);
    uint32_t l_sizeMruList    = xMruList.size()          * sizeof(SdcCallout);
    // approximate space needed for essentials.
    // This estimate is slightly higher than actual
    const uint32_t MIN_FLAT_SIZE = sizeof(ServiceDataCollector) +
                                   sizeof(struct Timer::prdftm_t) +
                                   l_sizeMruList + l_sizeSignList;

    uint8_t * current_ptr = i_buffer;

    if(max_size > MIN_FLAT_SIZE)
    {
        // must have this
        uint32_t l_huid = error_signature.getChipId();
        buffer_append(current_ptr,l_huid);
        buffer_append(current_ptr,error_signature.getSigId());
        // callouts
        buffer_append(current_ptr,xMruList.size());
        for ( SDC_MRU_LIST::const_iterator i = xMruList.begin();
              i != xMruList.end(); ++i )
        {
            buffer_append( current_ptr, (uint32_t)i->callout.getType() );
            buffer_append( current_ptr, i->callout.flatten()           );
            buffer_append( current_ptr, (uint32_t)i->priority          );
            buffer_append( current_ptr, (uint32_t)i->gardState         );
        }

        buffer_append(current_ptr, iv_SignatureList.size());
        for(PRDF_SIGNATURES::const_iterator i = iv_SignatureList.begin();
            i != iv_SignatureList.end(); ++i)
        {
            buffer_append( current_ptr, i->target );
            buffer_append( current_ptr, i->signature );
        }
        buffer_append(current_ptr,maskId);
        buffer_append(current_ptr,(uint32_t)attentionType);
        buffer_append(current_ptr,flags);
        buffer_append(current_ptr,hitCount);
        buffer_append(current_ptr,threshold);
        buffer_append(current_ptr,startingPoint);
        buffer_append(current_ptr,(uint32_t)errorType);
        //@ecdf - Removed ivDumpRequestType.
        buffer_append(current_ptr,ivDumpRequestContent);
        buffer_append(current_ptr,ivpDumpRequestChipHandle);
        Timer::prdftm_t l_tm = ivCurrentEventTime.gettm();
        const uint32_t PRDFTM_SIZE = sizeof(struct Timer::prdftm_t);
        memcpy(current_ptr,&l_tm,PRDFTM_SIZE);
        current_ptr += PRDFTM_SIZE;
        buffer_append(current_ptr,(uint32_t)causeAttentionType);

        // Add as much capture data as we have room.
        uint32_t cap_size = 0;
        uint8_t * cap_size_ptr = current_ptr;  // Place for Capture data size
        current_ptr += sizeof(cap_size);

        cap_size = captureData.Copy(
                        current_ptr,max_size - (current_ptr - i_buffer));
        current_ptr += cap_size;
        buffer_append(cap_size_ptr,cap_size);

        // Add as much TraceArray capture data as we have room.
        cap_size_ptr = current_ptr;  // Place for Capture data size
        current_ptr += sizeof(cap_size);

        cap_size = iv_traceArrayData.Copy(
                        current_ptr,max_size - (current_ptr - i_buffer));
        current_ptr += cap_size;
        buffer_append(cap_size_ptr,cap_size);

    }
    else // buffer is not big enough to capture the essentials
    {
        rc = 2;
    }

    io_size = current_ptr - i_buffer;

    return rc;
}

//------------------------------------------------------------------------------

ServiceDataCollector & ServiceDataCollector::operator=(
                                                    const uint8_t * i_flatdata )
{
    using namespace PRDcalloutData;

    error_signature.setChipId( buffer_get32(i_flatdata) );
    error_signature.setSigId(  buffer_get32(i_flatdata) );

    ClearCallouts();
    uint32_t value = buffer_get32(i_flatdata); // number of callouts
    for ( uint32_t i = 0; i < value; ++i )
    {
        MruType mt           = (MruType) buffer_get32(i_flatdata);
        uint32_t mru         = buffer_get32(i_flatdata);
        PRDpriority priority = (PRDpriority) buffer_get32(i_flatdata);
        GARD_POLICY gardState   = (GARD_POLICY) buffer_get32(i_flatdata);

        PRDcallout callout( mru, mt );
        xMruList.push_back( SdcCallout(callout, priority, gardState) );
    }

    ClearSignatureList();
    value = buffer_get32(i_flatdata);  // number of signatures
    for(uint32_t i = 0; i < value; ++i)
    {
        TARGETING::TargetHandle_t l_pChipHandle = buffer_getTarget(i_flatdata);
        uint32_t             l_signature   = (uint32_t)  buffer_get32(i_flatdata);
        if(NULL !=l_pChipHandle)
        {
            SignatureList    l_item(l_pChipHandle, l_signature);
            iv_SignatureList.push_back(l_item);
        }
    }

    maskId = buffer_get32(i_flatdata);
    attentionType = (ATTENTION_TYPE)buffer_get32(i_flatdata);
    flags = buffer_get32(i_flatdata);  //mp02 c from buffer_get16
    hitCount = buffer_get8(i_flatdata);
    threshold = buffer_get8(i_flatdata);
    startingPoint = buffer_getTarget(i_flatdata);
    errorType = (GardAction::ErrorType)buffer_get32(i_flatdata);
    ivDumpRequestContent = (hwTableContent) buffer_get32(i_flatdata); //@ecdf
    ivpDumpRequestChipHandle = buffer_getTarget(i_flatdata);

    Timer::prdftm_t l_tm;
    const uint32_t PRDFTM_SIZE = sizeof(struct Timer::prdftm_t);
    memcpy(&l_tm,i_flatdata,PRDFTM_SIZE);
    i_flatdata += PRDFTM_SIZE;
    ivCurrentEventTime.settm(l_tm);

    causeAttentionType = (ATTENTION_TYPE) buffer_get32(i_flatdata);

    // Capture data - oh joy
    // do we re-expand the data or change capture date
    // to hang onto the already flattened data?
    // lets give it back to the capture data object and let it decide.
    captureData = i_flatdata;

    // CaptureData::operator= will take care of unflattening the data
    iv_traceArrayData = i_flatdata;

    return *this;
}

#endif // #ifndef __HOSTBOOT_MODULE

} // end namespace PRDF


