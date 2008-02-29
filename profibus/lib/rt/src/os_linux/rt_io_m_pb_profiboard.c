/* 
 * Proview   $Id: rt_io_m_pb_profiboard.c,v 1.12 2008-02-29 13:18:59 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* rt_io_m_pb_profiboard.c -- io methods for the profibus master object
   The PbMaster object serves as agent for one Profibus DP bus
   The board we use is Profiboard from Softing
*/

#pragma pack(1)

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>

#include "keywords.h"

#include "pb_type.h"
#include "pb_conf.h"
#include "pb_if.h"
#include "pb_err.h"
#include "pb_fmb.h"
#include "pb_dp.h"

#include "rt_io_pb_locals.h"

#include "pwr.h"
#include "co_cdh.h"
#include "pwr_baseclasses.h"
#include "pwr_profibusclasses.h"
#include "rt_io_base.h"
#include "rt_io_bus.h"
#include "rt_io_msg.h"
#include "rt_errh.h"
#include "rt_io_agent_init.h"
#include "rt_pb_msg.h"

#include "rt_io_profiboard.h"

#define DP_MAX_SERVICE_RETRY    10



static int count;

static pwr_tStatus IoAgentInit (
  io_tCtx	ctx,
  io_sAgent	*ap
);
static pwr_tStatus IoAgentRead (
  io_tCtx	ctx,
  io_sAgent	*ap
);
static pwr_tStatus IoAgentWrite (
  io_tCtx	ctx,
  io_sAgent	*ap
);
static pwr_tStatus IoAgentClose (
  io_tCtx	ctx,
  io_sAgent	*ap
);
static pwr_tStatus IoAgentSwap (
  io_tCtx	ctx,
  io_sAgent	*ap
);



/*----------------------------------------------------------------------------*\
  Sends request to Profiboard for setting FMB parameters
\*----------------------------------------------------------------------------*/
static short try_profi_rcv_con_ind(T_PROFI_DEVICE_HANDLE *hDevice,
                                   T_PROFI_SERVICE_DESCR  *con_ind_sdb,  
				   USIGN8  *con_ind_buffer,
				   USIGN16 *con_ind_buffer_len,
				   INT16   *result) 
{
  int retry_counter;
  struct timespec rqtp = {0, 10000000}; // 10 ms  
  retry_counter = DP_MAX_SERVICE_RETRY;

  do {
    nanosleep(&rqtp, NULL);
    *result = profi_rcv_con_ind (hDevice, con_ind_sdb, con_ind_buffer, con_ind_buffer_len);
  } while ((*result == NO_CON_IND_RECEIVED) && (retry_counter-- > 0));

  if (*result == E_IF_FATAL_ERROR) return (PB_FALSE);

  return (PB_TRUE);
}
/*----------------------------------------------------------------------------*\
  Sends request to Profiboard for setting FMB parameters
\*----------------------------------------------------------------------------*/
static short fmb_set_configuration(T_PROFI_DEVICE_HANDLE *hDevice,  
				   pwr_sClass_Pb_Profiboard *op) 
{
  T_PROFI_SERVICE_DESCR sdb;
  T_FMB_SET_CONFIGURATION_REQ data;

  USIGN8                   con_ind_buffer [256];
  USIGN16                  con_ind_buffer_len = 256;
  T_PROFI_SERVICE_DESCR    con_ind_sdb;
  INT16                    result;              /* !!! local result variable !!! */

  sdb.comm_ref = 0;
  sdb.layer = FMB;
  sdb.service = FMB_SET_CONFIGURATION;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  data.fms_active = PB_FALSE;
  data.dp_active = PB_TRUE;
  data.fdlif_active = PB_FALSE;
  data.fdl_evt_receiver = DP_USR;
  data.sm7_active = PB_FALSE;
  data.data_buffer_length = 256;

  data.dp.max_number_slaves = op->MaxNumberSlaves;
  data.dp.max_slave_output_len = op->MaxSlaveOutputLen;
  data.dp.max_slave_input_len = op->MaxSlaveInputLen;
  data.dp.max_slave_diag_entries = op->MaxNumberSlaves;
  data.dp.max_slave_diag_len = 244;
  data.dp.max_bus_para_len = 1024;
  data.dp.max_slave_para_len = 1024;

  profi_snd_req_res(hDevice, &sdb, &data, PB_FALSE);

  try_profi_rcv_con_ind(hDevice, &con_ind_sdb, con_ind_buffer, &con_ind_buffer_len, &result);
  
  if ((con_ind_sdb.service   == FMB_SET_CONFIGURATION) &&
      (con_ind_sdb.primitive == CON                  ) &&
      (con_ind_sdb.result    == POS                  )) {
    return (PB_TRUE);
  }

  return (PB_FALSE);

}


