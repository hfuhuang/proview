/* 
 * Proview   $Id: rt_rtt_alarm.c,v 1.3 2005-09-01 14:57:56 claes Exp $
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

/* rt_rtt_alarm.c 
   This module contains routines for handling of alarms in rtt. */

/*_Include files_________________________________________________________*/



#if defined(OS_ELN)
# include $vaxelnc
# include stdio
# include string
# include chfdef
# include stdlib
# include $get_message_text
#else
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#ifndef RT_MH_LOADED
#include "rt_mh.h"
#endif
#ifndef RT_MH_OUTUNIT_LOADED
#include "rt_mh_outunit.h"
#endif
#include "rt_mh_msg.h"
#include "rt_mh_appl.h"
#include "rt_rtt.h"
#include "rt_rtt_edit.h"
#include "rt_rtt_global.h"
#include "rt_rtt_functions.h"
#include "rt_rtt_msg.h"

#if defined OS_LINUX
# include <time.h>
#endif

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define RTT_ALARMTYPE_INFO	0
#define RTT_ALARMTYPE_ALARM	1
#define RTT_ALARMTYPE_ACK	2
#define RTT_ALARMTYPE_RETURN	3

#define	BEEP	    rtt_printf( "%c", '\7');

typedef union alau_Event ala_uEvent;
union alau_Event 
{
    mh_sMsgInfo Info;
    mh_sAck	Ack;
    mh_sMessage Msg;
    mh_sReturn  Return;
};



/** Local variables ********************************************************/
static 	int			rtt_appl_alarm_connected = 0;
static	int			rtt_alarm_connected = 0;
static	int			rtt_eventlist_index;
static	int			rtt_alarmlist_index;
static	int			rtt_eventlist_maxindex;
static	int			rtt_alarmlist_maxindex;
static	FILE 			*rtt_alarmlog_file;
static	int			rtt_alarmlog_active = 0;


/** Local function prototypes ********************************************************/

static pwr_tStatus rtt_mh_info_bc( mh_sMessage *MsgP);
static pwr_tStatus rtt_mh_alarm_bc( mh_sMessage *MsgP);
static pwr_tStatus rtt_mh_ack_bc( mh_sAck *MsgP);
static pwr_tStatus rtt_mh_return_bc( mh_sReturn *MsgP);
static pwr_tStatus rtt_mh_block_bc( mh_sBlock *MsgP);
static pwr_tStatus rtt_mh_cancel_bc( mh_sReturn *MsgP);
static pwr_tStatus rtt_mh_clear_alarmlist_bc( pwr_tNodeIndex nix);
static pwr_tStatus rtt_mh_clear_blocklist_bc( pwr_tNodeIndex nix);
static int	rtt_appl_connect_alarm();
static int	rtt_menu_alarm_configure(
			menu_ctx	ctx,
			int		reconfigure);
static int	rtt_alarm_get_previous_page( menu_ctx	ctx);
static int	rtt_alarm_get_next_page( menu_ctx	ctx);
static int	rtt_menu_alarm_list_add(
			rtt_t_menu_alarm	**menulist,
			int		*index,
			int		maxindex,
			char		*text,
			int		(* func) (),
			int		(* func2) (),
			int		(* func3) (),
			pwr_tObjid	argoi,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			mh_sMessage 	*MsgP,
			int		type);
static int	rtt_menu_alarm_list_add_malloc(
			rtt_t_menu_alarm	**menulist,
			int			index);
static int	rtt_menu_item_alarm_delete(
			rtt_t_menu_alarm	*menu_ptr,
			int			item,
			int			*numberof_items);
static int	rtt_menu_alarm_draw(
			menu_ctx	ctx,
			int		noerase);
static int	rtt_event_item_text(
			rtt_t_menu_alarm	*menu_ptr,
			int			index);
int	rtt_alarm_item_text(
			rtt_t_menu_alarm	*menu_ptr,
			int			index);
static int	rtt_event_print_text(
			rtt_t_menu_alarm	*menu_ptr,
			int			index,
			char			*text,
			int			size,
			int			notext,
			int			noname);
static int	rtt_alarm_get_index( 
		mh_sEventId	*id,
		int 		*alarm_index);
static int	rtt_timestring(
			pwr_tTime	*time,
			char		*timestr);
static int	rtt_alarm_beep();
static int	rtt_alarm_last_message();
static int	rtt_alarmlog_log( char *type, rtt_t_menu_alarm	*menu_ptr);


/****************************************************************************
* Name:		rtt_appl_alarm_connect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Connect to alarm handler.
*
**************************************************************************/
static int	rtt_appl_connect_alarm () 
{
	int		sts ; 
	pwr_tUInt32	NoOfActMessages ;

        if ( rtt_appl_alarm_connected)
	  /* We are already connected */
	  return RTT__SUCCESS;

	sts = mh_ApplConnect(
		pwr_cNObjid,
		0,
		"AbortEventName",
		mh_eEvent_Info,
		mh_eEventPrio_D,
		mh_mEventFlags_Bell | mh_mEventFlags_Ack | mh_mEventFlags_Return ,
		"AbortEventText",
		&NoOfActMessages
		);
	if( EVEN(sts)) return sts;

	rtt_appl_alarm_connected = 1;
	return RTT__SUCCESS;    
}

/*************************************************************************
*
* Name:		rtt_alarm_send()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Sends an alarm with the specified text.
*
**************************************************************************/

int	rtt_alarm_send(
			char	*alarm_text,
			int	alarm_prio ) 
{
	mh_sApplMessage 	mh_msg ;
	pwr_tUInt32	  	mh_id ; 
	int			sts ;
  
	sts = rtt_appl_connect_alarm();
	if ( EVEN(sts)) return sts;	
	mh_msg.Object = pwr_cNObjid;
	mh_msg.EventFlags = mh_mEventFlags_Returned |
			 mh_mEventFlags_NoObject | 
			 mh_mEventFlags_Bell;
	time_GetTime( &mh_msg.EventTime);
	mh_msg.SupObject = pwr_cNObjid;
	mh_msg.Outunit = pwr_cNObjid;
	strcpy ( mh_msg.EventName , "Message from RTT");
	strcpy ( mh_msg.EventText , alarm_text );
	mh_msg.EventType = mh_eEvent_Alarm;
	mh_msg.SupInfo.SupType = mh_eSupType_None; 
	switch ( alarm_prio)
	{
	  case 'A':
	    mh_msg.EventPrio = mh_eEventPrio_A;
	    break ;
	  case 'B':
	    mh_msg.EventPrio = mh_eEventPrio_B;
	    break; 
	  case 'C':
	    mh_msg.EventPrio = mh_eEventPrio_C;
	    break; 
	  case 'D':
	    mh_msg.EventPrio = mh_eEventPrio_D;
	    break; 
	  case 'I':
	    mh_msg.EventType = mh_eEvent_Info;
	    mh_msg.EventFlags = mh_mEventFlags_InfoWindow;
	    break; 
	  default:
	    mh_msg.EventPrio = mh_eEventPrio_A;
	    break; 
	}
	sts = mh_ApplMessage ( &mh_id , &mh_msg ) ;
	if( EVEN(sts) ) return sts;
  
	return RTT__SUCCESS;
} 

