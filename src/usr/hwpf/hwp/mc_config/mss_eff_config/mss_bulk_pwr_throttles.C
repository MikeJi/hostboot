/* IBM_PROLOG_BEGIN_TAG                                                   */
/* This is an automatically generated prolog.                             */
/*                                                                        */
/* $Source: src/usr/hwpf/hwp/mc_config/mss_eff_config/mss_bulk_pwr_throttles.C $ */
/*                                                                        */
/* IBM CONFIDENTIAL                                                       */
/*                                                                        */
/* COPYRIGHT International Business Machines Corp. 2012                   */
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
// $Id: mss_bulk_pwr_throttles.C,v 1.10 2012/11/13 16:45:28 bellows Exp $
// $Source: /afs/awd/projects/eclipz/KnowledgeBase/.cvsroot/eclipz/chips/centaur/working/procedures/ipl/fapi/mss_bulk_pwr_throttles.C,v $
//------------------------------------------------------------------------------
// *! (C) Copyright International Business Machines Corp. 2011
// *! All Rights Reserved -- Property of IBM
// *! *** IBM Confidential ***
//------------------------------------------------------------------------------
// *! TITLE       : mss_bulk_pwr_throttles
// *! DESCRIPTION : see additional comments below
// *! OWNER NAME  : Joab Henderson    Email: joabhend@us.ibm.com
// *! BACKUP NAME : Michael Pardeik   Email: pardeik@us.ibm.com
// *! ADDITIONAL COMMENTS :
//
// applicable CQ component memory_screen
//
// DESCRIPTION:
// The purpose of this procedure is to set the throttle attributes based on a power limit for the dimms on the channel pair
// At the end, output attributes will be updated with throttle values that will have dimms at or below the limit
// NOTE:  ISDIMMs and CDIMMs are handled differently
//   ISDIMMs use a power per DIMM for the thermal power limit from the MRW
//   CDIMM will use power per CDIMM (power for all virtual dimms) for the thermal power limit from the MRW
// Plan is to have ISDIMM use the per-slot throttles (thermal throttles) or per-mba throttles (power throttles), and CDIMM to use the per-chip throttles
// Note that throttle_n_per_mba takes on different meanings depending on how cfg_nm_per_slot_enabled is set
//   Can be slot0/slot1 OR slot0/MBA throttling
// Note that  throttle_n_per_chip takes on different meaning depending on how cfg_count_other_mba_dis is set
//  Can be per-chip OR per-mba throttling
// ISDIMM:  These registers need to be setup to these values, will be able to do per slot or per MBA throttling
//   cfg_nm_per_slot_enabled = 1
//   cfg_count_other_mba_dis = 1
// CDIMM:  These registers need to be setup to these values, will be able to do per slot or per chip throttling
//   cfg_nm_per_slot_enabled = 1
//   cfg_count_other_mba_dis = 0
//
//
//------------------------------------------------------------------------------
// Don't forget to create CVS comments when you check in your changes!
//------------------------------------------------------------------------------
// CHANGE HISTORY:
//------------------------------------------------------------------------------
// Version:|  Author: |  Date:  | Comment:
//---------|----------|---------|-----------------------------------------------
//   1.10   | pardeik  |08-NOV-12| attribute name update for runtime per chip throttles
//   1.9   | pardeik  |25-OCT-12| updated FAPI_ERR sections, use per_chip variables (in if statements) in the throttle update section when channel pair power is greater than the limit, added CQ component comment line
//   1.8   | pardeik  |19-OCT-12| Changed throttle_n_per_chip to be based on num_mba_with_dimms
//         | pardeik  |19-OCT-12| Updated default throttle values to represent cmd bus utilization instead of dram bus utilization
//         | pardeik  |19-OCT-12| multiple throttle N values by 4 to get dram utilization
//   1.7   | pardeik  |10-OCT-12| Changed throttle attributes and call new function (mss_throttle_to_power) to calculate the power
//   1.6   | pardeik  |10-APR-12| power calculation fixes and updates
//   1.5   | pardeik  |04-APR-12| moved cdimm power calculation to end of section instead of having it in multiple places
//   1.4   | pardeik  |04-APR-12| do channel throttle denominator check as zero only if there are ranks present
//         | pardeik  |04-APR-12| use else if instead of if after checking throttle denominator to zero
//   1.3   | pardeik  |03-APR-12| added cdimm power calculation for half of cdimm, changed i_target from mbs to mba
//   1.2   | pardeik  |03-APR-12| call mss_eff_config_thermal directly
//   1.1   | pardeik  |28-MAR-12| Updated to use Attributes
//         | pardeik  |11-NOV-11| First Draft.