/*----------------------------------------------------------------------------*\
  Sends request to Profiboard for setting DP master parameters
\*----------------------------------------------------------------------------*/
static short dp_init_master(T_PROFI_DEVICE_HANDLE *hDevice)
{
  T_PROFI_SERVICE_DESCR sdb;
  T_DP_INIT_MASTER_REQ data;

  USIGN8                   con_ind_buffer [256];
  USIGN16                  con_ind_buffer_len = 256;
  T_PROFI_SERVICE_DESCR    con_ind_sdb;
  INT16                    result;              /* !!! local result variable !!! */

  sdb.comm_ref = 0;
  sdb.layer = DP;
  sdb.service = DP_INIT_MASTER;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  data.master_default_address = 0;
  data.master_class2 = PB_FALSE;
  data.lowest_slave_address = 2;
  data.slave_io_address_mode = DP_AAM_IO_BLOCKS; // only mode possible with Linux-driver DP_AAM_ARRAY;
  data.clear_outputs = PB_TRUE;
  data.auto_remote_services = DP_AUTO_REMOTE_SERVICES;
  data.cyclic_data_transfer = PB_TRUE;

  profi_snd_req_res(hDevice, &sdb, &data, PB_FALSE);

  try_profi_rcv_con_ind(hDevice, &con_ind_sdb, con_ind_buffer, &con_ind_buffer_len, &result);
  
  if ((con_ind_sdb.service   == DP_INIT_MASTER) &&
      (con_ind_sdb.primitive == CON                  ) &&
      (con_ind_sdb.result    == POS                  )) {
    return (PB_TRUE);
  }

  return (PB_FALSE);
}


/*----------------------------------------------------------------------------*\
  Sends request to Profiboard for setting DP bus parameters
\*----------------------------------------------------------------------------*/
static short dp_download_bus(T_PROFI_DEVICE_HANDLE *hDevice,  
			    pwr_sClass_Pb_Profiboard *op)
{
  T_PROFI_SERVICE_DESCR sdb;
  struct {
    T_DP_DOWNLOAD_REQ drp; 
    T_DP_BUS_PARA_SET dbp;
  } data;
  int i;

  USIGN8                   con_ind_buffer [256];
  USIGN16                  con_ind_buffer_len = 256;
  T_PROFI_SERVICE_DESCR    con_ind_sdb;
  INT16                    result;              /* !!! local result variable !!! */

  sdb.comm_ref = 0;
  sdb.layer = DP;
  sdb.service = DP_DOWNLOAD_LOC;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  data.drp.data_len =  66;
  data.drp.rem_add = 0;
  data.drp.area_code = DP_AREA_BUS_PARAM;
  data.drp.add_offset = 0;

  data.dbp.bus_para_len = swap16(66);
  data.dbp.fdl_add = 0;

  switch (op->BaudRate) {
    case 500:
      data.dbp.baud_rate = DP_KBAUD_500;
      break;
    case 750:
      data.dbp.baud_rate = DP_KBAUD_750;
      break;
    case 1500:
      data.dbp.baud_rate = DP_KBAUD_1500;
      break;
    case 3000:
      data.dbp.baud_rate = DP_KBAUD_3000;
      break;
    case 6000:
      data.dbp.baud_rate = DP_KBAUD_6000;
      break;
    case 12000:
      data.dbp.baud_rate = DP_KBAUD_12000;
      break;
    default:
      data.dbp.baud_rate = DP_KBAUD_1500;
      break;
  }

  data.dbp.tsl = swap16(op->Tsl);
  data.dbp.min_tsdr = swap16(op->MinTsdr);
  data.dbp.max_tsdr = swap16(op->MaxTsdr);
  data.dbp.tqui = op->Tqui;
  data.dbp.tset = op->Tset;
  data.dbp.ttr = swap32(op->Ttr);
  data.dbp.g = op->G;
  data.dbp.hsa = op->Hsa;
  data.dbp.max_retry_limit = op->MaxRetryLimit;
  data.dbp.bp_flag = op->BpFlag;
  data.dbp.min_slave_interval = swap16(op->MinSlaveInterval);
  data.dbp.poll_timeout = swap16(op->PollTimeout);
  data.dbp.data_control_time = swap16(op->DataControlTime);
  for (i=0; i<6; i++) 
    data.dbp.reserved[i] = 0;
  data.dbp.master_user_data_len = swap16(DP_MASTER_USER_DATA_LEN);
  for (i=0; i<32; i++) 
    data.dbp.master_class2_name[i] = 0;

  profi_snd_req_res(hDevice, &sdb, &data, PB_FALSE);

  try_profi_rcv_con_ind(hDevice, &con_ind_sdb, con_ind_buffer, &con_ind_buffer_len, &result);

  if ((con_ind_sdb.service   == DP_DOWNLOAD_LOC) &&
      (con_ind_sdb.primitive == CON                  ) &&
      (con_ind_sdb.result    == POS                  )) {
    return (PB_TRUE);
  }

  return (PB_FALSE);

}