/****   OUTUNIT ARE IMPLEMENTED FOR VMS ONLY !!! ************/


/****************************************************************************
* Name:		rtt_mh_info_bc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Backcall from mh.
*
**************************************************************************/
static pwr_tStatus rtt_mh_info_bc( mh_sMessage *MsgP)
{
	int		sts;
	ala_uEvent	*EventP;

	EventP = (ala_uEvent *) MsgP; 

/*	if (    EventP->Info.EventType == mh_eEvent_Info ||
		EventP->Info.EventType == mh_eEvent_Alarm  )
*/
	{
	  /* Insert in event list */
	  sts = rtt_menu_alarm_list_add( 
		(rtt_t_menu_alarm **) &rtt_event_ctx->menu,
		&rtt_eventlist_index, 
		rtt_eventlist_maxindex,
		"", 
		0, 
		0,
		0, pwr_cNObjid, 0, 0, 0, 0, 
		MsgP, RTT_ALARMTYPE_INFO);
	  if ( EVEN(sts)) return sts;

	  sts = rtt_event_item_text( (rtt_t_menu_alarm *) rtt_event_ctx->menu,
		rtt_eventlist_index - 1);

	  if ( rtt_event_ctx != 0)
	    rtt_event_ctx->update_init = 1;
	}

        if ((MsgP->Status & mh_mEventStatus_NotAck) ||
            (MsgP->Status & mh_mEventStatus_NotRet))
	{
	  /* Insert in alarm list */
	  sts = rtt_menu_alarm_list_add( 
		(rtt_t_menu_alarm **) &(rtt_alarm_ctx->menu),
		&rtt_alarmlist_index, 
		rtt_alarmlist_maxindex,
		"", 
		0, 
		0,
		0, pwr_cNObjid, 0, 0, 0, 0, 
		MsgP, RTT_ALARMTYPE_INFO);
	  if ( EVEN(sts)) return sts;

	  sts = rtt_alarm_item_text( (rtt_t_menu_alarm *) rtt_alarm_ctx->menu,
		rtt_alarmlist_index - 1);

	  if ( rtt_alarm_ctx != 0)
	    rtt_alarm_ctx->update_init = 1;
	}
	rtt_alarm_last_message();

	return RTT__SUCCESS;
}

/****************************************************************************
* Name:		rtt_mh_alarm_bc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Backcall from mh.
*
**************************************************************************/
static pwr_tStatus rtt_mh_alarm_bc( mh_sMessage *MsgP)
{
	int		sts;
	ala_uEvent	*EventP;

	EventP = (ala_uEvent *) MsgP ; 

/*	if (    EventP->Info.EventType == mh_eEvent_Info ||
		EventP->Info.EventType == mh_eEvent_Alarm  )
*/
	{
	  /* Insert in event list */
	  sts = rtt_menu_alarm_list_add( 
		(rtt_t_menu_alarm **) &rtt_event_ctx->menu,
		&rtt_eventlist_index, 
		rtt_eventlist_maxindex,
		"", 
		0, 
		0,
		0, pwr_cNObjid, 0, 0, 0, 0, 
		MsgP, RTT_ALARMTYPE_ALARM);
	  if ( EVEN(sts)) return sts;

	  sts = rtt_event_item_text( (rtt_t_menu_alarm *) rtt_event_ctx->menu,
		rtt_eventlist_index - 1);

	  if ( rtt_event_ctx != 0)
	    rtt_event_ctx->update_init = 1;
	}

        if ((MsgP->Status & mh_mEventStatus_NotAck) ||
            (MsgP->Status & mh_mEventStatus_NotRet))
	{
	  /* Insert in alarm list */
	  sts = rtt_menu_alarm_list_add( 
		(rtt_t_menu_alarm **) &rtt_alarm_ctx->menu,
		&rtt_alarmlist_index, 
		rtt_alarmlist_maxindex,
		"", 
		0, 
		0,
		0, pwr_cNObjid, 0, 0, 0, 0, 
		MsgP, RTT_ALARMTYPE_ALARM);
	  if ( EVEN(sts)) return sts;

	  sts = rtt_alarm_item_text( (rtt_t_menu_alarm *) rtt_alarm_ctx->menu,
		rtt_alarmlist_index -1);

	  if ( rtt_alarm_ctx != 0)
	    rtt_alarm_ctx->update_init = 1;
	}
	rtt_alarm_last_message();

	return RTT__SUCCESS;
}

