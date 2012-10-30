/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/diag/prdf/framework/service/iipsdbug.h $              */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2002,2012              */
/*                                                                        */
/* p1                                                                     */
/*                                                                        */
/* Object Code Only (OCO) source materials                                */
/* Licensed Internal Code Source Materials                                */
/* IBM HostBoot Licensed Internal Code                                    */
/*                                                                        */
/* The source code for this program is not published or otherwise         */
/* divested of its trade secrets, irrespective of what has been           */
/* deposited with the U.S. Copyright Office.                              */
/*                                                                        */
/* Origin: 30                                                             */
/*                                                                        */
/* IBM_PROLOG_END_TAG                                                     */

#ifndef IIPSDBUG_H
#define IIPSDBUG_H

/*!
 @file iipsdbug.h
 @brief PRD wrapper to the system debug data
*/

/* Module Description *************************************************/
/*                                                                    */
/*  Name:  iipsdbug.h                                                 */
/*                                                                    */
/*  Description:  This module contains the Processor Runtime
                  Diagnostics System Debug area class declaration.    */
/*                                                                    */
/* End Module Description *********************************************/

/*--------------------------------------------------------------------*/
/* Reference the virtual function tables and inline function
   defintions in another translation unit.                            */
/*--------------------------------------------------------------------*/
#ifdef __GNUC__
 #pragma interface
#endif

/*--------------------------------------------------------------------*/
/*  Includes                                                          */
/*--------------------------------------------------------------------*/

#include <stdint.h>
#include <prdfMain.H>
#include <targeting/common/target.H>

/*--------------------------------------------------------------------*/
/*  Forward References                                                */
/*--------------------------------------------------------------------*/

//class CHIP_CLASS;
struct PRD_SRC_TYPE;
struct STEP_CODE_DATA_STRUCT;

/*--------------------------------------------------------------------*/
/*  User Types                                                        */
/*--------------------------------------------------------------------*/

typedef uint8_t ATTENTION_TYPE;


/*--------------------------------------------------------------------*/
/*  Constants                                                         */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*  Macros                                                            */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*  Global Variables                                                  */
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
/*  Function Prototypes                                               */
/*--------------------------------------------------------------------*/

/* Class Specification ************************************************/
/*                                                                    */
/*  Title:  System Debug                                              */
/*                                                                    */
/*  Purpose:  SYSTEM_DEBUG_CLASS is an interface to the Service
              Processor Communcation Area (SPCA) System Debug common
              memory area.                                            */
/*                                                                    */
/*  Usage:  Concrete class                                            */
/*                                                                    */
/*  Notes:  This System Debug specifies an interface for accessing
            data in the SPCA sysdbug structure.                       */
/*                                                                    */
/*  Cardinality:  N                                                   */
/*                                                                    */
/*  Space Complexity:  Constant                                       */
/*                                                                    */
/* End Class Specification ********************************************/
/**
 Provide services associated with the service processor system debug area
 @author Douglas R. Gilbert
 @version V4R5
*/
class SYSTEM_DEBUG_CLASS
{
public:

  /**
   Constructor
   <ul>
   <br><b>Parameters:  </b> None.
   <br><b>Requirements:</b> sp virtuals established
   <br><b>Promises:    </b> Object instantiated
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
  SYSTEM_DEBUG_CLASS(void);


  /**
   Re-read attention data
   <ul>
   <br><b>Parameters:  </b> i_attnList list of chips at attention
   <br><b>Returns:     </b> None.
   <br><b>Requirements:</b> sp virtuals established
   <br><b>Promises:    </b> object resurrected
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
  uint32_t Reinitialize(const PRDF::AttnList & i_attnList);

  /**
   Indicates if an attention is active for a particular chip
   <ul>
   <br><b>Parameters:  </b> i_pTargetHandle
   <br><b>Returns:     </b> [true | false]
   <br><b>Requirements:</b> None.
   <br><b>Promises:    </b> None.
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
  bool IsAttentionActive(TARGETING::TargetHandle_t i_ptargetHandle ) const;

  /**
   Indicates if an attention is active for a particular chip
   <ul>
   <br><b>Parameters:  </b> ChipClass
   <br><b>Returns:     </b> [true | false]
   <br><b>Requirements:</b> None.
   <br><b>Promises:    </b> None.
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
//  bool IsAttentionActive(const CHIP_CLASS & chip) const;

  /**
   Get the attention type for the attention that is active on this chip
   <ul>
   <br><b>Parameters:  </b> i_pTargetHandle
   <br><b>Returns:     </b> ATTENTION_TYPE
   <br><b>Requirements:</b> IsAttentionActive() == true
   <br><b>Promises:    </b> None.
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
  uint8_t GetAttentionType(TARGETING::TargetHandle_t i_ptargetHandle ) const;

  /**
   Get the attention type for the attention that is active on this chip
   <ul>
   <br><b>Parameters:  </b> ChipClass
   <br><b>Returns:     </b> ATTENTION_TYPE
   <br><b>Requirements:</b> IsAttentionActive() == true
   <br><b>Promises:    </b> None.
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
//  uint8_t GetAttentionType(const CHIP_CLASS & chip) const;

  /**
   Get the global(overall) attention type
   <ul>
   <br><b>Parameters:  </b> None.
   <br><b>Returns:     </b> ATTENTION_TYPE
   <br><b>Requirements:</b> None,
   <br><b>Promises:    </b> None.
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
  uint32_t GetGlobalAttentionType(void) const;


  /**
   Set the sysdebug SRC pointer to the PRD generated SRC
   <ul>
   <br><b>Parameters:  </b> ptr to SRC
   <br><b>Returns:     </b> None
   <br><b>Requirements:</b> src_ptr is valid
   <br><b>Promises:    </b> sysdebug modified
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
  void SetPrdSrcPointer(uint32_t*);
  void SetPrdSrcPointer(); // called by main - can we remove it?


  /**
   Create an SRC in the SOT (obsolete)
   <ul>
   <br><b>Parameters:  </b> reference code, step code, analysis return code
   <br><b>Returns:     </b> return code
   <br><b>Requirements:</b> none.
   <br><b>Promises:    </b> SRC written to SOT
   <br><b>Exceptions:  </b> None.
   <br><b>Notes:       </b> No implementation after V3R7
   </ul><br>
   */
//  int32_t SrcFill(uint16_t ref_code, uint16_t step_code, uint16_t mop_rc) const;

  /**
   Callout all chips at attention
   <ul>
   <br><b>Parameters:  </b> ServiceData
   <br><b>Returns:     </b> None.
   <br><b>Requirements:</b> None.
   <br><b>Promises:    </b> None.
   <br><b>Exceptions:  </b> None.
   </ul><br>
   */
  void CalloutThoseAtAttention(STEP_CODE_DATA_STRUCT & serviceData) const;

  enum { MAX_ERROR_ENTRY_INDEX = 80 };

  /**
   Clear the attentions
   <ul>
   <br><b>Parameters:  </b> None.
   <br><b>Returns:     </b> None.
   <br><b>Requirements:</b> None.
   <br><b>Promises:    </b> Nothing is at attention
   <br><b>Exceptions:  </b> None.
   <br><b>Notes:       </b>
   </ul><br>
   */
  void Clear(void);

  // Functions used by the simulator only
  /**
   Get the pointer to the PRD SRC in sysdebug (Simulator only)
   <ul>
   <br><b>Parameters:  </b> None.
   <br><b>Returns:     </b> ptr to SRC
   <br><b>Requirements:</b> SetPrdSrcPointer()
   <br><b>Promises:    </b> None.
   <br><b>Exceptions:  </b> None.
   <br><b>Notes:       </b> Available in PRD simulator only
   </ul><br>
   */
  const uint32_t *GetPrdSrcPointer(void) const;

  /**
   Set the attention type for the specified chip (Simulator only)
   <ul>
   <br><b>Parameters:  </b> i_pTargetHandle, ATTENTION_TYPE
   <br><b>Returns:     </b> None.
   <br><b>Requirements:</b> None.
   <br><b>Promises:    </b> GetAttentionType() == at
   <br><b>Exceptions:  </b> None.
   <br><b>Notes:       </b> Available in PRD simulator only
   </ul><br>
   */
  void SetAttentionType(TARGETING::TargetHandle_t i_pTargetHandle ,PRDF::ATTENTION_VALUE_TYPE i_eAttentionType);

  // SetGlobalAttentionType(uint8_t ga); // FIXME : remove this function when merge, no longer used

private:

  enum
  {
    SRCFILL_FORMAT = 1,
    SRCFILL_GROUP = 7
  };
};
#endif //IIPSDBUG_H

// Change Log *************************************************************************************
//
//  Flag Reason  Vers  Date     Coder    Description
//  ---- ------- ----- -------- -------- ----------------------------------------------------------
//  jl02 605874 fips330 07/31/07 lukas    Add functions to PRD framework/Galaxy 2 code for unit CS
// End Change Log *********************************************************************************