/*----------------------------------------------------------------------------*\
  Sends request for selecting operation mode to the Profiboard
\*----------------------------------------------------------------------------*/
static short dp_act_param_loc(T_PROFI_DEVICE_HANDLE *hDevice, 
			   short arg) {
  T_PROFI_SERVICE_DESCR sdb;
  T_DP_ACT_PARAM_REQ apr;
  pwr_tUInt16 retval;
    
  /* Fill the service description block */
  sdb.comm_ref = 0;
  sdb.layer = DP;
  sdb.service = DP_ACT_PARAM_LOC;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  apr.rem_add = 0;
  apr.area_code = DP_AREA_SET_MODE;
  apr.activate = arg;
  apr.dummy = 0;

  retval = profi_snd_req_res(hDevice, &sdb, &apr, PB_FALSE);

  return retval;
}

/*----------------------------------------------------------------------------*\
  Sends request for getting slave diagnostics
\*----------------------------------------------------------------------------*/
static pwr_tBoolean dp_get_slave_diag(T_PROFI_DEVICE_HANDLE *hDevice) 
{
  T_PROFI_SERVICE_DESCR sdb;
  pwr_tUInt16 retval;
    
  /* Fill the service description block */
  sdb.comm_ref = 0;
  sdb.layer = DP;
  sdb.service = DP_GET_SLAVE_DIAG;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  retval = profi_snd_req_res(hDevice, &sdb, &sdb, PB_FALSE);

  return ((pwr_tBoolean) (retval == E_OK));
}

