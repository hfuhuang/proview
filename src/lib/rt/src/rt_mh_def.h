/* 
 * Proview   $Id: rt_mh_def.h,v 1.2 2005-09-01 14:57:56 claes Exp $
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

#ifndef rt_mh_def_h
#define rt_mh_def_h

#ifndef pwr_h
#include "pwr.h"
#endif

#ifndef pwr_class_h
#include "pwr_class.h"
#endif

#ifndef pwr_baseclasses_h
#include "pwr_baseclasses.h"
#endif

#ifndef co_tree_h
#include "co_tree.h"
#endif

#ifndef rt_qcom_h
#include "rt_qcom.h"
#endif

#ifndef pwr_lst_h
#include "pwr_lst.h"
#endif

#ifndef rt_mh_net_h
#include "rt_mh_net.h"
#endif

#ifndef rt_mh_appl_h
#include "rt_mh_appl.h"
#endif

#ifndef rt_mh_outunit_h
#include "rt_mh_outunit.h"
#endif

#if 0
#ifndef rt_plc_timer_h
#include "rt_plc_timer.h"
#endif
#endif

#include "rt_gdh.h"
#include "rt_mh_msg.h"
#include "rt_gdh_msg.h"


#ifdef OS_ELN
# include $vaxelnc
# include $kernelmsg
# include $mutex
# include descrip
# include stdio
# include ctype
#elif defined OS_VMS
# include <descrip.h>
#endif

#ifndef OS_ELN
# include <stdio.h>
# include <ctype.h>
#endif


#define mh_cScanRate		10	/* Tenths of seconds between scans of sup objects */
#define mh_cMaxApplAlarm	50      /* Default value*/
#define mh_cMaxEvents		100	/* Default value*/
#define mh_cDetectTimer		1.0
#define mh_cMessageTimer	1.0

typedef struct mhs_ApplConnect mh_sApplConnect;
typedef struct mhs_ApplReturn mh_sApplReturn;
typedef union mhu_ApplReply mh_uApplReply;
typedef struct mhs_ProcLink mh_sProcLink;


/*------------------------------------------------------------------------------
* Enumerations
*/

typedef enum { 
  mh_eAgent__		= 0,    
  mh_eAgent_None	= 1,
  mh_eAgent_MH		= 2,
  mh_eAgent_IO		= 3,
  mh_eAgent_Plc		= 4,
  mh_eAgent_
} mh_eAgent;

typedef enum { 
  mh_eApplState__		= 0,    
  mh_eApplState_Aborted		= 1,
  mh_eApplState_Connected	= 2,
  mh_eApplState_Disconnected	= 3,
  mh_eApplState_New		= 4,
  mh_eApplState_Restarted	= 5,
  mh_eApplState_
} mh_eApplState;


/*------------------------------------------------------------------------------
* Structs
*/

struct mhs_ApplConnect {
  pwr_tObjid	  ApplObject;
  qcom_sQid       Qid;
  mh_mApplFlags   Flags;
  pwr_tString80   AbortEventName;
  mh_eEvent	  AbortEventType;
  mh_eEventPrio   AbortEventPrio;
  mh_mEventFlags  AbortEventFlags;
  pwr_tString80   AbortEventText;
};

struct mhs_ApplContext {
  mh_sHead		Head;
  qcom_sQid		Handler;
  mh_eApplState		State;
};

union mhu_ApplReply {
  pwr_tUInt32	    Sts;
  struct {
    pwr_tUInt32     Sts;
    pwr_tUInt32     NoOfActMessages;
  } Connect;
  struct {
    pwr_tUInt32     Sts;
    pwr_tUInt32     Idx;
  } Message;
  struct {
    pwr_tUInt32     Sts;
    mh_sApplMessage Message;
  } Info;
};

struct mhs_ApplReturn {
  pwr_tUInt32	  TargetIdx;
  mh_eEvent	  ReturnType;
  pwr_tString80   ReturnText;
};

#endif