//------------------------------------------------------------------------------
//  My Includes
//------------------------------------------------------------------------------
#include <mss_bulk_pwr_throttles.H>
#include <mss_throttle_to_power.H>

//------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------
#include <fapi.H>


extern "C" {

    using namespace fapi;


//------------------------------------------------------------------------------
// Funtions in this file
//------------------------------------------------------------------------------
    fapi::ReturnCode mss_bulk_pwr_throttles(const fapi::Target & i_target_mba);


//------------------------------------------------------------------------------
// @brief mss_bulk_pwr_throttles(): This function determines the throttle values from a MBA channel pair power limit
//
// @param    const fapi::Target & i_target_mba:  MBA Target passed in
//
// @return fapi::ReturnCode
//------------------------------------------------------------------------------

    fapi::ReturnCode mss_bulk_pwr_throttles(const fapi::Target & i_target_mba)
    {
	fapi::ReturnCode rc;

	char procedure_name[32];
	sprintf(procedure_name, "mss_bulk_pwr_throttles");
	FAPI_INF("*** Running %s ***", procedure_name);

	enum
	{
	    CDIMM  = fapi::ENUM_ATTR_EFF_DIMM_TYPE_CDIMM,
	    RDIMM  = fapi::ENUM_ATTR_EFF_DIMM_TYPE_RDIMM,
	    UDIMM  = fapi::ENUM_ATTR_EFF_DIMM_TYPE_UDIMM,
	    LRDIMM = fapi::ENUM_ATTR_EFF_DIMM_TYPE_LRDIMM,
	};

// other variables used in this procedure
	const uint8_t MAX_NUM_PORTS = 2;			// number of ports per MBA
	const uint8_t MAX_NUM_DIMMS = 2;			// number of dimms per MBA port
	const float MIN_UTIL = 1;				// Minimum percent data bus utilization (percent of max) allowed (for floor)
// If this is changed, also change mss_throttle_to_power MAX_UTIL
	const float MAX_UTIL = 75;				// Maximum theoretical data bus utilization (percent of max) (for ceiling)
	const uint32_t MEM_THROTTLE_D_DEFAULT = 512;		// default throttle denominator (unthrottled) for cfg_nm_m
	const uint32_t MEM_THROTTLE_N_DEFAULT_PER_MBA = (int)(MEM_THROTTLE_D_DEFAULT * (MAX_UTIL / 100) / 4);		// default throttle numerator (unthrottled) for cfg_nm_n_per_mba
	const uint32_t MEM_THROTTLE_N_DEFAULT_PER_CHIP = (int)(MEM_THROTTLE_D_DEFAULT * (MAX_UTIL / 100) / 4);	// default throttle numerator (unthrottled) for cfg_nm_n_per_chip
	fapi::Target target_chip;
	std::vector<fapi::Target> target_mba_array;
	std::vector<fapi::Target> target_dimm_array;
	uint32_t channel_pair_watt_target;
	uint32_t throttle_n_per_mba;
	uint32_t throttle_n_per_chip;
	uint32_t throttle_d;
	uint8_t port;
	uint8_t dimm;
	bool not_enough_available_power;
	bool channel_pair_throttle_done;
	float channel_pair_power;
	uint8_t dimm_type;
	uint8_t num_mba_with_dimms;
	uint32_t power_int_array[MAX_NUM_PORTS][MAX_NUM_DIMMS];
	bool thermal_throttle_active;
	uint8_t mba_index;

// Get input attributes
	rc = FAPI_ATTR_GET(ATTR_EFF_DIMM_TYPE, &i_target_mba, dimm_type);
	if(rc) return rc;
	rc = FAPI_ATTR_GET(ATTR_MSS_MEM_WATT_TARGET, &i_target_mba, channel_pair_watt_target);
	if(rc) return rc;
	rc = FAPI_ATTR_GET(ATTR_MSS_POWER_INT, &i_target_mba, power_int_array);
	if(rc) return rc;
// runtime throttles will be the thermal throttle values (or zero if not initialized yet)
	rc = FAPI_ATTR_GET(ATTR_MSS_RUNTIME_MEM_THROTTLE_NUMERATOR_PER_MBA, &i_target_mba, throttle_n_per_mba);
	if(rc) return rc;
	rc = FAPI_ATTR_GET(ATTR_MSS_RUNTIME_MEM_THROTTLE_NUMERATOR_PER_CHIP, &i_target_mba, throttle_n_per_chip);
	if(rc) return rc;
	rc = FAPI_ATTR_GET(ATTR_MSS_RUNTIME_MEM_THROTTLE_DENOMINATOR, &i_target_mba, throttle_d);
	if(rc) return rc;

// get number of mba's with dimms for a CDIMM
	if (dimm_type == CDIMM)
	{
// Get Centaur target for the given MBA
	    rc = fapiGetParentChip(i_target_mba, target_chip);
	    if(rc) return rc;
// Get MBA targets from the parent chip centaur
	    rc = fapiGetChildChiplets(target_chip, fapi::TARGET_TYPE_MBA_CHIPLET, target_mba_array, fapi::TARGET_STATE_PRESENT);
	    if(rc) return rc;
	    num_mba_with_dimms = 0;
	    for (mba_index=0; mba_index < target_mba_array.size(); mba_index++)
	    {
		rc = fapiGetAssociatedDimms(target_mba_array[mba_index], target_dimm_array, fapi::TARGET_STATE_PRESENT);
		if(rc) return rc;
		if (target_dimm_array.size() > 0)
		{
		    num_mba_with_dimms++;
		}
	    }

	}
	else
	{
	    // ISDIMMs, set to a value of one since they are handled on a per MBA basis
	    num_mba_with_dimms = 1;
	}

///////////////////////////////
// THROTTLE SECTION
///////////////////////////////

// Determine if the channel pair power for this MBA is over the limit when the runtime memory throttle settings are used
// If not over the limit, then use the runtime throttle settings (defined in mss_eff_config_thermal)
// If over limit, then increase throttle value until it is at or below limit
// If unable to get power below limit, then call out an error

// Determine whether to base throttles on thermal or power reasons (power throttles can give you better performance than thermal throttles)
	if ((throttle_n_per_mba == 0) && (throttle_n_per_chip == 0) && (throttle_d == 0))
	{
    // runtime throttles are all zero here, they have not been defined yet and need to be
	    thermal_throttle_active = true;
    // Set runtime throttles to default values as a starting value
	    throttle_n_per_mba = MEM_THROTTLE_N_DEFAULT_PER_MBA;
	    throttle_n_per_chip = MEM_THROTTLE_N_DEFAULT_PER_CHIP * num_mba_with_dimms;
	    throttle_d = MEM_THROTTLE_D_DEFAULT;
	}
	else if ((throttle_n_per_mba != MEM_THROTTLE_N_DEFAULT_PER_MBA) || (throttle_n_per_chip != (MEM_THROTTLE_N_DEFAULT_PER_CHIP * num_mba_with_dimms)) || (throttle_d != MEM_THROTTLE_D_DEFAULT))
	{
    // if runtime throttles are not equal to the default values, then thermal throttles are in place
	    thermal_throttle_active = true;
	}
	else
	{
    // runtime throttles are not all zero and equal to the defaults, so no thermal throttles are in place - so now any throttles will be power based
	    thermal_throttle_active = false;
	}

// Adjust power limit value as needed here
// For CDIMM, we want the throttles to be per-chip, and to allow all commands to go to one MBA to get to the power limit
	if (dimm_type == CDIMM)
	{
// Set channel pair power limit to whole CDIMM power limit (multiply by number of MBAs used) and subtract off idle power for dimms on other MBA
	    channel_pair_watt_target = channel_pair_watt_target * num_mba_with_dimms;
	    for (port=0; port < MAX_NUM_PORTS; port++)
	    {
		for (dimm=0; dimm < MAX_NUM_DIMMS; dimm++)
		{
		    channel_pair_watt_target = channel_pair_watt_target - ((num_mba_with_dimms - 1) * (power_int_array[port][dimm]));
		}
	    }
	}

// calculate power and change throttle values in this while loop until limit has been satisfied or throttles have reached the minimum limit
	not_enough_available_power = false;
	channel_pair_throttle_done = false;
	while (channel_pair_throttle_done == false)
	{
	    rc = mss_throttle_to_power_calc(
					    i_target_mba,
					    throttle_n_per_mba,
					    throttle_n_per_chip,
					    throttle_d,
					    channel_pair_power
					    );
	    if(rc)
	    {
		FAPI_ERR("Error (0x%x) calling mss_throttle_to_power_calc", static_cast<uint32_t>(rc));
		return rc;
	    }

// compare channel pair power to mss_watt_target for channel and decrease throttles if it is above this limit
// throttle decrease will decrement throttle numerator by one (or increase throttle denominator) and recalculate power until utilization (N/M) reaches a lower limit

	    if (channel_pair_power > channel_pair_watt_target)
	    {
// check to see if dimm utilization is greater than the min utilization limit, continue if it is, error if it is not
		if ((((((float)throttle_n_per_chip * 100 * 4) / throttle_d) > MIN_UTIL) && (dimm_type != CDIMM) && (thermal_throttle_active == false)) || (((((float)throttle_n_per_chip * 100 * 4) / throttle_d) > MIN_UTIL) && (dimm_type != CDIMM) && (thermal_throttle_active == true)) || (((((float)throttle_n_per_chip * 100 * 4) / throttle_d) > MIN_UTIL) && (dimm_type == CDIMM)))
		{
		    if (((throttle_n_per_chip > 1) && (dimm_type != CDIMM) && (thermal_throttle_active == false)) || ((throttle_n_per_chip > 1) && (dimm_type != CDIMM) && (thermal_throttle_active == true)) || ((throttle_n_per_chip > 1) && (dimm_type == CDIMM)))
		    {
			if (dimm_type == CDIMM)
			{
			    // CDIMMs, use per chip throttling for any thermal or available power limits
			    throttle_n_per_chip--;
			}
			else
			{
			    // ISDIMMs, use per slot throttling for thermal power limits
			    if (thermal_throttle_active == true)
			    {
// per_mba throttling (ie.  per dimm for ISDIMMs) will limit performance if all traffic is sent to one dimm, so use the per_chip
// This works as long as the other dimm is providing termination (for 2 dimms per channel)
//    If the other dimm is not providing termination, then we would want to redefine the power curve in mss_eff_config_thermal and use the per_mba throttle here
// It there is only one dimm on channel, then it will provide its own termination and the per_mba and per_chip will effectively do the same throttling (ie.  doesn't matter which one we do in this case)
// Warning:  If this changes, then the two if statements above need to be modified
//				throttle_n_per_mba--;
				throttle_n_per_chip--;
			    }
			    else
			    {
				// ISDIMMs, use per mba throttling for available power limit
// Warning:  If this changes, then the two if statements above need to be modified
				throttle_n_per_chip--;
			    }
			}
		    }
// increment throttle denominator if numerator is at one (its lowest setting)
		    else
		    {
			throttle_d++;
		    }
		    FAPI_DBG("Throttle update [N_per_mba/N_per_chip/M %d/%d/%d]", throttle_n_per_mba, throttle_n_per_chip, throttle_d);
		}
// minimum utilization limit was reached for this throttle N/M value
		else
		{
// Throttles can't be changed anymore (already at or below MIN_UTIL)
		    channel_pair_throttle_done = true;
		    not_enough_available_power = true;
		}
	    }
// channel pair power is less than limit, so keep existing throttles
	    else
	    {
		FAPI_DBG("There is enough available memory power [Channel Pair Power %4.2f/%d cW]", channel_pair_power, channel_pair_watt_target);
		channel_pair_throttle_done = true;
	    }
	}

	FAPI_DBG("Final Throttle Settings [N_per_mba/N_per_chip/M %d/%d/%d]", throttle_n_per_mba, throttle_n_per_chip, throttle_d);


// update output attributes
	rc = FAPI_ATTR_SET(ATTR_MSS_MEM_THROTTLE_NUMERATOR_PER_MBA, &i_target_mba, throttle_n_per_mba);
	if(rc) return rc;
	rc = FAPI_ATTR_SET(ATTR_MSS_MEM_THROTTLE_NUMERATOR_PER_CHIP, &i_target_mba, throttle_n_per_chip);
	if(rc) return rc;
	rc = FAPI_ATTR_SET(ATTR_MSS_MEM_THROTTLE_DENOMINATOR, &i_target_mba, throttle_d);
	if(rc) return rc;

	if (not_enough_available_power == true)
	{
	    FAPI_ERR("Not enough available memory power [Channel Pair Power %4.2f/%d cW]", channel_pair_power, channel_pair_watt_target);
	    FAPI_SET_HWP_ERROR(rc, RC_MSS_NOT_ENOUGH_AVAILABLE_DIMM_POWER);
	    if (rc) fapiLogError(rc);
	}
	FAPI_INF("*** %s COMPLETE ***", procedure_name);
	return rc;

    }


} //end extern C