/*----------------------------------------------------------------------------*\
  Get slave diagnostics
\*----------------------------------------------------------------------------*/
static void dp_get_slave_diag_con(T_DP_GET_SLAVE_DIAG_CON * get_slave_diag_con_ptr, io_sRack *slave_list) 
{
  T_DP_DIAG_DATA    FAR *diag_data_ptr;
//  char                     s [128];
  pwr_sClass_Pb_DP_Slave  *sp;

  if (get_slave_diag_con_ptr->diag_data_len >= DP_MIN_SLAVE_DIAG_LEN)
  {
    diag_data_ptr = (T_DP_DIAG_DATA FAR*) (get_slave_diag_con_ptr + 1);

    while (slave_list != NULL) {
      sp = (pwr_sClass_Pb_DP_Slave *) slave_list->op;
      
      if (sp->SlaveAddress == get_slave_diag_con_ptr->rem_add) {
        sp->StationStatus1 = diag_data_ptr->station_status_1;
        sp->StationStatus2 = diag_data_ptr->station_status_2;
        sp->StationStatus3 = diag_data_ptr->station_status_3;
	
	sp->BytesOfDiag = get_slave_diag_con_ptr->diag_data_len - DP_MIN_SLAVE_DIAG_LEN;
	
	memcpy(sp->Diag, diag_data_ptr + 1, MIN(get_slave_diag_con_ptr->diag_data_len - DP_MIN_SLAVE_DIAG_LEN, DP_MAX_EXT_DIAG_DATA_LEN));	
	
	/* Update slave status */
	
	if (!(sp->StationStatus1 & ~pwr_mPbStationStatus1Mask_ExternalDiag) &&
	    !(sp->StationStatus2 & ~(pwr_mPbStationStatus2Mask_Default |
		                     pwr_mPbStationStatus2Mask_ResponseMonitoringOn))) {
	  sp->Status = PB__NORMAL;
	}
	else if (sp->StationStatus1 & pwr_mPbStationStatus1Mask_NonExistent) {
	  sp->Status = PB__NOCONN;
	}
	else if ((sp->StationStatus1 & (pwr_mPbStationStatus1Mask_ConfigFault |
	                                pwr_mPbStationStatus1Mask_ParamFault)) ||
	         (sp->StationStatus2 & pwr_mPbStationStatus2Mask_NewParamsRequested)) {
	  sp->Status = PB__CONFIGERR;
        } 
	else if (sp->StationStatus1 & pwr_mPbStationStatus1Mask_MasterLock) {
	  sp->Status = PB__MASTERLOCK;
	}
	else //if (sp->StationStatus1 & pwr_mPbStationStatus1Mask_NotReady) 
	{
	  sp->Status = PB__NOTREADY;
	}
	
	
	break;
      }
      
      slave_list = slave_list->next;
    }

/*    sprintf (s, "Slave [%3hu] [0x%04hX]: Status = 0x%02hX 0x%02hX 0x%02hX, Master = %3hu, Ext = %hu, Diags = %hu",
             get_slave_diag_con_ptr->rem_add,
             swap16 (diag_data_ptr->ident_number),
             diag_data_ptr->station_status_1,
             diag_data_ptr->station_status_2,
             diag_data_ptr->station_status_3,
             diag_data_ptr->master_add,
             get_slave_diag_con_ptr->diag_data_len - DP_MIN_SLAVE_DIAG_LEN,
             get_slave_diag_con_ptr->diag_entries);

    errh_Info( "Profibus DP slave diag - %s", s); */

  } /* diag_data_len */

}
/*----------------------------------------------------------------------------*\
  Initializes one DP slave in the master card
\*----------------------------------------------------------------------------*/
static pwr_tStatus dp_download_slave (
  T_PROFI_DEVICE_HANDLE *hDevice,
  pwr_sClass_Pb_DP_Slave *op
)
{

  int i;
  T_PROFI_SERVICE_DESCR   sdb;
  struct {
    T_DP_DOWNLOAD_REQ drp; 
    unsigned char param[512];
  } slave_data;

  T_DP_SLAVE_PARA_SET prm_head;
  T_DP_PRM_DATA prm_data;
  T_DP_AAT_DATA aat_data;
  T_DP_SLAVE_USER_DATA user_data;
  USIGN16 download_data_size;
  USIGN16 data_len;

  USIGN8                   con_ind_buffer [256];
  USIGN16                  con_ind_buffer_len = 256;
  T_PROFI_SERVICE_DESCR    con_ind_sdb;
  INT16                    result;              /* !!! local result variable !!! */

  op->Status = PB__NOTINIT;
 
  download_data_size = sizeof(prm_head) + sizeof(prm_data) + 
			op->PrmUserDataLen + op->ConfigDataLen +
			sizeof(aat_data) + sizeof(user_data);

  data_len = sizeof(slave_data.drp) + download_data_size;

  sdb.comm_ref = 0;
  sdb.layer = DP;
  sdb.service = DP_DOWNLOAD_LOC;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  slave_data.drp.data_len =  download_data_size;
  slave_data.drp.rem_add = 0;
  slave_data.drp.area_code = op->SlaveAddress;
  slave_data.drp.add_offset = 0;

  prm_head.slave_para_len = swap16(download_data_size);
  prm_head.sl_flag = DP_SL_NEW_PRM | DP_SL_ACTIVE;
  prm_head.slave_type = DP_SLAVE_TYPE_DP;
  for (i = 0; i < 12; i++) prm_head.reserved[i] = 0;
  
  i = 0;
  memcpy(&slave_data.param[i], &prm_head, sizeof(prm_head));
  i += sizeof(prm_head);

  prm_data.prm_data_len = swap16(sizeof(prm_data) + op->PrmUserDataLen);
  prm_data.station_status = DP_PRM_LOCK_REQ | DP_PRM_WD_ON;
  prm_data.wd_fact_1 = op->WdFact1;
  prm_data.wd_fact_2 = op->WdFact2;
  prm_data.min_tsdr = 0;
  prm_data.ident_number = swap16(op->PNOIdent);
  prm_data.group_ident = op->GroupIdent;

  memcpy(&slave_data.param[i], &prm_data, sizeof(prm_data));
  i += sizeof(prm_data);

  memcpy(&slave_data.param[i], op->PrmUserData, op->PrmUserDataLen);
  i += op->PrmUserDataLen;

  memcpy(&slave_data.param[i], op->ConfigData, op->ConfigDataLen);
  i += op->ConfigDataLen;

  aat_data.aat_data_len = swap16(4);		// AAT data not used in array mode
  aat_data.number_inputs = 0;
  aat_data.number_outputs = 0;
//  aat_data.offset_inputs = 0;
//  aat_data.offset_outputs = 0;

  memcpy(&slave_data.param[i], &aat_data, sizeof(aat_data));
  i += sizeof(aat_data);

  user_data.slave_user_data_len = swap16(2);

  memcpy(&slave_data.param[i], &user_data, sizeof(user_data));
  i += sizeof(user_data);

  profi_snd_req_res(hDevice, &sdb, &slave_data, PB_FALSE);

  try_profi_rcv_con_ind(hDevice, &con_ind_sdb, con_ind_buffer, &con_ind_buffer_len, &result);

  if ((con_ind_sdb.service   == DP_DOWNLOAD_LOC) &&
      (con_ind_sdb.primitive == CON                  ) &&
      (con_ind_sdb.result    == POS                  )) {
    op->Status = PB__NOCONN;
    return (PB_TRUE);
  }

  op->Status = PB__INITFAIL;

  return (PB_FALSE);

}