/****************************************************************************
* Name:		rtt_mh_ack_bc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Backcall from mh.
*
**************************************************************************/
static pwr_tStatus rtt_mh_ack_bc( mh_sAck *MsgP)
{
	int			sts;
	ala_uEvent		*EventP;
	int			alarm_item;
	rtt_t_menu_alarm	*menu_ptr;

	EventP = (ala_uEvent *) MsgP; 

	if ( rtt_AlarmAck)
	{
	  /* Insert in event list */
	  sts = rtt_menu_alarm_list_add( 
		(rtt_t_menu_alarm **) &rtt_event_ctx->menu,
		&rtt_eventlist_index, 
		rtt_eventlist_maxindex,
		"", 
		0, 
		0,
		0, pwr_cNObjid, 0, 0, 0, 0, 
		(mh_sMessage *)MsgP, RTT_ALARMTYPE_ACK);
	  if ( EVEN(sts)) return sts;

	  sts = rtt_event_item_text( (rtt_t_menu_alarm *) rtt_event_ctx->menu,
		rtt_eventlist_index - 1);

	  if ( rtt_event_ctx != 0)
	    rtt_event_ctx->update_init = 1;
	}

	sts = rtt_alarm_get_index( &MsgP->TargetId, &alarm_item);
	if ( ODD(sts))
	{
	  menu_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;
	  menu_ptr += alarm_item;
          if ( !(menu_ptr->status & mh_mEventStatus_NotRet))
  	  {
	    sts = rtt_menu_item_alarm_delete( 
		(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, alarm_item,
		&rtt_alarmlist_index);
	  }
	  else
	  {
	    menu_ptr->status &= ~mh_mEventStatus_NotAck;
	    sts = rtt_alarm_item_text( 
		(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, alarm_item );
	  }
	}
	if ( rtt_alarm_ctx != 0)
	  rtt_alarm_ctx->update_init = 1;
	rtt_alarm_last_message();

	return RTT__SUCCESS;
}

/****************************************************************************
* Name:		rtt_mh_return_bc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Backcall from mh.
*
**************************************************************************/
static pwr_tStatus rtt_mh_return_bc( mh_sReturn *MsgP)
{
	int		sts;
	ala_uEvent	*EventP;
	int		alarm_item;
	rtt_t_menu_alarm	*menu_ptr;

	EventP = (ala_uEvent *) MsgP ; 

	if ( rtt_AlarmReturn)
	{
	  /* Insert in event list */
	  sts = rtt_menu_alarm_list_add( 
		(rtt_t_menu_alarm **) &rtt_event_ctx->menu,
		&rtt_eventlist_index, 
		rtt_eventlist_maxindex,
		"", 
		0, 
		0,
		0, pwr_cNObjid, 0, 0, 0, 0, 
		(mh_sMessage *) MsgP, RTT_ALARMTYPE_RETURN);
	  if ( EVEN(sts)) return sts;

	  sts = rtt_event_item_text( (rtt_t_menu_alarm *) rtt_event_ctx->menu,
		rtt_eventlist_index -1);

	  if ( rtt_event_ctx != 0)
	    rtt_event_ctx->update_init = 1;
	}

	sts = rtt_alarm_get_index( &MsgP->TargetId, &alarm_item);
	if ( ODD(sts))
	{
	  menu_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;
	  menu_ptr += alarm_item;
          if ( !(menu_ptr->status & mh_mEventStatus_NotAck))
  	  {
	    sts = rtt_menu_item_alarm_delete( 
		(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, alarm_item, 
		&rtt_alarmlist_index);
	  }
	  else
	  {
	    menu_ptr->status &= ~mh_mEventStatus_NotRet;
	    /* Remove the returned marks in the item text */
	    sts = rtt_alarm_item_text( 
		(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, alarm_item );
	  }
	  if ( rtt_alarm_ctx != 0)
	    rtt_alarm_ctx->update_init = 1;
	}
	rtt_alarm_last_message();

	return RTT__SUCCESS;
}


/****************************************************************************
* Name:		rtt_mh_cancel_bc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Backcall from mh.
*
**************************************************************************/
static pwr_tStatus rtt_mh_cancel_bc( mh_sReturn *MsgP)
{
	int		sts;
	ala_uEvent	*EventP;
	int		alarm_item;
	rtt_t_menu_alarm	*menu_ptr;

	EventP = (ala_uEvent *) MsgP ; 

	sts = rtt_alarm_get_index( &MsgP->TargetId, &alarm_item);
	if ( ODD(sts))
	{
	  menu_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;
	  menu_ptr += alarm_item;
	  sts = rtt_menu_item_alarm_delete( 
		(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, alarm_item, 
		&rtt_alarmlist_index);
	  if ( rtt_alarm_ctx != 0)
	    rtt_alarm_ctx->update_init = 1;
	}
	rtt_alarm_last_message();

	return RTT__SUCCESS;
}
static pwr_tStatus rtt_mh_block_bc( mh_sBlock *MsgP)
{
        /* NYI */
	return RTT__SUCCESS;
}

static pwr_tStatus rtt_mh_clear_alarmlist_bc( pwr_tNodeIndex nix)
{
         /* NYI */
         return 1;
}

static pwr_tStatus rtt_mh_clear_blocklist_bc( pwr_tNodeIndex nix)
{
         /* NYI */
         return 1;
}


/****************************************************************************
* Name:		rtt_alarm_connect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Connect to alarm handler.
*
**************************************************************************/
int	rtt_alarm_disconnect ()
{
	int	sts;

	if ( !rtt_alarm_connected)
	  return RTT__SUCCESS;

	sts = mh_OutunitDisconnect();
	return sts;
}

/****************************************************************************
* Name:		rtt_alarm_connect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Connect to alarm handler.
*
**************************************************************************/
int	rtt_alarm_connect (
			pwr_tObjid	UserObject,
			int		maxalarm,
			int		maxevent,
			int		acknowledge,
			int		returned,
			int		beep)
{
	int		sts;
 	pwr_tObjid	Objid;
	pwr_tObjid	OutUnit;
	pwr_sClass_User	*userobject_ptr;

#ifdef OS_ELN
	mh_UtilWaitForMh();
	/* Wait for mh has flagged initizated */
	mh_UtilWaitForMh();
#endif

	if ( rtt_alarm_connected != 0)
	  return RTT__SUCCESS;

	if ( cdh_ObjidIsNull( UserObject))
	{
	  if ( cdh_ObjidIsNull( rtt_UserObject))
	    return RTT__OBJNOTFOUND;

	  Objid = rtt_UserObject;
	}
	else
	{
	  /* Userobject is supplied */
	  Objid = UserObject;
	}
	OutUnit = Objid;

	sts = gdh_ObjidToPointer ( Objid, (pwr_tAddress *) &userobject_ptr);
	if ( EVEN(sts)) return RTT__OBJNOTFOUND;

	
	if ( maxevent)
	  rtt_eventlist_maxindex = maxevent;
	else if ( userobject_ptr->MaxNoOfEvents)
	  rtt_eventlist_maxindex = userobject_ptr->MaxNoOfEvents + 1;
	else
	  rtt_eventlist_maxindex = 200;

	if ( maxalarm)
	  rtt_alarmlist_maxindex = maxalarm;
	else if ( userobject_ptr->MaxNoOfAlarms)
	  rtt_alarmlist_maxindex = userobject_ptr->MaxNoOfAlarms + 1;
	else
	  rtt_alarmlist_maxindex = 100;

	rtt_AlarmAck = acknowledge;
	rtt_AlarmReturn = returned;
	rtt_AlarmBeep = beep;
	rtt_message('I', "Loading alarmlist...");
	sts = mh_OutunitConnect(
		OutUnit,
		mh_eOutunitType_Operator,
		0,
		rtt_mh_ack_bc,
		rtt_mh_alarm_bc,
		rtt_mh_block_bc,
		rtt_mh_cancel_bc,
		rtt_mh_clear_alarmlist_bc,
		rtt_mh_clear_blocklist_bc,
		rtt_mh_info_bc,
		rtt_mh_return_bc
		);

	if (EVEN(sts)) return sts;

	/* Allocate memory for the data structure och alarm and event lists */

	sts = rtt_menu_create_ctx( &rtt_alarm_ctx, 0, 0, "RTT ALARM LIST", 
		RTT_MENUTYPE_ALARM);
	if ( EVEN(sts)) return sts;
	rtt_ctx_pop();

	sts = rtt_menu_create_ctx( &rtt_event_ctx, 0, 0, "RTT EVENT LIST",
		RTT_MENUTYPE_ALARM);
	if ( EVEN(sts)) return sts;
	rtt_ctx_pop();

	sts = rtt_menu_alarm_list_add_malloc( 
		(rtt_t_menu_alarm **) &rtt_alarm_ctx->menu, 
		rtt_alarmlist_maxindex);
	if ( EVEN(sts)) return sts;
	sts = rtt_menu_alarm_list_add_malloc( 
		(rtt_t_menu_alarm **) &rtt_event_ctx->menu, 
		rtt_eventlist_maxindex);
	if ( EVEN(sts)) return sts;

	rtt_alarm_connected = 1;

	sts = rtt_alarm_update( 0);

	return RTT__SUCCESS;
}

/****************************************************************************
* Name:		rtt_alarm_update()
*
* Type		int
*
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Connect to alarm handler.
*
**************************************************************************/
int	rtt_alarm_update ( 
			menu_ctx	ctx)
{
	int		sts;
  	static int 	beep_cnt = 0;

	if ( !rtt_alarm_connected)
	  return RTT__SUCCESS;

	sts = mh_OutunitReceive();     
	while (ODD(sts))
	{
	  beep_cnt = 0;
          sts = mh_OutunitReceive();     
	}
      
	if ( ctx != 0)
	  if ( ctx->update_init == 1)
	  {
	    /* Redraw the event or alarm list */
	    rtt_menu_alarm_configure( ctx, 1);
	    rtt_menu_alarm_draw( ctx, 1);
	    rtt_menu_select( ctx);
	    ctx->update_init = 0;
	  }

	if ( rtt_AlarmBeep)
	{
	  if ( !beep_cnt)
	    rtt_alarm_beep();

	  if ( beep_cnt > rtt_AlarmBeep)
	    beep_cnt = 0;
	  else
	    beep_cnt++;

	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_alarm_configure()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Configures a menu.
*	Calculates number of pages, rows, columns etc.
*
**************************************************************************/

static int	rtt_menu_alarm_configure(
			menu_ctx	ctx,
			int		reconfigure)
{
	rtt_t_menu_alarm	*menu_ptr;
	int			item_maxsize;
	int			item_size;

	/* Calculate no of items */
	menu_ptr = (rtt_t_menu_alarm *) ctx->menu;
	ctx->no_items = 0;
	item_maxsize = 0;
	if ( menu_ptr == 0)
	  return RTT__SUCCESS;

	while ( menu_ptr->text[0] != '\0' )
	{
	  ctx->no_items++;
	  item_size = strlen( menu_ptr->text);
	  if ( item_size > item_maxsize)
	    item_maxsize = item_size; 
	  menu_ptr++;
	}

	ctx->max_item_size = 80; /*item_maxsize;*/

	/* Calculate rows and columns and pages */
	ctx->page_len = RTT_MENU_MAXROWS;
	ctx->no_pages = (ctx->no_items - 1) / ctx->page_len + 1;
	ctx->cols = ctx->page_len / RTT_MENU_MAXROWS;
	ctx->row_size = 1;	
	ctx->rows = (ctx->page_len - 1)/ ctx->cols + 1;
	ctx->col_size = (RTT_MENU_MAXCOLS / ctx->cols + ctx->max_item_size + 2)/2;
	ctx->left_margin = 0;
	ctx->up_margin = 
		(RTT_MENU_MAXROWS - ctx->rows ) / 2;

	if ( !reconfigure)
	{
	  ctx->current_page = 0;
	  ctx->current_item = 0;
	  ctx->current_row = 0;
	  ctx->current_col = 0;
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_alarm_get_previous_page()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign previous page to be current page.
*
**************************************************************************/

static int	rtt_alarm_get_previous_page( menu_ctx	ctx)
{
	
	if ( ctx->current_page == 0)
	  ctx->current_page = ctx->no_pages - 1;
	else
	  ctx->current_page--;

	ctx->current_item = ctx->current_page * ctx->page_len;
	return RTT__SUCCESS;
}
/*************************************************************************
*
* Name:		rtt_alarm_get_next_page()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
*
* Description:
*	Assign next page to be current page.
*
**************************************************************************/

static int	rtt_alarm_get_next_page( menu_ctx	ctx)
{
	
	if ( ctx->current_page == (ctx->no_pages - 1))
	  ctx->current_page = 0;
	else
	  ctx->current_page++;

	ctx->current_item = ctx->current_page * ctx->page_len;
	return 1;
}


/*************************************************************************
*
* Name:		rtt_menu_alarm_new()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	parent_ctx	I	parents rtt context.
* rtt_t_menu_alarm **menu_p	I	menu list.
* char		*title		I	menu title.
* unsigned long	userdata	I	...
* unsigned long	flag		I	menu type
*
* Description:
*	Create a new alarm menu.
*
**************************************************************************/

int	rtt_menu_alarm_new(
			menu_ctx		parent_ctx,
			menu_ctx		ctx)
{
	unsigned long	terminator;
	unsigned long	option;
	char		input_str[80];
	int		maxlen = 30;
	rtt_t_menu_alarm	*menu_ptr;
	int		sts;
	char		*s;
	char		eventobject[80];
	pwr_tObjid	objid;
	

	rtt_ctx_push( ctx);
	ctx->menutype = RTT_MENUTYPE_ALARM;
	ctx->parent_ctx = parent_ctx;

	rtt_menu_alarm_configure( ctx, 0);
	rtt_menu_alarm_draw( ctx, 0);
	rtt_menu_select( ctx);
	menu_ptr = (rtt_t_menu_alarm *) ctx->menu;

	option = RTT_OPT_NORECALL | RTT_OPT_NOEDIT | RTT_OPT_NOECHO | 
		RTT_OPT_TIMEOUT;

	while (1)
	{
	  rtt_command_get_input_string( (char *) &rtt_chn, input_str, 
		&terminator, maxlen, 
		rtt_recallbuff, option, rtt_scantime, &rtt_scan, 
		ctx, 
		NULL, RTT_COMMAND_PICTURE);
	  rtt_message('S',"");

	  switch ( terminator)
	  {
	    case RTT_K_ARROW_UP:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_up( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_ARROW_DOWN:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_down( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_ARROW_LEFT:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_left( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_ARROW_RIGHT:
	      rtt_menu_unselect( ctx);
	      rtt_get_next_item_right( ctx);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_NEXTPAGE:
	      /* Next page */
	      sts = rtt_alarm_get_next_page( ctx);
	      if ( ODD(sts))
	      {
	        rtt_menu_alarm_draw( ctx, 0);
	        rtt_menu_select( ctx);
	      }
	      break;
	    case RTT_K_PREVPAGE:
	      /* Previous page */
	      sts = rtt_alarm_get_previous_page( ctx);	
	      if ( ODD(sts))
	      {
	        rtt_menu_alarm_draw( ctx, 0);
	        rtt_menu_select( ctx);
	      }
	      break;
	    case RTT_K_RETURN:
	      if ( (menu_ptr + ctx->current_item)->func != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func) ( ctx,
			(menu_ptr + ctx->current_item)->argoi,
			(menu_ptr + ctx->current_item)->arg1,
			(menu_ptr + ctx->current_item)->arg2,
			(menu_ptr + ctx->current_item)->arg3,
			(menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_alarm_draw( ctx, 0);
	          rtt_menu_select( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF1:
	      /* Show the objectname of the element */
	      strcpy ( eventobject, (menu_ptr + ctx->current_item)->eventname);
	      s = strchr( eventobject, '.');
	      if ( s != 0)
	        *s = 0;

	      sts = gdh_NameToObjid( eventobject, &objid);
	      if ( ODD(sts))	
	      {
	        sts = rtt_show_obj_hier_class_name( ctx, 0, 0,
			eventobject, 1, 0);
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	           else
	           {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_alarm_draw( ctx, 0);
	          rtt_menu_select( ctx);
	        }
	       }
	      else
	      {
	        /* Message the eventname */
	        strcpy( eventobject, "Text: ");
	        strncat ( eventobject, 
			(menu_ptr + ctx->current_item)->eventname,
			sizeof(menu_ptr->eventname) - strlen(eventobject) -1);
	         eventobject[ sizeof(eventobject) -1] = 0;
	         rtt_message('I', eventobject);
	      }
	      break;
	    case RTT_K_PF2:
	      if ( (menu_ptr + ctx->current_item)->func3 != NULL)
	      {
	        sts = ((menu_ptr + ctx->current_item)->func3) ( ctx,
			(menu_ptr + ctx->current_item)->argoi,
			(menu_ptr + ctx->current_item)->arg1,
			(menu_ptr + ctx->current_item)->arg2,
			(menu_ptr + ctx->current_item)->arg3,
			(menu_ptr + ctx->current_item)->arg4);
	        if ( EVEN(sts)) return sts;
	        if ( sts == RTT__FASTBACK)
	        {
	          if ( ctx->parent_ctx != 0)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__FASTBACK;
	          }
	          else
	          {
	            ctx->current_page = 0;
	            ctx->current_item = 0;
	          }
	        }
	        if ( sts == RTT__BACKTOCOLLECT)
	        {
	          if ( ctx != rtt_collectionmenuctx)
	          {	
	            rtt_menu_delete(ctx);
	            return RTT__BACKTOCOLLECT;
	          }
	        }
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_alarm_draw( ctx, 0);
	          rtt_menu_select( ctx);
	        }
	      }
	      else 
	        rtt_message('E', "Function not defined");
	      break;
	    case RTT_K_PF3:
	      rtt_alarm_ack( ctx);
	      /* Reconfigure the menu */
	      rtt_menu_alarm_configure( ctx, 0);
	      rtt_menu_alarm_draw( ctx, 0);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_PF4:
	      rtt_menu_delete(ctx);
	      return RTT__SUCCESS;
	    case RTT_K_FAST_1:
	    case RTT_K_FAST_2:
	    case RTT_K_FAST_3:
	    case RTT_K_FAST_4:
	    case RTT_K_FAST_5:
	    case RTT_K_FAST_6:
	    case RTT_K_FAST_7:
	    case RTT_K_FAST_8:
	    case RTT_K_FAST_9:
	    case RTT_K_FAST_10:
	    case RTT_K_FAST_11:
	    case RTT_K_FAST_12:
	    case RTT_K_FAST_13:
	    case RTT_K_FAST_14:
	    case RTT_K_FAST_15:
	    case RTT_K_FAST_16:
	    case RTT_K_FAST_17:
	    case RTT_K_FAST_18:
	    case RTT_K_FAST_19:
	    case RTT_K_FAST_20:
	    case RTT_K_FAST_21:
	    case RTT_K_FAST_22:
	      rtt_fastkey = terminator - RTT_K_FAST;
	      sts = rtt_get_fastkey_type();
	      if ( sts == RTT__NOPICTURE)
	      {
	        sts = rtt_get_fastkey_picture( ctx);
	        if ( EVEN(sts)) return sts;
	        break;
	      }
	    case RTT_K_CTRLZ:
	      rtt_menu_delete(ctx);
	      return RTT__FASTBACK;
	    case RTT_K_CTRLW:
	      rtt_menu_alarm_draw( ctx, 0);
	      rtt_menu_select( ctx);
	      break;
	    case RTT_K_CTRLK:
	      /* Acknowledge last alarm */
	      sts = rtt_alarm_ack_last();
	      break;
	    case RTT_K_CTRLL:
	      /* Change description mode */
	      if ( !rtt_description_on)
	        sts = rtt_cli( rtt_command_table, "SET DESCRIPTION", (void *) ctx, 0);
	      else
	        sts = rtt_cli( rtt_command_table, "SET NODESCRIPTION", (void *) ctx, 0);
	      break;
	    case RTT_K_CTRLV:
	      /* Insert current item in collection picture */
	      sts = rtt_collect_insert( ctx, NULL);
	      break;
	    case RTT_K_CTRLN:
	      /* Show collection picture */
	      sts = rtt_collect_show( ctx);
	      if ( EVEN(sts)) return sts;
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      else
	      {
	        if ( sts != RTT__NOPICTURE)
	        {
	          rtt_menu_alarm_draw( ctx, 0);
	          rtt_menu_select( ctx);
	        } 
	      }
	      break;
	    case RTT_K_DELETE:
	      break;
	    case RTT_K_COMMAND:
	      sts = rtt_get_command( ctx, (char *) &rtt_chn, rtt_recallbuff, 
			rtt_scantime, &rtt_scan, ctx, 
	    		"pwr_rtt> ", 0, RTT_ROW_COMMAND, rtt_command_table);
	      /* menu_ptr might have been changed */
	      if ( EVEN(sts)) return sts;
	      menu_ptr = (rtt_t_menu_alarm *) ctx->menu;
	      if ( sts == RTT__FASTBACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__FASTBACK;
	        }
	        else
	        {
	          ctx->current_page = 0;
	          ctx->current_item = 0;
	        }
	      }
	      if ( sts == RTT__BACK)
	      {
	        if ( ctx->parent_ctx != 0)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__SUCCESS;
	        }
	      }
	      if ( sts == RTT__BACKTOCOLLECT)
	      {
	        if ( ctx != rtt_collectionmenuctx)
	        {	
	          rtt_menu_delete(ctx);
	          return RTT__BACKTOCOLLECT;
	        }
	      }
	      if ( sts != RTT__NOPICTURE)
	      {
	        rtt_menu_alarm_draw( ctx, 0);
	        rtt_menu_select( ctx);
	      } 
	      break;
	    case RTT_K_HELP:
	      /* Try to find help in application help */
	      sts = rtt_help( ctx, ctx->title, (rtt_t_helptext *)rtt_appl_helptext);
	      if ( sts == RTT__NOHELPSUBJ)
	        /* Not found, show the 'object menu' help */
	        rtt_help( ctx, "OBJECT MENU", (rtt_t_helptext *)rtt_command_helptext);

	      rtt_menu_alarm_draw( ctx, 0);
	      rtt_menu_select( ctx);
	      break;
	  }
	}

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_alarm_list_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	**menulist	I	menulist.
* int		*index		I	index in menulist
* char		*text		I	menu text.
* int		(* func) ()	I	function to be called at RETURN.
* int		(* func2) ()	I	function to be called at PF1
* int		(* func3) ()	I	function to be called at PF2
* pwr_tObjid	argoi		I	argument passed to the functions
* void		*arg1		I	argument passed to the functions
* void		*arg2		I	argument passed to the functions
* void		*arg3		I	argument passed to the functions
* void		*arg4		I	argument passed to the functions
*
* Description:
*	Adds an item to an alarm menu list.
*
**************************************************************************/

static int	rtt_menu_alarm_list_add(
			rtt_t_menu_alarm	**menulist,
			int		*index,
			int		maxindex,
			char		*text,
			int		(* func) (),
			int		(* func2) (),
			int		(* func3) (),
			pwr_tObjid	argoi,
			void		*arg1,
			void		*arg2,
			void		*arg3,
			void		*arg4,
			mh_sMessage 	*MsgP,
			int		type)
{
	rtt_t_menu_alarm	*menu_ptr;
	ala_uEvent		*EventP;
	int			sts;

	EventP = (ala_uEvent *) MsgP ; 

	if ( maxindex == 0)
	  return RTT__SUCCESS;

	if ( *index >= maxindex) 
	{
	  /* Delete the first item first */
	    sts = rtt_menu_item_alarm_delete( *menulist, 0, index);
	}

	menu_ptr = *menulist + *index;
	strncpy( menu_ptr->text, text, sizeof(menu_ptr->text));
	menu_ptr->func = func;
	menu_ptr->func2 = func2;
	menu_ptr->func3 = func3;
	menu_ptr->argoi = argoi;
	menu_ptr->arg1 = arg1;
	menu_ptr->arg2 = arg2;
	menu_ptr->arg3 = arg3;
	menu_ptr->arg4 = arg4;
        menu_ptr->time = EventP->Info.EventTime; 
        strncpy( menu_ptr->eventname, EventP->Info.EventName,
			sizeof( menu_ptr->eventname));
        strncpy( menu_ptr->eventtext, EventP->Msg.EventText, 80); 
        menu_ptr->eventflags = EventP->Info.EventFlags;
	menu_ptr->type = type;
	menu_ptr->eventprio = MsgP->Info.EventPrio;
	memcpy ( &menu_ptr->eventid , &EventP->Info.Id,
			sizeof(EventP->Info.Id));
	menu_ptr->object = EventP->Info.Object;
	menu_ptr->status = MsgP->Status;

	(*index)++;
	menu_ptr++;
	strcpy( menu_ptr->text, "");

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_list_add_malloc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* rtt_t_menu	**menulist	I	menulist.
* int		index		I	number of items to allocate memory.
*
* Description:
*	Allocates memory for a menu list.
*	Allocated memory is number of items + 1.
*
**************************************************************************/

static int	rtt_menu_alarm_list_add_malloc(
			rtt_t_menu_alarm	**menulist,
			int			index)
{
	*menulist = (rtt_t_menu_alarm *) calloc( index + 1 , 
			sizeof(rtt_t_menu_alarm));
	if ( *menulist == 0)
	  return RTT__NOMEMORY;

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_alarm_item_delete()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	
* int		item		I	
*
* Description:
*	Delete an item in a alarm menu.
*
**************************************************************************/

static int	rtt_menu_item_alarm_delete(
			rtt_t_menu_alarm	*menu_ptr,
			int			item,
			int			*numberof_items)
{
	char		*menu_charptr;
	char		*menu_charptr_next;
	int		size;
	int		itemsize;

	itemsize = sizeof(rtt_t_menu_alarm);

	menu_charptr = (char *) menu_ptr;
	menu_charptr += item * itemsize;
	menu_charptr_next = (char *) menu_ptr;
	menu_charptr_next += (item + 1) * itemsize;
	size = (*numberof_items - item) * itemsize;

	memcpy( menu_charptr, menu_charptr_next, size);	

	(*numberof_items)--;
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_menu_alarm_draw()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* menu_ctx	ctx		I	menu ctx.
*
* Description:
*	Draw a menu.
*
**************************************************************************/

static int	rtt_menu_alarm_draw(
			menu_ctx	ctx,
			int		noerase)
{
	rtt_t_menu	*menu_ptr;
	int		i;

/*	if ( !noerase)
*/
	rtt_display_erase();
	rtt_menu_draw_title( ctx);
	menu_ptr = ctx->menu;
	for ( i = ctx->current_page * ctx->page_len; 
		(i < ctx->no_items) && ( i < (ctx->current_page + 1) *
		ctx->page_len); i++)
	{
	  rtt_menu_draw_item( ctx, i);
	}
	rtt_message('S',"");
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_menu_alarm_list_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Creates the text in the eventlist.
*
**************************************************************************/

static int	rtt_event_item_text(
			rtt_t_menu_alarm	*menu_ptr,
			int			index)
{
	char			timestr[64];

	menu_ptr += index;

	switch ( menu_ptr->type)
	{
	  case RTT_ALARMTYPE_INFO:
	    strcpy( menu_ptr->text, " I ");
	    break;
	  case RTT_ALARMTYPE_RETURN:
	    strcpy( menu_ptr->text, " r ");
	    break;
	  case RTT_ALARMTYPE_ACK:
	    strcpy( menu_ptr->text, " a ");
	    break;
	  case RTT_ALARMTYPE_ALARM:
	    switch ( menu_ptr->eventprio)
	    {
	      case mh_eEventPrio_A:
	        strcpy( menu_ptr->text, "*A ");
	        break;
	      case mh_eEventPrio_B:
	        strcpy( menu_ptr->text, "*B ");
	        break;
	      case mh_eEventPrio_C:
	        strcpy( menu_ptr->text, " C ");
	        break;
	      case mh_eEventPrio_D:
	        strcpy( menu_ptr->text, " D ");
	        break;
	    }
	    break;
	}
	rtt_timestring( &menu_ptr->time, timestr);
	strcpy( &timestr[7], &timestr[12]);
	timestr[6] = ' ';
	timestr[15] = 0;
	strcat( menu_ptr->text, timestr); 

	strcat( menu_ptr->text, "  ");
	if ( (menu_ptr->type == RTT_ALARMTYPE_INFO) ||
	     (menu_ptr->type == RTT_ALARMTYPE_ALARM))
	  strncat( menu_ptr->text, menu_ptr->eventtext,
		sizeof(menu_ptr->text) - strlen( menu_ptr->text));
	
	menu_ptr->text[ 79 ] = 0;
#if 0
	char			eventname[80];
	char			*s;
	int			i;

	for ( i = strlen( menu_ptr->text); i < 71; i++)
	  menu_ptr->text[i] = ' ';
	menu_ptr->text[i] = 0;
	strcpy( eventname, menu_ptr->eventname);
	s = strchr( eventname, '.');
	if ( s != 0)
	  *s = 0;
	s = strchr( eventname, '-');
	while ( s != 0)
	{
	  strcpy( eventname, s + 1);			
	  s = strchr( eventname, '-');
	}
	strncat( menu_ptr->text, eventname, 
		sizeof(menu_ptr->text) - strlen( menu_ptr->text));
	menu_ptr->text[ sizeof(menu_ptr->text) -1 ] = 0;
#endif	
	/* If rtt alarm log is activ, log event on file */
	rtt_alarmlog_log( "Event", menu_ptr);

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_alarm_item_text()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Creates the text in the alarmlist.
*
**************************************************************************/

int	rtt_alarm_item_text(
			rtt_t_menu_alarm	*menu_ptr,
			int			index)
{
	char			timestr[64];

	menu_ptr += index;

        if (menu_ptr->status & mh_mEventStatus_NotAck) 
	  strcpy( menu_ptr->text, "!");
	else
	  strcpy( menu_ptr->text, " ");
        if (menu_ptr->status & mh_mEventStatus_NotRet) 
	  strcat( menu_ptr->text, "* ");
	else
	  strcat( menu_ptr->text, "  ");

	switch ( menu_ptr->type)
	{
	  case RTT_ALARMTYPE_INFO:
	    strcat( menu_ptr->text, "I ");
	    break;
	  case RTT_ALARMTYPE_ALARM:
	    switch ( menu_ptr->eventprio)
	    {
	      case mh_eEventPrio_A:
	        strcat( menu_ptr->text, "A ");
	        break;
	      case mh_eEventPrio_B:
	        strcat( menu_ptr->text, "B ");
	        break;
	      case mh_eEventPrio_C:
	        strcat( menu_ptr->text, "C ");
	        break;
	      case mh_eEventPrio_D:
	        strcat( menu_ptr->text, "D ");
	        break;
	    }
	    break;
	}
	rtt_timestring( &menu_ptr->time, timestr);
	strcpy( &timestr[7], &timestr[12]);
	timestr[6] = ' ';
	timestr[15] = 0;
	strcat( menu_ptr->text, timestr); 

	strcat( menu_ptr->text, "  ");
	strncat( menu_ptr->text, menu_ptr->eventtext,
		sizeof(menu_ptr->text) - strlen( menu_ptr->text));
	
	menu_ptr->text[ 79 ] = 0;
#if 0
	char			eventname[80];
	char			*s;
	int			i;

	/*  Skip event name ... */
	for ( i = strlen( menu_ptr->text); i < 71; i++)
	  menu_ptr->text[i] = ' ';
	menu_ptr->text[i] = 0;
	strcpy( eventname, menu_ptr->eventname);
	s = strchr( eventname, '.');
	if ( s != 0)
	  *s = 0;
	s = strchr( eventname, '-');
	while ( s != 0)
	{
	  strcpy( eventname, s + 1);			
	  s = strchr( eventname, '-');
	}
	strncat( menu_ptr->text, eventname, 
		sizeof(menu_ptr->text) - strlen( menu_ptr->text));
	menu_ptr->text[ sizeof(menu_ptr->text) -1 ] = 0;
#endif	
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_event_print_text()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Creates the text in the file at a print command.
*
**************************************************************************/

static int	rtt_event_print_text(
			rtt_t_menu_alarm	*menu_ptr,
			int			index,
			char			*text,
			int			size,
			int			notext,
			int			noname)
{
	int			i;
	char			timestr[64];

	menu_ptr += index;

	switch ( menu_ptr->type)
	{
	  case RTT_ALARMTYPE_INFO:
	    strcpy( text, " I ");
	    break;
	  case RTT_ALARMTYPE_RETURN:
	    strcpy( text, " r ");
	    break;
	  case RTT_ALARMTYPE_ACK:
	    strcpy( text, " a ");
	    break;
	  case RTT_ALARMTYPE_ALARM:
	    switch ( menu_ptr->eventprio)
	    {
	      case mh_eEventPrio_A:
	        strcpy( text, "*A ");
	        break;
	      case mh_eEventPrio_B:
	        strcpy( text, "*B ");
	        break;
	      case mh_eEventPrio_C:
	        strcpy( text, " C ");
	        break;
	      case mh_eEventPrio_D:
	        strcpy( text, " D ");
	        break;
	    }
	    break;
	}
	rtt_timestring( &menu_ptr->time, timestr);
	strcpy( &timestr[7], &timestr[12]);
	timestr[6] = ' ';
	timestr[15] = 0;
	strcat( text, timestr); 

	strcat( text, "  ");

	if ( !notext)
	{
	  if ( (menu_ptr->type == RTT_ALARMTYPE_INFO) ||
	     	(menu_ptr->type == RTT_ALARMTYPE_ALARM))
	    strncat( text, menu_ptr->eventtext, size - strlen(text));
	
	  *(text + 78) = 0;
	  for ( i = strlen( text); i < 80; i++)
	    *(text + i) = ' ';
	  *(text + i) = 0;
	}
	if ( !noname)
	{
	  strncat( text, menu_ptr->eventname, size - strlen(text));
	}	
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_event_print()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Prints the event list on a file.
*
**************************************************************************/

int	rtt_event_print(
			char		*filename,
			int		notext,
			int		noname)
{
	char			text[200];
	FILE			*file;
	int			i;
	
	if ( rtt_event_ctx == 0)
	{
	  rtt_message('E', "Eventlist not loaded");
	  return RTT__NOPICTURE;
	}

	file = fopen( filename, "w");
	if ( file == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__HOLDCOMMAND;
	}
	
	fprintf( file, "	EVENT LIST\n\n");

	for ( i = 0; i < rtt_eventlist_index; i++)
	{
	  rtt_event_print_text( (rtt_t_menu_alarm *) rtt_event_ctx->menu, 
		i, text, sizeof( text), 
		notext, noname);
	  fprintf( file, "%s\n", text);
	}
	fclose( file);
	
	rtt_message('I', "Eventlist printed");
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_alarm_get_index()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the index in alarmlist from eventid.
*
**************************************************************************/

static int	rtt_alarm_get_index( 
		mh_sEventId	*id,
		int 		*alarm_index)
{
	rtt_t_menu_alarm	*menu_ptr;
	int			i;

	menu_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;

	for ( i = 0; i < rtt_alarmlist_index; i++) 
	{
	  if ( !memcmp( id, &menu_ptr->eventid, sizeof( *id)))
	  {
	    *alarm_index = i;
	    return RTT__SUCCESS;
	  }
	  menu_ptr++;
	}
	return 0;
}

/*************************************************************************
*
* Name:		rtt_alarm_ack()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the index in alarmlist from eventid.
*
**************************************************************************/

int	rtt_alarm_ack(
			menu_ctx 	ctx)
{
	rtt_t_menu_alarm	*menu_ptr;
	rtt_t_menu_alarm	*menustart_ptr;
	int			i;
	int			sts;

	if ( ctx != rtt_alarm_ctx) 
	  return RTT__SUCCESS;

	menustart_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;

	for ( i = 0; i < rtt_alarmlist_index; i++) 
	{
	  menu_ptr = menustart_ptr + i;
          if (menu_ptr->status & mh_mEventStatus_NotAck) 
	  {
	    menu_ptr->status &= ~mh_mEventStatus_NotAck;

	    /* Ack should maybe be sent to mh here... */
	    mh_OutunitAck( &menu_ptr->eventid);

	    /* If rtt alarm log is activ, log event on file */
	    rtt_alarmlog_log( "Ack   ", menu_ptr);

            if (menu_ptr->status & mh_mEventStatus_NotRet) 
	      /* Remove the not ack marks in the item text */
	      sts = rtt_alarm_item_text( 
			(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, i);
	    else
	    {
	      /* Detete the item from the alarm list */
	      rtt_menu_item_alarm_delete( 
			(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, 
			i, &rtt_alarmlist_index);
	      i--;
	    }

	  }
	}
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_alarm_ack()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the index in alarmlist from eventid.
*
**************************************************************************/

int	rtt_alarm_ack_last()
{
	rtt_t_menu_alarm	*menu_ptr;
	rtt_t_menu_alarm	*menustart_ptr;
	int			i;
	int			sts;

	if ( !rtt_alarm_connected)
	  return RTT__SUCCESS;

	menustart_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;

	for ( i = rtt_alarmlist_index - 1; i >= 0; i--) 
	{
	  menu_ptr = menustart_ptr + i;
          if (menu_ptr->status & mh_mEventStatus_NotAck) 
	  {
	    menu_ptr->status &= ~mh_mEventStatus_NotAck;

	    /* Ack should maybe be sent to mh here... */
	    mh_OutunitAck( &menu_ptr->eventid);

	    /* If rtt alarm log is activ, log event on file */
	    rtt_alarmlog_log( "Ack   ", menu_ptr);

            if (menu_ptr->status & mh_mEventStatus_NotRet) 
	      /* Remove the not ack marks in the item text */
	      sts = rtt_alarm_item_text( 
			(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, i);
	    else
	    {
	      /* Detete the item from the alarm list */
	      rtt_menu_item_alarm_delete( 
			(rtt_t_menu_alarm *) rtt_alarm_ctx->menu, 
			i, &rtt_alarmlist_index);
	      i--;
	    }
	    rtt_alarm_last_message();
	    break;
	  }
	}
	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_timestring()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Get the index in alarmlist from eventid.
*
**************************************************************************/

static int	rtt_timestring(
			pwr_tTime	*time,
			char		*timestr)
{
	char			timstr[64];
	int			sts;

	sts = time_AtoAscii( time, time_eFormat_DateAndTime, timstr, 
		sizeof(timstr));
	strcpy( timestr, timstr);

	return RTT__SUCCESS;
}


/*************************************************************************
*
* Name:		rtt_alarm_beep()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Beep it there is a unacked alarm in alarmlist.
*
**************************************************************************/

static int	rtt_alarm_beep()
{
	rtt_t_menu_alarm	*menu_ptr;
	int			i;

	menu_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;
	for ( i = 0; i < rtt_alarmlist_index; i++)
	{
          if (menu_ptr->status & mh_mEventStatus_NotAck) 
	  {
	    BEEP;
	    return RTT__SUCCESS;	    
	  }
	  menu_ptr++;
	}

	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_alarm_beep()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Print in rtt_AlarmLastMessage if there is a unacked alarm in alarmlis.
*
**************************************************************************/

static int	rtt_alarm_last_message()
{
	rtt_t_menu_alarm	*menu_ptr;
	rtt_t_menu_alarm	*last_menu_ptr;
	int			i;
	int			not_returned_alarms;
	char			alarm_count_str[10];

	if ( !rtt_alarm_connected)
	  return RTT__SUCCESS;

	/* Count not returned alarms and detect last not acked alarm */
	menu_ptr = (rtt_t_menu_alarm *) rtt_alarm_ctx->menu;
	last_menu_ptr = 0;
	not_returned_alarms = 0;
	rtt_AlarmText1[0] = 0;
	rtt_AlarmText2[0] = 0;
	rtt_AlarmText3[0] = 0;
	rtt_AlarmText4[0] = 0;
	rtt_AlarmText5[0] = 0;
	for ( i = 0; i < rtt_alarmlist_index ; i++)
	{
	  if ( i == rtt_alarmlist_index - 1)
	    strcpy ( rtt_AlarmText1, &menu_ptr->text[24]);
	  else if ( i == rtt_alarmlist_index - 2)
	    strcpy ( rtt_AlarmText2, &menu_ptr->text[24]);
	  else if ( i == rtt_alarmlist_index - 3)
	    strcpy ( rtt_AlarmText3, &menu_ptr->text[24]);
	  else if ( i == rtt_alarmlist_index - 4)
	    strcpy ( rtt_AlarmText4, &menu_ptr->text[24]);
	  else if ( i == rtt_alarmlist_index - 5)
	    strcpy ( rtt_AlarmText5, &menu_ptr->text[24]);
          if (menu_ptr->status & mh_mEventStatus_NotAck)
	    last_menu_ptr = menu_ptr;
	  if (menu_ptr->status & mh_mEventStatus_NotRet)
	    not_returned_alarms++;
	  menu_ptr++;
	}
	if ( last_menu_ptr)
	{
	  strncpy( rtt_AlarmLastMessage, &last_menu_ptr->text[2], 
		sizeof(rtt_AlarmLastMessage) - 2);
	  rtt_AlarmLastMessage[79] = 0;
	  sprintf( alarm_count_str, "%3d:%3d", 
		not_returned_alarms, rtt_alarmlist_index);
	  if ( strlen(rtt_AlarmLastMessage) < 72)
	  {
	    for ( i = 0; i < 73; i++)
	      strcat( rtt_AlarmLastMessage, " ");
	    strcpy( &rtt_AlarmLastMessage[72], alarm_count_str);
	  }
	}
	else
	{
	  strcpy( rtt_AlarmLastMessage, 
  "                                                                      ");
	  sprintf( alarm_count_str, "%3d:%3d", 
		rtt_alarmlist_index - not_returned_alarms, rtt_alarmlist_index);
	  strcpy( &rtt_AlarmLastMessage[72], alarm_count_str);
	}
	rtt_message('S', "");
	r_print_buffer();
	return RTT__SUCCESS;
}

/*************************************************************************
*
* Name:		rtt_alarmlog_start()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Start the rtt alarm log.
*
**************************************************************************/

int	rtt_alarmlog_start( char *filename)
{
	pwr_tTime	time;
	char		timestr[80];

	if ( rtt_alarmlog_active)
	{
	  rtt_message('E',"Alarm log is already started");
	  return RTT__NOPICTURE;
	}
	
	rtt_alarmlog_file = fopen( filename, "w");
	if ( rtt_alarmlog_file == 0)
	{
	  rtt_message('E', "Unable to open file");
	  return RTT__HOLDCOMMAND;
	}

	time_GetTime( &time);
	time_AtoAscii( &time, time_eFormat_DateAndTime, timestr, 
			sizeof(timestr));
	fprintf( rtt_alarmlog_file, "Rtt event log list started at %s\n\n", 
			timestr);

	rtt_alarmlog_active = 1;
	rtt_message('I',"Alarm log started");
	return RTT__NOPICTURE;
}
/*************************************************************************
*
* Name:		rtt_alarmlog_stop()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Stop the rtt alarm log.
*
**************************************************************************/

int	rtt_alarmlog_stop()
{
	pwr_tTime	time;
	char		timestr[80];

	if ( !rtt_alarmlog_active)
	{
	  rtt_message('E',"Alarm log is not started");
	  return RTT__NOPICTURE;
	}	
	
	
	time_GetTime( &time);
	time_AtoAscii( &time, time_eFormat_DateAndTime, timestr, 
			sizeof(timestr));
	fprintf( rtt_alarmlog_file, "\nRtt event log list stopped at %s\n", 
			timestr);
	fclose( rtt_alarmlog_file);

	rtt_alarmlog_active = 0;
	rtt_message('I',"Alarm log stopped");
	return RTT__NOPICTURE;
}

/*************************************************************************
*
* Name:		rtt_alarmlog_stop()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Log event on rtt alarmlog.
*
**************************************************************************/

static int	rtt_alarmlog_log( char *type, rtt_t_menu_alarm	*menu_ptr)
{
	if ( !rtt_alarmlog_active)
	  return RTT__SUCCESS;
	
	fprintf( rtt_alarmlog_file, "%s  %s\n", type, menu_ptr->text);
	
	return RTT__SUCCESS;
}