/*----------------------------------------------------------------------------*\
  Calculate offsets of inputs and outputs for a slave
\*----------------------------------------------------------------------------*/
static pwr_tStatus dp_io_offsets (
  T_PROFI_DEVICE_HANDLE *hDevice,
  pwr_sClass_Pb_DP_Slave *op
)
{
  T_PROFI_SERVICE_DESCR         sdb;
  T_DP_GET_SLAVE_PARAM_REQ      get_slave_param_req;
  T_DP_GET_SLAVE_PARAM_CON      FAR *get_slave_param_con_ptr;
  T_DP_SLAVE_PARAM_SLAVE_INFO   FAR *slave_info_ptr;

  USIGN8                   con_ind_buffer [256];
  USIGN16                  con_ind_buffer_len = 256;
  T_PROFI_SERVICE_DESCR    con_ind_sdb;
  INT16                    result;              /* !!! local result variable !!! */

  sdb.comm_ref = 0;
  sdb.layer = DP;
  sdb.service = DP_GET_SLAVE_PARAM;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  get_slave_param_req.identifier = DP_SLAVE_PARAM_SLAVE_INFO;
  get_slave_param_req.rem_add    = op->SlaveAddress;

  result = profi_snd_req_res (hDevice, &sdb, &get_slave_param_req, PB_FALSE);

  if (result != E_OK) return (result);
  
  try_profi_rcv_con_ind(hDevice, &con_ind_sdb, con_ind_buffer, &con_ind_buffer_len, &result);

  get_slave_param_con_ptr = (T_DP_GET_SLAVE_PARAM_CON FAR*) con_ind_buffer;

  if ( (con_ind_sdb.service   == DP_GET_SLAVE_PARAM) &&
       (con_ind_sdb.primitive == CON               ) &&
       (con_ind_sdb.result    == POS               ) )
  {
    slave_info_ptr = (T_DP_SLAVE_PARAM_SLAVE_INFO FAR*) (get_slave_param_con_ptr + 1);
    
    op->BytesOfInput  = slave_info_ptr->number_inputs;
    op->BytesOfOutput = slave_info_ptr->number_outputs;
    op->OffsetInputs  = slave_info_ptr->offset_inputs;
    op->OffsetOutputs = slave_info_ptr->offset_outputs;

    return (PB_TRUE);

  } 
  
  return (PB_FALSE);
}

/*----------------------------------------------------------------------------*\
   Init method for the Pb_profiboard agent  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoAgentInit (
  io_tCtx	ctx,
  io_sAgent	*ap
) 
{
  pwr_sClass_Pb_Profiboard *op;
  pwr_tUInt16 sts;
  pwr_tStatus status;
  T_PROFI_DEVICE_HANDLE *hDevice;
  char ok;

  pwr_tObjid slave_objid;
  pwr_tClassId slave_class;

  pwr_sClass_Pb_DP_Slave *sop;
  char name[196];

  struct timespec rqtp = {0, 20000000}; // 20 ms  
  
  int retry;
    
  count=0;

  /* Allocate area for local data structure */
  ap->Local = calloc(1, sizeof(io_sAgentLocal));
  if (!ap->Local) {
    errh_Error( "ERROR config Profibus DP Master %s - %s", ap->Name, "calloc");
    return IO__ERRINIDEVICE;
  }
    
  hDevice = (T_PROFI_DEVICE_HANDLE *) ap->Local;
  op = (pwr_sClass_Pb_Profiboard *) ap->op;

  op->Status = PB__NOTINIT;

  /* Initialize interface */
  
  if (ctx->Node->Restarts > 0) {
    nanosleep(&rqtp, NULL);
  }
  
  errh_Info( "Initializing interface for Profibus DP Master %s", ap->Name);
  sts = profi_init(hDevice, (unsigned char) op->BusNumber - 1, 0, 0);

  if (sts != E_OK)
  {
    /* Can't open driver */
    op->Status = PB__INITFAIL;
    errh_Error( "ERROR config Profibus DP Master %s - %s", ap->Name, "open device");
    ctx->Node->EmergBreakTrue = 1;
    return IO__ERRDEVICE;
  }

  /* If this is not the Profibus I/O process, return */
  
  if ((op->Process & io_mProcess_Profibus) && (ctx->Process != io_mProcess_Profibus)) {
    op->Status = PB__NOTINIT;
    errh_Info( "Init template I/O agent for Profibus DP Master %s, %d", ap->Name, ctx->Process );
    return IO__SUCCESS;
  }
  
  if (ctx->Node->Restarts > 0) {
    errh_Info( "Warm restart - Skipping config of Profibus DP Master %s", ap->Name);
    op->Status = PB__NORMAL;
//    return IO__SUCCESS;
  }
  
  errh_Info( "Config of Profibus DP Master %s", ap->Name);
      
  if (op->DisableBus != 1) {
  
    ok = FALSE;
    
    if (ctx->Node->Restarts == 0) {

      retry = 0;
      while (!ok) {
    
        op->Status = PB__NOTINIT;
      
        /* Set FMB configuration */

        sts = fmb_set_configuration(hDevice,  op); 
        if (!sts) {
          op->Status = PB__INITFAIL;
          errh_Error( "ERROR config Profibus DP  Master %s - %s", ap->Name, "fmb set configuration");
	  retry++;
	  if (retry < 2) {
            nanosleep(&rqtp, NULL);
	    continue;
	  } 
          return IO__ERRINIDEVICE;
        }

        /* Set DP master parameters */

        sts = dp_init_master(hDevice); 
        if (!sts) {
          op->Status = PB__INITFAIL;
          errh_Error( "ERROR config Profibus DP Master %s - %s", ap->Name, "dp init master");
          return IO__ERRINIDEVICE;
        }

        /* Download DP bus parameters */

        sts = dp_download_bus(hDevice,  op); 
        if (!sts) {
          op->Status = PB__INITFAIL;
          errh_Error( "ERROR config Profibus DP Master %s - %s", ap->Name, "dp download bus");
          return IO__ERRINIDEVICE;
        }
      
        /* Loop through all slaves (traverse agent's children) and initialize them */

        op->NumberSlaves = 0;
        status = gdh_GetChild(ap->Objid, &slave_objid);
  
        while (ODD(status)) {
          status = gdh_GetObjectClass(slave_objid, &slave_class);

          status = gdh_ObjidToPointer(slave_objid, (pwr_tAddress *) &sop);
          status = gdh_ObjidToName(slave_objid, (char *) &name, sizeof(name), cdh_mNName);
  
          errh_Info( "Download Profibus DP Slave config - %s", name );

          status = dp_download_slave(hDevice, sop);

          if (!status) {
            errh_Error( "ERROR Init Profibus DP slave %s", name);
	  }

          op->NumberSlaves++;
          status = gdh_GetNextSibling(slave_objid, &slave_objid);
        }

        /* Calculate offsets of inputs and outputs for a slave */
      
        status = gdh_GetChild(ap->Objid, &slave_objid);
  
        while (ODD(status)) {
          status = gdh_GetObjectClass(slave_objid, &slave_class);
          status = gdh_ObjidToPointer(slave_objid, (pwr_tAddress *) &sop);

          status = dp_io_offsets(hDevice, sop);

          status = gdh_GetNextSibling(slave_objid, &slave_objid);
        }

        /* Move to STOP mode, this will fix the DPRAM layout */

        sts = dp_act_param_loc(hDevice, DP_OP_MODE_STOP);
        if (sts != E_OK) {
          op->Status = PB__INITFAIL;
          errh_Error( "ERROR config Profibus DP Master %s - %s", ap->Name, "act param loc to STOPPED");
          return IO__ERRINIDEVICE;
        }
	      
        ok = TRUE;
	
      }  /* End - While !ok */
    }  /* End - Initialization only if not restart   */
/*    else {
       Move to STOP mode, this will fix the DPRAM layout 

      sts = dp_act_param_loc(hDevice, DP_OP_MODE_STOP);
      if (sts != E_OK) {
        op->Status = PB__INITFAIL;
        errh_Error( "ERROR config Profibus DP Master %s - %s", ap->Name, "act param loc to STOPPED");
        return IO__ERRINIDEVICE;
      }
    } */
  }    
  else
    op->Status = PB__DISABLED;
  
  return IO__SUCCESS;
}

/*----------------------------------------------------------------------------*\
   Swap method for the Pb_profiboard agent  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoAgentSwap (
  io_tCtx	ctx,
  io_sAgent	*ap
) 
{
  pwr_sClass_Pb_Profiboard *op;
  char DeviceName[64];
  io_sAgentLocal *local;
  
  if (ap->Local == NULL) {
    /* Allocate area for local data structure */
    ap->Local = calloc(1, sizeof(io_sAgentLocal));
    if (!ap->Local) {
      errh_Error( "ERROR swap init Profibus DP Master %s - %s", ap->Name, "calloc");
      return IO__ERRINIDEVICE;
    }
    
    local = (io_sAgentLocal *) ap->Local;

    errh_Info( "Swap init interface for Profibus DP Master %s", ap->Name);

    op = (pwr_sClass_Pb_Profiboard *) ap->op;
    
    sprintf(DeviceName, "/dev/pbboard%u", op->BusNumber - 1);
    local->hDpsBoardDevice = open(DeviceName, O_RDONLY | O_NONBLOCK);
    
    if (local->hDpsBoardDevice == -1) {
      errh_Error( "ERROR swap init Profibus DP Master %s - %s", ap->Name, "open");
      return IO__ERRINIDEVICE;
    }
  }
      
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
   Read method for the Pb_Profiboard agent  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoAgentRead (
  io_tCtx	ctx,
  io_sAgent	*ap
)
{
  io_sAgentLocal *local;
  T_PROFI_DEVICE_HANDLE *hDevice;
  pwr_sClass_Pb_Profiboard *op;
  pwr_tUInt16 sts;
  static int count;
  char                     s [128];

  USIGN8                   con_ind_buffer [256];
  USIGN16                  con_ind_buffer_len = 256;
  T_PROFI_SERVICE_DESCR    con_ind_sdb;
  T_DP_GET_SLAVE_DIAG_CON   FAR *get_slave_diag_con_ptr;

  hDevice = (T_PROFI_DEVICE_HANDLE *) ap->Local;
  local = (io_sAgentLocal *) ap->Local;
  op = (pwr_sClass_Pb_Profiboard *) ap->op;

  /* If everything is fine we should be in state OPERATE
     Make a poll to see if there are diagnostics, the answer also tell us
     if there are any hardware faults. In that case, make a reset and a new init. */

  count++;
  
  if ((op->Process & io_mProcess_Profibus) && (ctx->Process != io_mProcess_Profibus))
    return IO__SUCCESS;
    
  if (op->DisableBus != 1) {
  
    switch (op->Status) {
  
      case PB__NORMAL:
      case PB__STOPPED:
      case PB__CLEARED:
      case PB__NOTINIT:
        sts = profi_rcv_con_ind (  hDevice, &con_ind_sdb, con_ind_buffer, &con_ind_buffer_len);
	
	if (sts == CON_IND_RECEIVED) {
          if (con_ind_sdb.primitive == CON) {
            if (con_ind_sdb.result == POS) {
              switch (con_ind_sdb.service) {
                /*--------------------------------------------------------------*/

                case DP_ACT_PARAM_LOC: {

                  if (op->Status == PB__NOTINIT) {
                    op->Status = PB__STOPPED;
                    errh_Info( "Profibus DP Master %s to state STOPPED", ap->Name);
                    dp_act_param_loc(hDevice, DP_OP_MODE_CLEAR);
                  }
                  else if (op->Status == PB__STOPPED) {
                    op->Status = PB__CLEARED;
                    errh_Info( "Profibus DP Master %s to state CLEARED", ap->Name);
                    dp_act_param_loc(hDevice, DP_OP_MODE_OPERATE);
                  }
                  else if (op->Status == PB__CLEARED) {
                    errh_Info( "Profibus DP Master %s to state OPERATE", ap->Name);
                    op->Status = PB__NORMAL;
                  }

                  break;
                } /* case DP_ACT_PARAM_LOC */

                /*--------------------------------------------------------------*/

                case DP_GET_SLAVE_DIAG: {
                  get_slave_diag_con_ptr = (T_DP_GET_SLAVE_DIAG_CON FAR*) con_ind_buffer;

                  dp_get_slave_diag_con (get_slave_diag_con_ptr, ap->racklist);

                  local->slave_diag_requested = PB_FALSE;

                  if (get_slave_diag_con_ptr->diag_entries) {
                    local->slave_diag_requested = PB_TRUE;
                    dp_get_slave_diag (hDevice);
                  }
                  break;
                } /* case DP_GET_SLAVE_DIAG */

                /*--------------------------------------------------------------*/

                default: {
		  break;
                } /* deafult service */
              } /* switch */
            } /* if POS */
            else {
              op->Status = PB__NOTINIT;
              errh_Error( "Profibus DP Master %s - %d neg con rec", ap->Name, count );      
            } /* else POS */
          } /* if CON */
          else if (con_ind_sdb.primitive == IND) {
            if (con_ind_sdb.result == POS) {
              switch (con_ind_sdb.service) {
                case FMB_FM2_EVENT: {
                  switch (((T_FMB_FM2_EVENT_IND FAR*) con_ind_buffer)->reason) {
                    case FM2_FAULT_ADDRESS     : sprintf (s, "Duplicate address recognized"); break;
                    case FM2_FAULT_PHY         : sprintf (s, "Phys.layer is malfunctioning"); break;
                    case FM2_FAULT_TTO         : sprintf (s, "Time out on bus detected    "); break;
                    case FM2_FAULT_SYN         : sprintf (s, "No receiver synchronization "); break;
                    case FM2_FAULT_OUT_OF_RING : sprintf (s, "Station out of ring         "); break;
                    case FM2_GAP_EVENT         : sprintf (s, "New station in ring         "); break;

                    default                    : sprintf (s, "Unknown reason code received");

                  } /* switch reason */

                  errh_Info( "Profibus DP Master %s - %s", ap->Name, s );      
 
                  break;
                } /* case FMB_FM2_EVENT */

                /*--------------------------------------------------------------*/

                case DP_ACT_PARAM_LOC: {
		  USIGN8 usif_state;
                  usif_state = ((T_DP_ACT_PARAM_IND FAR*) con_ind_buffer)->activate;

                  switch (usif_state) {
                    case DP_OP_MODE_STOP   : {
		      op->Status = PB__STOPPED;
		      sprintf (s, "Mode changed to STOP");
		      break;
		    }
                    case DP_OP_MODE_CLEAR  : {
		      op->Status = PB__CLEARED;
		      sprintf (s, "Mode changed to CLEAR");
		      break;
		    }
                    case DP_OP_MODE_OPERATE: { 
		      op->Status = PB__NORMAL;
		      sprintf (s, "Mode changed to OPERATE");
		      break;
		    }
                  }

                  errh_Info( "Profibus DP Master %s - %s", ap->Name, s );      

                  if (usif_state == DP_OP_MODE_STOP) {
                    usif_state = DP_OP_MODE_CLEAR;

                    dp_act_param_loc(hDevice, DP_OP_MODE_CLEAR);
                  }
                  else if (usif_state == DP_OP_MODE_CLEAR) {
                    usif_state = DP_OP_MODE_OPERATE;

                    dp_act_param_loc (hDevice, DP_OP_MODE_OPERATE);
                  }

                  break;
                } /* case DP_ACT_PARAM_LOC */

                /*--------------------------------------------------------------*/

                case DP_GET_SLAVE_DIAG: {
                  get_slave_diag_con_ptr = (T_DP_GET_SLAVE_DIAG_CON FAR*) con_ind_buffer;

                  dp_get_slave_diag_con (get_slave_diag_con_ptr, ap->racklist);

                  op->Diag[0]++;
		  
                  if ( (get_slave_diag_con_ptr->diag_entries) &&
                       (! local->slave_diag_requested              ) ) {
                    if (dp_get_slave_diag(hDevice)) {
                      local->slave_diag_requested = PB_TRUE;
                    }
                  }

                  break;
                } /* case DP_GET_SLAVE_DIAG */

                /*--------------------------------------------------------------*/

                default: {
		  break;
                } /* deafult service */
              } /* switch */
            } /* if POS */
            else {
              op->Status = PB__NOTINIT;
              errh_Error( "Profibus DP Master %s - %d neg ind rec", ap->Name, count );      
            } /* else POS */
          } /* if IND */
	} else if (sts != NO_CON_IND_RECEIVED) {
          op->Status = PB__NOTINIT;
	}

        break;

      default:
        op->Status = PB__NOTINIT;
        errh_Error( "Reconfig of Profibus DP Master %s - %d", ap->Name, count );      
        IoAgentClose(ctx, ap);
        IoAgentInit(ctx, ap);
        break;
    }
  }
   
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
   Write method for the Pb_Profiboard agent  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoAgentWrite (
  io_tCtx	ctx,
  io_sAgent	*ap
) 
{
  return IO__SUCCESS;
}


/*----------------------------------------------------------------------------*\
  
\*----------------------------------------------------------------------------*/
static pwr_tStatus IoAgentClose (
  io_tCtx	ctx,
  io_sAgent	*ap
) 
{
  io_sAgentLocal 	*local;
  T_PROFI_DEVICE_HANDLE *hDevice;
  pwr_tStatus            sts = PB_FALSE;

  T_PROFI_SERVICE_DESCR sdb;

  USIGN8                   con_ind_buffer [256];
  USIGN16                  con_ind_buffer_len = 256;
  T_PROFI_SERVICE_DESCR    con_ind_sdb;
  INT16                    result;              /* !!! local result variable !!! */

  local = (io_sAgentLocal *) ap->Local;
  hDevice = (T_PROFI_DEVICE_HANDLE *) ap->Local;
  sdb.comm_ref = 0;
  sdb.layer = DP;
  sdb.service = DP_EXIT_MASTER;
  sdb.primitive = REQ;
  sdb.invoke_id = 0;
  sdb.result = 0;

  profi_snd_req_res(hDevice, &sdb, &sdb, PB_FALSE);

  try_profi_rcv_con_ind(hDevice, &con_ind_sdb, con_ind_buffer, &con_ind_buffer_len, &result);
    
  if ((con_ind_sdb.service   == DP_EXIT_MASTER) &&
      (con_ind_sdb.primitive == CON                  ) &&
      (con_ind_sdb.result    == POS                  )) {
    sts = PB_TRUE;
  }

  close(local->hServiceReadDevice);
  close(local->hServiceWriteDevice);
  close(local->hDpDataDevice);
  close(local->hDpsInputDataDevice);
  close(local->hDpsOutputDataDevice);
  close(local->hDpsBoardDevice);

  free( (char *)local);

  return sts;
}


/*----------------------------------------------------------------------------*\
  Every method to be exported to the workbench should be registred here.
\*----------------------------------------------------------------------------*/

pwr_dExport pwr_BindIoMethods(Pb_Profiboard) = {
  pwr_BindIoMethod(IoAgentInit),
  pwr_BindIoMethod(IoAgentRead),
  pwr_BindIoMethod(IoAgentWrite),
  pwr_BindIoMethod(IoAgentClose),
  pwr_BindIoMethod(IoAgentSwap),
  pwr_NullMethod
};
