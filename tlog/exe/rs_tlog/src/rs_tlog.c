/* 
 * Proview   $Id: rs_tlog.c,v 1.1 2006-01-10 14:38:36 claes Exp $
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

/************************************************************************
*
* Filename:		rs_tlog.c
*			Date	Pgm.	Read.	Remark
* Modified		940104	CS		Initial creation
*
* Description:
*	Test Logging Handler.
*
**************************************************************************/


/*_Include files_________________________________________________________*/
#ifdef OS_VMS
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <descrip.h>
#include <string.h>
#include <starlet.h>
#include <lib$routines.h>
#endif

#if defined  OS_LYNX || defined OS_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#endif

#ifdef OS_ELN
#include stdio
#include stdlib
#include signal
#include descrip
#include string
#include starlet
#include lib$routines
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "rt_errh.h"
#include "pwr_tlogclasses.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#ifndef rt_mh_h
#include "rt_mh.h"
#endif
#ifndef rt_mh_outunit_h
#include "rt_mh_outunit.h"
#endif
#include "rt_mh_msg.h"
#include "rt_mh_appl.h"
#include "rs_tlog_msg.h"
#include "co_time.h"

/* Nice functions */
#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif

#define	LogAndExit( status) \
{\
  errh_CErrLog(status, NULL);\
  exit( status);\
}

typedef union alau_Event ala_uEvent;
union alau_Event 
{
    mh_sMsgInfo Info;
    mh_sAck	Ack;
    mh_sMessage Msg;
    mh_sReturn  Return;
};


#define TLOG_ALLOC	50

typedef struct {
	pwr_tObjid 	objid;
	char		name[80];
	char		*object_ptr;
	gdh_tDlid	subid;
	} tlog_t_objectlist;

enum tlog_ee_logtype
{
	tlog_logtype_DtLogUp,
	tlog_logtype_DtLogDown,
	tlog_logtype_AtLogUp,
	tlog_logtype_AtLogDown,
	tlog_logtype_Info,
	tlog_logtype_AlarmA,
	tlog_logtype_AlarmB,
	tlog_logtype_AlarmC,
	tlog_logtype_AlarmD,
	tlog_logtype_Modify,
	tlog_logtype_UnModify,
	tlog_logtype_Close,
	tlog_logtype_CAtLog,
	tlog_logtype_CDtLog,
	tlog_logtype_COtLog,
	tlog_logtype_ExecOn,
	tlog_logtype_ExecOff,
	tlog_logtype_ChgAtLog,
	tlog_logtype_ChgOtLog
};
typedef enum tlog_ee_logtype tlog_e_logtype;

typedef struct {
	pwr_tObjid     	objid;
	tlog_e_logtype	logtype;
	pwr_tString80	logtext;
	pwr_tAName	attrname;
	pwr_tTime	logtime;
	pwr_tDeltaTime	diftime;
	} tlog_t_loglist;

typedef struct {
	FILE			*outfile;
	pwr_tObjid		objid;
	int			listindex;
	int			extselectlist;
	pwr_tTime		open_time;
	pwr_sClass_TLogOpen	*object_ptr;
	pwr_tOName		TLogSelectList[10];
	} tlog_t_filectx;

typedef struct {
	pwr_tObjid			objid;
	int			deleted;
	int			check;
	pwr_tOName		TLogSelectList[10];
	pwr_tOName		AlarmSelectList[10];
	} tlog_t_selectlist;

/*** Local variables *****************************************************/

static	pwr_sClass_TLogConfigure *tlog_TLogConf;
static	tlog_t_loglist		*log_list;
static	int			log_count;
static	int			log_alloc;
static	tlog_t_filectx		filectx;
static	tlog_t_selectlist	selectlists[20];
static	int			selectlist_count;
static	int			selectlist_current;
static 	int			log = 0;



static pwr_tUInt32 tlog_sleep( float time);
static pwr_tUInt32 tlog_exec_off_all( pwr_tObjid objid);
static pwr_tUInt32 tlog_exec_on_all( pwr_tObjid objid);
static pwr_tUInt32 tlog_file_close( tlog_t_filectx *filectx);
static pwr_tUInt32 tlog_extselectlist_reset( tlog_t_filectx *filectx);
static pwr_tUInt32 tlog_selectlist_modify( tlog_t_filectx *filectx,
			pwr_tObjid	objid);
static pwr_tUInt32 tlog_extselectlist_unmodify( tlog_t_filectx *filectx,
			pwr_tObjid	objid);
static pwr_tUInt32 tlog_selectlist_check( tlog_t_filectx *filectx,
			pwr_tObjid	objid);
static pwr_tUInt32 tlog_extselectlist_check( tlog_t_filectx *filectx,
			pwr_tObjid	objid);
static pwr_tUInt32 tlog_extselectlist_modify( tlog_t_filectx *filectx,
			pwr_tObjid	objid,
			pwr_sClass_User *UserObject,
			pwr_tObjid *tlog_selectlist,
			pwr_tObjid *alarm_selectlist);
static pwr_tUInt32 tlog_selectlist_unmodify( tlog_t_filectx *filectx,
			pwr_tObjid	objid,
			pwr_sClass_User *UserObject);
static pwr_tStatus tlog_mh_info_bc( mh_sMessage *MsgP);
static pwr_tStatus tlog_mh_alarm_bc( mh_sMessage *MsgP);
static pwr_tUInt32 tlog_alarm_update () ;
static pwr_tUInt32 tlog_alarm_connect( 	pwr_tObjid	user_object);
static pwr_tUInt32 tlog_loglist_sort(
		tlog_t_loglist		*loglist,
		int			loglist_count);
static pwr_tUInt32 tlog_loglist_print( 
		tlog_t_filectx 		*filectx,
		tlog_t_loglist		*loglist,
		int			*loglist_count,
		pwr_sClass_User 	*UserObject);
static pwr_tUInt32 tlog_loglist_add( 
		tlog_t_filectx 		*filectx,
		pwr_tObjid		objid,
		tlog_e_logtype		logtype,
		pwr_tString80		*logtext,
		pwr_sAttrRef		*attribute,
		pwr_tString80		*attrname,
		pwr_tTime		*logtime,
		tlog_t_loglist		**loglist,
		int			*loglist_count,
		int			*alloc);
static pwr_tUInt32 tlog_objectlist_add( 
				pwr_tObjid		object_objid,
				pwr_tClassId		class,
				tlog_t_objectlist	**objectlist,
				int			*objectlist_count,
				int			*alloc);
static pwr_tUInt32 tlog_object_add( 
				pwr_tClassId		class,
				tlog_t_objectlist	**objectlist,
				int			*objectlist_count,
				int			*object_alloc);
static pwr_tUInt32 tlog_timestring( 	pwr_tDeltaTime	*time,
					char		*timestr);
static pwr_tUInt32 tlog_fulltimestring( 	pwr_tTime	*time,
						char		*timestr);
static pwr_tUInt32 tlog_getfilename( pwr_tObjid objid, char *filename, 
		pwr_sClass_TLogConfigure *TLogConf);
static pwr_tUInt32 tlog_getconfigobject( pwr_sClass_TLogConfigure **TLogConf, 
		pwr_sClass_User **UserObject);



static pwr_tUInt32 tlog_loglist_add();
static pwr_tUInt32 tlog_timestring();


/****************************************************************************
* Name:		tlog_sleep()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Sleep...
*
**************************************************************************/
static pwr_tUInt32 tlog_sleep( float time)
{
#ifdef OS_VMS
        int sts;
        sts = lib$wait(&time);
#elif OS_ELN
	LARGE_INTEGER	l_time;

	l_time.high = -1;	
	l_time.low = - time * 10000000;	
	ker$wait_any( NULL, NULL, &l_time);
#else
        pwr_tDeltaTime  p_time;

        time_FloatToD( &p_time, time);
        nanosleep( (pwr_tTime *)&p_time, NULL);
#endif
	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_exec_off_all()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Turn execution of all plcpgm's off.
*
**************************************************************************/
static pwr_tUInt32 tlog_exec_off_all( pwr_tObjid objid)
{
	int		sts;
	pwr_tObjid	plcpgm;
	pwr_tClassId	class;
	pwr_tObjid	window;
	pwr_tObjid	parent;
	pwr_tOName	name;
	pwr_tBoolean	value = 1;
	int		found;

	/* Get the plcpgm of the calling objid, it is not advisable to turn
	   execution of this plc off... */
	found = 0;
	sts = gdh_GetParent ( objid, &parent);
	while( ODD(sts))
	{
	  sts = gdh_GetObjectClass ( parent, &class);
	  if ( EVEN(sts)) return sts;
	  if ( class == pwr_cClass_plc)
	  {
	    found = 1;
	    break;
	  } 
	  sts = gdh_GetParent ( parent, &parent);
	}
	if ( !found)
	  return sts;

	/* Get all plcpgm of this node */
	sts = gdh_GetClassList( pwr_cClass_plc, &plcpgm);
	while ( ODD(sts))
	{
	  if ( cdh_ObjidIsNotEqual( plcpgm, parent))
	  {
	    /* Get the child window to the plc and set ScanOff to true */
	    sts = gdh_GetChild( plcpgm, &window);
	    while ( ODD(sts))
	    {
	      sts = gdh_GetObjectClass( window, &class);
	      if ( EVEN(sts)) return sts;
	      if ( class == pwr_cClass_windowplc )
	      {
	        sts = gdh_ObjidToName ( window, name, sizeof(name), cdh_mNName);
	        if ( EVEN(sts)) return sts;
	        strcat( name, ".ScanOff");
	        sts = gdh_SetObjectInfo ( name, &value, sizeof(value));
	        if ( EVEN(sts)) return sts;
	        break;
	      }
	      sts = gdh_GetNextSibling ( window, &window);
	    }
	  }
	  sts = gdh_GetNextObject( plcpgm, &plcpgm);
	}

	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_exec_on_all()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Turn execution of all plcpgm's on.
*
**************************************************************************/
static pwr_tUInt32 tlog_exec_on_all( pwr_tObjid objid)
{
	int		sts;
	pwr_tObjid	plcpgm;
	pwr_tClassId	class;
	pwr_tObjid	window;
	pwr_tAName	name;
	pwr_tBoolean	value = 0;

	/* Get all initsteps of this node */
	sts = gdh_GetClassList( pwr_cClass_plc, &plcpgm);
	while ( ODD(sts))
	{
	  /* Get the child window to the plc and set ScanOff to true */
	  sts = gdh_GetChild( plcpgm, &window);
	  while ( ODD(sts))
	  {
	    sts = gdh_GetObjectClass( window, &class);
	    if ( EVEN(sts)) return sts;
	    if ( class == pwr_cClass_windowplc )
	    {
	      sts = gdh_ObjidToName ( window, name, sizeof(name), cdh_mNName);
	      if ( EVEN(sts)) return sts;
	      strcat( name, ".ScanOff");
	      sts = gdh_SetObjectInfo ( name, &value, sizeof(value));
	      if ( EVEN(sts)) return sts;
	      break;
	    }
	    sts = gdh_GetNextSibling ( window, &window);
	  }
	  sts = gdh_GetNextObject( plcpgm, &plcpgm);
	}

	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_file_close()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_file_close( tlog_t_filectx *filectx)
{

	fclose( filectx->outfile);
	filectx->outfile = 0;
	filectx->open_time.tv_sec = 0;
	filectx->open_time.tv_nsec = 0;

	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_extselectlist_reset()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_extselectlist_reset( tlog_t_filectx *filectx)
{
	int	i;

	filectx->extselectlist = 0;
	selectlist_current = selectlist_count;
	if ( selectlist_count == 0)
	  return TLOG__SUCCESS;

	for ( i = 0; i < selectlist_count; i++)
	{
	  selectlists[i].check = 0;
	}
	if ( selectlist_count > 0)
	  selectlists[ selectlist_count - 1].check = 1;

	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_selectlist_modify()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_selectlist_modify( tlog_t_filectx *filectx,
			pwr_tObjid	objid)
{
	int	i;

	for ( i = 0; i < selectlist_count; i++)
	{
	  if ( cdh_ObjidIsEqual( objid, selectlists[i].objid))
	  {
	    selectlist_current = i + 1;
	    break;
	  }
	}

	if ( log) printf( "mod      num: %2d curr %2d check: %1d %1d %1d %1d\n", 
	  selectlist_count,
	  selectlist_current,
	  selectlists[0].check,
	  selectlists[1].check,
	  selectlists[2].check,
	  selectlists[3].check);


	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_extselectlist_unmodify()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_extselectlist_unmodify( tlog_t_filectx *filectx,
			pwr_tObjid	objid)
{
	int	i, j;

	for ( i = 0; i < selectlist_count; i++)
	{
	  if ( cdh_ObjidIsEqual( objid, selectlists[i].objid))
	  {
	    if ( selectlists[i].check)
	    {
	      /* This is the current one, the previous should also be checked */
	      for ( j = i - 1; j >= 0; j--)
	      {
	        if ( !selectlists[j].deleted )
	        {
	          selectlists[j].check = 1;
		  filectx->extselectlist = 1;
	          break;
	        }
	      }
	    }
	    break;
	  }
	}

	if ( log) printf( "extunmod num: %2d curr %2d check: %1d %1d %1d %1d\n", 
	  selectlist_count,
	  selectlist_current,
	  selectlists[0].check,
	  selectlists[1].check,
	  selectlists[2].check,
	  selectlists[3].check);

	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_selectlist_check()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_selectlist_check( tlog_t_filectx *filectx,
			pwr_tObjid	objid)
{
	int	i, j;
	int	sts;
	pwr_tOName name;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mNName);
	if ( EVEN(sts)) return sts;
	j = 0;
	i = selectlist_current - 1;
	while( j < 10 && selectlists[i].TLogSelectList[j][0] != 0)
	{
	  if ( strncmp( selectlists[i].TLogSelectList[j], name, 
			strlen( selectlists[i].TLogSelectList[j])) ==0)
	  return TLOG__SUCCESS;
	  j++;
	}
	return TLOG__NOTSEL;
}

/****************************************************************************
* Name:		tlog_extselectlist_check()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_extselectlist_check( tlog_t_filectx *filectx,
			pwr_tObjid	objid)
{
	int	i, j;
	int	sts;
	pwr_tOName	name;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mNName);
	if ( EVEN(sts)) return sts;
	for ( i = 0; i < selectlist_count; i++)
	{
	  if ( selectlists[i].check)
	  {
	    j = 0;
	    while( j < 10 && selectlists[i].TLogSelectList[j][0] != 0)
	    {
	      if ( strncmp( selectlists[i].TLogSelectList[j], name, 
			strlen( selectlists[i].TLogSelectList[j])) ==0)
	        return TLOG__SUCCESS;
	      j++;
	    }
	  }
	}
	return TLOG__NOTSEL;
}

/****************************************************************************
* Name:		tlog_extselectlist_modify()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_extselectlist_modify( tlog_t_filectx *filectx,
			pwr_tObjid	objid,
			pwr_sClass_User *UserObject,
			pwr_tObjid *tlog_selectlist,
			pwr_tObjid *alarm_selectlist)
{
	pwr_tObjid	*objid_ptr;
	int	i;
	int	sts;


	if ( selectlist_count >= 20)
	  return TLOG__MAXMODIFY;

	objid_ptr = tlog_selectlist;
	for ( i = 0; i < 10; i++)
	{
	  if ( cdh_ObjidIsNotNull( *objid_ptr))
	  {
	    sts = gdh_ObjidToName ( *objid_ptr, filectx->TLogSelectList[i],
		sizeof( filectx->TLogSelectList[0]), cdh_mNName);
	    if ( EVEN(sts)) return sts;
	    strcat( filectx->TLogSelectList[i], "-");
	  }
	  else
	    strcpy( filectx->TLogSelectList[i], "");
	  objid_ptr++;
	}
	objid_ptr = alarm_selectlist;
	for ( i = 0; i < 8; i++)
	{
	  if ( cdh_ObjidIsNotNull( *objid_ptr))
	  {
	    sts = gdh_ObjidToName ( *objid_ptr, UserObject->SelectList[i],
		sizeof( UserObject->SelectList[0]), cdh_mNName);
	    if ( EVEN(sts)) return sts;
	  }
	  else
	    strcpy( UserObject->SelectList[i], "");
	  objid_ptr++;
	}

	memcpy( &selectlists[ selectlist_count].TLogSelectList,
		&filectx->TLogSelectList,
		sizeof( selectlists[0].TLogSelectList));
/*
	memcpy( &selectlists[ selectlist_count].AlarmSelectList, 
		&UserObject->SelectList,
		sizeof( selectlists[0].AlarmSelectList));
*/
	selectlists[ selectlist_count].deleted = 0;
	selectlists[ selectlist_count].objid = objid;
	selectlist_count++;
	selectlists[ selectlist_count - 1].check = 1;
	filectx->extselectlist = 1;

	if ( log) printf( "extmod   num: %2d curr %2d check: %1d %1d %1d %1d\n", 
	  selectlist_count,
	  selectlist_current,
	  selectlists[0].check,
	  selectlists[1].check,
	  selectlists[2].check,
	  selectlists[3].check);
	if ( log) printf( "alarmsel: %s\n", UserObject->SelectList[0]);

	return TLOG__SUCCESS;
}


/****************************************************************************
* Name:		tlog_selectlist_unmodify()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*
**************************************************************************/
static pwr_tUInt32 tlog_selectlist_unmodify( tlog_t_filectx *filectx,
			pwr_tObjid	objid,
			pwr_sClass_User *UserObject)
{
	int	i;

	if ( cdh_ObjidIsEqual( selectlists[ selectlist_count - 1].objid, objid))
	{
	  selectlist_count--;
	  selectlists[ selectlist_count].deleted = 0;
	  selectlists[ selectlist_count].objid = pwr_cNObjid;
	  selectlists[ selectlist_count].check = 0;
	  if ( selectlist_count > 0)
	  {
	    while ( selectlists[ selectlist_count - 1].deleted == 1
		&& selectlist_count > 0)
	    {
	      selectlist_count--;
	      selectlists[ selectlist_count].deleted = 0;
	      selectlists[ selectlist_count].objid = pwr_cNObjid;
	      selectlists[ selectlist_count].check = 0;
	    }
	  }
	  selectlist_current = selectlist_count;
	  memcpy(
		&filectx->TLogSelectList,
		&selectlists[ selectlist_count - 1].AlarmSelectList,
		sizeof( selectlists[0].TLogSelectList));
/*
	  memcpy(
		&UserObject->SelectList,
		&selectlists[ selectlist_count - 1].AlarmSelectList,
		sizeof( selectlists[0].AlarmSelectList));
*/
	}
	else
	{
	  for ( i = 0; i < selectlist_count; i++)
	  {
	    if ( cdh_ObjidIsEqual( selectlists[i].objid, objid))
	    {
	      selectlists[i].deleted = 1;
	      break;
	    }
	  }
	}

	if ( log) printf( "unmod    num: %2d curr %2d check: %1d %1d %1d %1d\n", 
	  selectlist_count,
	  selectlist_current,
	  selectlists[0].check,
	  selectlists[1].check,
	  selectlists[2].check,
	  selectlists[3].check);
	if ( log) printf( "alarmsel: %s\n", UserObject->SelectList[0]);

	return TLOG__SUCCESS;
}


/****************************************************************************
* Name:		tlog_mh_info_bc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Backcall from mh.
*
**************************************************************************/
static pwr_tStatus tlog_mh_info_bc( mh_sMessage *MsgP)
{
	int		sts;
	ala_uEvent	*EventP;

	EventP = (ala_uEvent *) MsgP ; 

	/* Insert in loglist */
        if ((MsgP->Status & mh_mEventStatus_NotAck) ||
            (MsgP->Status & mh_mEventStatus_NotRet))
	{
	  sts = tlog_loglist_add( &filectx, 
		  pwr_cNObjid,
		  tlog_logtype_Info,
		  &EventP->Msg.EventText, 
		  NULL,
		  &EventP->Info.EventName, 
		  &EventP->Info.EventTime,
		  &log_list, &log_count, &log_alloc);
	  if ( EVEN(sts)) return sts;

	  if ( tlog_TLogConf->AutoAck)
	    mh_OutunitAck( &EventP->Info.Id);
	}
	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_mh_alarm_bc()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Backcall from mh.
*
**************************************************************************/
static pwr_tStatus tlog_mh_alarm_bc( mh_sMessage *MsgP)
{
	int		sts;
	ala_uEvent	*EventP;
	tlog_e_logtype	logtype;

	EventP = (ala_uEvent *) MsgP ; 

	/* Insert in loglist */
        if ((MsgP->Status & mh_mEventStatus_NotAck) ||
            (MsgP->Status & mh_mEventStatus_NotRet))
	{
	  switch ( MsgP->Info.EventPrio)
	  {
	    case mh_eEventPrio_A: logtype = tlog_logtype_AlarmA; break;
	    case mh_eEventPrio_B: logtype = tlog_logtype_AlarmB; break;
	    case mh_eEventPrio_C: logtype = tlog_logtype_AlarmC; break;
	    case mh_eEventPrio_D: logtype = tlog_logtype_AlarmD; break;
	    default: logtype = 0;
	  }
	  sts = tlog_loglist_add( &filectx, 
		  pwr_cNObjid,
		  logtype,
		  &EventP->Msg.EventText, 
		  NULL,
		  &EventP->Info.EventName, 
		  &EventP->Info.EventTime,
		  &log_list, &log_count, &log_alloc);
	  if ( EVEN(sts)) return sts;

	  if ( tlog_TLogConf->AutoAck)
	    mh_OutunitAck( &EventP->Info.Id);
	}
	return TLOG__SUCCESS;
}


/****************************************************************************
* Name:		tlog_alarm_update()
*
* Type		int
*
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Receive alarms.
*
**************************************************************************/
static pwr_tUInt32 tlog_alarm_update () 
{
	int	sts;

	sts = mh_OutunitReceive();     
	while (ODD(sts) && sts != MH__NOMESSAVAIL)
	{
          sts = mh_OutunitReceive();     
	}
      
	return TLOG__SUCCESS;
}

/****************************************************************************
* Name:		tlog_alarm_connect()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Connect to alarm handler.
*
**************************************************************************/
static pwr_tUInt32 tlog_alarm_connect( 	pwr_tObjid	user_object)
{
	int		sts;

	sts = mh_OutunitConnect(
		user_object,
		mh_eOutunitType_Operator,
		0,
		NULL,
		tlog_mh_alarm_bc,
		NULL,
		NULL,
		NULL,
		NULL,
		tlog_mh_info_bc,
		NULL
		);
	if (EVEN(sts)) return sts;

/*
	sts = tlog_alarm_update();
*/
	return TLOG__SUCCESS;
}


/*************************************************************************
*
* Name:		tlog_loglist_sort()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	This function sorts the items in the loglist in chronologic order.
*
**************************************************************************/

static pwr_tUInt32 tlog_loglist_sort(
		tlog_t_loglist		*loglist,
		int			loglist_count)
{
	int	i, j;
	tlog_t_loglist	dum;
	tlog_t_loglist	*loglist_ptr;

	for ( i = loglist_count - 1; i > 0; i--)
	{
	  loglist_ptr = loglist;
	  for ( j = 0; j < i; j++)
	  {
/***
            lib$subx( &loglist_ptr->diftime, &(loglist_ptr + 1)->diftime, 
		&diftime);
	    if ( diftime.high < 0)
***/
	    if ( time_Dcomp( &loglist_ptr->diftime, &(loglist_ptr + 1)->diftime)
			== -1)
	    {
	      /* Change order */
	      memcpy( &dum, loglist_ptr + 1, sizeof( tlog_t_loglist));
	      memcpy( loglist_ptr + 1, loglist_ptr, sizeof( tlog_t_loglist));
	      memcpy( loglist_ptr, &dum, sizeof( tlog_t_loglist));
	    }
	    loglist_ptr++;
	  }
	}
	return TLOG__SUCCESS;
}


/*************************************************************************
*
* Name:		tlog_loglist_print()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Print the loglist.
*
**************************************************************************/

static pwr_tUInt32 tlog_loglist_print( 
		tlog_t_filectx 		*filectx,
		tlog_t_loglist		*loglist,
		int			*loglist_count,
		pwr_sClass_User 	*UserObject
	)
{
	int		i;
	int		sts;
	char		timestr[64];
	tlog_t_loglist	*loglist_ptr;
	char		logtypestr[5];
	int		close;

	tlog_loglist_sort( loglist, *loglist_count);

	close = 0;
	loglist_ptr = loglist;
	for ( i = 0; i < *loglist_count; i++, loglist_ptr++)
	{
	  /* Print time, text and object on logfile */

	  if ( 
		filectx->extselectlist == 1 &&
		cdh_ObjidIsNotNull( loglist_ptr->objid) &&
		!(loglist_ptr->logtype == tlog_logtype_Modify ||
		loglist_ptr->logtype == tlog_logtype_UnModify  ||
		loglist_ptr->logtype == tlog_logtype_Close ))
	  {
	    sts = tlog_selectlist_check( filectx, loglist_ptr->objid);
	    if ( sts == TLOG__NOTSEL)
	      continue;
	    else if ( EVEN(sts))
	      return sts;
	  }

	  switch( loglist_ptr->logtype)
	  {
	    case tlog_logtype_DtLogUp:
	      strcpy( logtypestr, "Du"); 	
	      break;
	    case tlog_logtype_DtLogDown:
	      strcpy( logtypestr, "Dd");
	      break;
	    case tlog_logtype_AtLogUp:
	      strcpy( logtypestr, "Au");
	      break;
	    case tlog_logtype_AtLogDown:
	      strcpy( logtypestr, "Ad");
	      break;
	    case tlog_logtype_Info:
	      strcpy( logtypestr, "Mi");
	      break;
	    case tlog_logtype_AlarmA:
	      strcpy( logtypestr, "Ma");
	      break;
	    case tlog_logtype_AlarmB:
	      strcpy( logtypestr, "Mb");
	      break;
	    case tlog_logtype_AlarmC:
	      strcpy( logtypestr, "Mc");
	      break;
	    case tlog_logtype_AlarmD:
	      strcpy( logtypestr, "Md");
	      break;
	    case tlog_logtype_Modify:
	      strcpy( logtypestr, "Sl");
	      sts = tlog_selectlist_modify( filectx, loglist_ptr->objid);
	      break;
	    case tlog_logtype_UnModify:
	      strcpy( logtypestr, "Sl");
	      sts = tlog_selectlist_unmodify( filectx, loglist_ptr->objid,
			UserObject);
	      break;
	    case tlog_logtype_Close:
	      sts = tlog_selectlist_unmodify( filectx, loglist_ptr->objid,
			UserObject);
	      strcpy( logtypestr, "Fc");
	      close = 1;
	      break;
	    case tlog_logtype_CAtLog:
	      strcpy( logtypestr, "Ca");
	      break;
	    case tlog_logtype_CDtLog:
	      strcpy( logtypestr, "Cd");
	      break;
	    case tlog_logtype_COtLog:
	      strcpy( logtypestr, "Co");
	      break;
	    case tlog_logtype_ChgAtLog:
	      strcpy( logtypestr, "Ha");
	      break;
	    case tlog_logtype_ChgOtLog:
	      strcpy( logtypestr, "Ho");
	      break;
	    case tlog_logtype_ExecOn:
	    case tlog_logtype_ExecOff:
	      strcpy( logtypestr, "Ex");
	      break;
	    default:
	      strcpy( logtypestr, "  ");
	  }
	  tlog_timestring( &loglist_ptr->diftime, timestr);
	  fprintf( filectx->outfile, "%11s %s %-40.40s %s\n", timestr, 
			logtypestr,
			loglist_ptr->attrname, 
			loglist_ptr->logtext);
	  
	  if ( close)
	  {
	    tlog_file_close( filectx);
	    break;
	  }
	}
	*loglist_count = 0;
	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		tlog_loglist_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Add an object to the loglist.
*
**************************************************************************/

static pwr_tUInt32 tlog_loglist_add( 
		tlog_t_filectx 		*filectx,
		pwr_tObjid		objid,
		tlog_e_logtype		logtype,
		pwr_tString80		*logtext,
		pwr_sAttrRef		*attribute,
		pwr_tString80		*attrname,
		pwr_tTime		*logtime,
		tlog_t_loglist		**loglist,
		int			*loglist_count,
		int			*alloc)
{
	tlog_t_loglist	*loglist_ptr;
	tlog_t_loglist	*new_loglist;
	int			sts;

	if ( filectx->outfile == 0)
	  return TLOG__SUCCESS;

	if ( filectx->open_time.tv_sec == 0 && filectx->open_time.tv_nsec == 0)
	  return TLOG__SUCCESS;

	if ( cdh_ObjidIsNotNull( objid) && !(logtype == tlog_logtype_Modify ||
		logtype == tlog_logtype_UnModify ||
		logtype == tlog_logtype_Close ))
	{
	  if ( filectx->extselectlist == 1 )
	    sts = tlog_extselectlist_check( filectx, objid);
	  else
	    sts = tlog_selectlist_check( filectx, objid);
	  if ( sts == TLOG__NOTSEL)
	    return TLOG__SUCCESS;
	  else if ( EVEN(sts))
	    return sts;
	}

	if ( *alloc == 0)
	{
	  *loglist = calloc( TLOG_ALLOC , sizeof( tlog_t_loglist));
	  if ( *loglist == 0)
	    return TLOG__NOMEMORY;
	  *alloc = TLOG_ALLOC;
	}
	else if ( *alloc <= *loglist_count)
	{
	  new_loglist = calloc( *alloc + TLOG_ALLOC, sizeof( tlog_t_loglist));
	  if ( new_loglist == 0)
	    return TLOG__NOMEMORY;
	  memcpy( new_loglist, *loglist, 
			*loglist_count * sizeof( tlog_t_loglist));
	  free( *loglist);
	  *loglist = new_loglist;
	  (*alloc) += TLOG_ALLOC;
	}
	loglist_ptr = *loglist + *loglist_count;
	loglist_ptr->logtype = logtype;
	loglist_ptr->objid = objid;
	strcpy( loglist_ptr->logtext, (char *) logtext);
	if ( attribute != NULL)
	{
/*	  sts = gdh_AttrrefToName ( attribute, loglist_ptr->attrname, 
			sizeof(pwr_tString80), cdh_mNName);
*/
	  sts = gdh_ObjidToName ( attribute->Objid, loglist_ptr->attrname, 
			sizeof(loglist_ptr->attrname), cdh_mNName);
	  if ( EVEN(sts))
	  {
	    sts = gdh_ObjidToName ( objid, loglist_ptr->attrname, 
			sizeof(loglist_ptr->attrname), cdh_mNName);
	    if ( EVEN(sts))
	      strcpy( loglist_ptr->attrname, "");
	  }
	}
	else
	  strcpy( loglist_ptr->attrname, (char *) attrname);

	memcpy( &loglist_ptr->logtime, logtime, sizeof( pwr_tTime));
	time_Adiff( &loglist_ptr->diftime, logtime, &filectx->open_time);

	(*loglist_count)++;
	return TLOG__SUCCESS;
}
/*************************************************************************
*
* Name:		tlog_objectlist_add()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Add an object to the objectlist.
*
**************************************************************************/

static pwr_tUInt32 tlog_objectlist_add( 
				pwr_tObjid		object_objid,
				pwr_tClassId		class,
				tlog_t_objectlist	**objectlist,
				int			*objectlist_count,
				int			*alloc)
{
	tlog_t_objectlist	*objectlist_ptr;
	tlog_t_objectlist	*new_objectlist;
	pwr_tOName     		namebuf;
	int			sts;
	pwr_sAttrRef		attrref;

	if ( *objectlist_count == 0)
	{
	  *objectlist = calloc( TLOG_ALLOC , sizeof( tlog_t_objectlist));
	  if ( *objectlist == 0)
	    return TLOG__NOMEMORY;
	  *alloc = TLOG_ALLOC;
	}
	else if ( *alloc <= *objectlist_count)
	{
	  new_objectlist = calloc( *alloc + TLOG_ALLOC, sizeof( tlog_t_objectlist));
	  if ( new_objectlist == 0)
	    return TLOG__NOMEMORY;
	  memcpy( new_objectlist, *objectlist, 
			*objectlist_count * sizeof( tlog_t_objectlist));
	  free( *objectlist);
	  *objectlist = new_objectlist;
	  (*alloc) += TLOG_ALLOC;
	}
	objectlist_ptr = *objectlist + *objectlist_count;
	objectlist_ptr->objid = object_objid;

        sts = gdh_ObjidToName ( object_objid, namebuf, sizeof( namebuf), cdh_mNName);
        if ( EVEN(sts)) return sts;

        strcpy( objectlist_ptr->name, namebuf);

	memset( &attrref, 0, sizeof( attrref));
	attrref.Objid = object_objid;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
		(pwr_tAddress *) &objectlist_ptr->object_ptr,
		&objectlist_ptr->subid);
        if ( EVEN(sts)) return sts;

	(*objectlist_count)++;
	return TLOG__SUCCESS;
}

static pwr_tUInt32 tlog_object_add( 
				pwr_tClassId		class,
				tlog_t_objectlist	**objectlist,
				int			*objectlist_count,
				int			*object_alloc)
{
    pwr_tObjid		object_objid;
    int			sts;

    /* Get all initsteps of this node */
    sts = gdh_GetClassList ( class, &object_objid);
    while ( ODD(sts))
    {
      /* Store and direct link the initstep */
      sts = tlog_objectlist_add( object_objid, class, objectlist,
		objectlist_count, object_alloc);
      if ( EVEN(sts)) return sts;

      sts = gdh_GetNextObject ( object_objid, &object_objid);
    }

    return TLOG__SUCCESS;
}

static pwr_tUInt32 tlog_timestring( 	pwr_tDeltaTime	*time,
					char		*timestr)
{
	char			timstr[64];
	int			sts;
/****
	unsigned short int	len;
	struct dsc$descriptor_s	timstr_desc = {sizeof(timstr)-1,DSC$K_DTYPE_T,
					DSC$K_CLASS_S,};

	timstr_desc.dsc$a_pointer = timstr;

	if ( time->high >= 0)
	  strcpy( timestr, "00:00:00.00");
	else
	{
	  sys$asctim( &len, &timstr_desc, time, 0);
	  timstr[len] = 0 ; 
	  strcpy( timestr, &timstr[5]);
	}
****/
	sts = time_DtoAscii( time, 1, timstr, sizeof(timstr));
	if ( ODD(sts))
	  strcpy( timestr, timstr);
	return TLOG__SUCCESS;
}

static pwr_tUInt32 tlog_fulltimestring( 	pwr_tTime	*time,
						char		*timestr)
{
	char			timstr[64];
	int			sts;
/**********
	unsigned short int	len;
	struct dsc$descriptor_s	timstr_desc = {sizeof(timstr)-1,DSC$K_DTYPE_T,
					DSC$K_CLASS_S,};

	timstr_desc.dsc$a_pointer = timstr;

	sys$asctim( &len, &timstr_desc, time, 0);
	timstr[len] = 0 ; 
	strcpy( timestr, &timstr[0]);
**********/
	sts = time_AtoAscii( time, time_eFormat_DateAndTime, timstr, 
		sizeof(timstr));
	if ( ODD(sts))
	  strcpy( timestr, timstr);
	return TLOG__SUCCESS;
}

static pwr_tUInt32 tlog_getfilename( pwr_tObjid objid, char *filename, 
		pwr_sClass_TLogConfigure *TLogConf)
{
	int	sts;
	pwr_tOName name;
	char	*s;

	sts = gdh_ObjidToName ( objid, name, sizeof(name), cdh_mNName);
	if ( EVEN(sts)) return sts;
	for ( s = name; *s; s++)
	{
	  if ( *s == '�')
	    *s = 'a';
	  else if ( *s == '�')
	    *s = 'a';
	  else if ( *s == '�')
	    *s = 'o';
	  else if ( *s == '�')
	    *s = 'A';
	  else if ( *s == '�')
	    *s = 'A';
	  else if ( *s == '�')
	    *s = 'O';
	}

	strcpy( filename, TLogConf->TLogDirectory);
	s = strrchr( name, '-');
	if ( s == 0)
	  strcat( filename, name);
	else
	{
	  *s = '_';
	  s = strrchr( name, '-');
	  if ( s == 0)
	    strcat( filename, name);
	  else
	  {
	    *s = '_';
	    s = strrchr( name, '-');
	    if ( s == 0)
	      strcat( filename, name);
	    else
	    {
	      s++;
	      strcat( filename, s);
	    }
	  }
	}	
	strcat(filename, ".tlog");
	return TLOG__SUCCESS;
}

static pwr_tUInt32 tlog_getconfigobject( pwr_sClass_TLogConfigure **TLogConf, 
		pwr_sClass_User **UserObject)
{
	pwr_tObjid		node;
	pwr_tObjid		Objid;
	pwr_tClassId	class;
	int		sts;
	pwr_sAttrRef	attrref;
	int		found;
	gdh_tDlid	subid;

	/* Try to find a TLogConfigure object for the node */
	sts = gdh_GetNodeObject ( 0, &node);
	if ( EVEN(sts)) return sts;

	/* Look for a User object as a child to the node object */
	found = 0;
	sts = gdh_GetChild( node, &Objid);
	while ( ODD(sts))
	{
	  sts = gdh_GetObjectClass ( Objid, &class);
	  if ( EVEN(sts)) return sts;
	  if ( class == pwr_cClass_TLogConfigure)
	  {
	    memset( &attrref, 0, sizeof( attrref));
	    attrref.Objid = Objid;
	    sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
				(pwr_tAddress *) TLogConf, &subid);
            if ( EVEN(sts)) return sts;

	    found = 1;
	    break;
	  }	    
	  sts = gdh_GetNextSibling ( Objid, &Objid);
	}
	if ( found == 0)
	  return TLOG__CONFOBJNOTFOUND;

	/* Get the userobject */
	memset( &attrref, 0, sizeof( attrref));
	attrref.Objid = (*TLogConf)->UserObject;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref, 
				(pwr_tAddress *) UserObject, &subid);
        if ( EVEN(sts)) return TLOG__USEROBJNOTFOUND;

	return TLOG__SUCCESS;
}

/*************************************************************************
*
* Name:		main()
*
* Type		int
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Main program of rs_tlog.
*
**************************************************************************/

int main( int argc, char **argv)
{
	tlog_t_objectlist	*TLogOpen_list;
	int			TLogOpen_count;
	int			TLogOpen_alloc;
	tlog_t_objectlist	*TLogModify_list;
	int			TLogModify_count;
	int			TLogModify_alloc;
	tlog_t_objectlist	*DtLog_list;
	int			DtLog_count;
	int			DtLog_alloc;
	tlog_t_objectlist	*AtLog_list;
	int			AtLog_count;
	int			AtLog_alloc;
	tlog_t_objectlist	*CAtLog_list;
	int			CAtLog_count;
	int			CAtLog_alloc;
	tlog_t_objectlist	*CDtLog_list;
	int			CDtLog_count;
	int			CDtLog_alloc;
	tlog_t_objectlist	*COtLog_list;
	int			COtLog_count;
	int			COtLog_alloc;
	tlog_t_objectlist	*ChgAtLog_list;
	int			ChgAtLog_count;
	int			ChgAtLog_alloc;
	tlog_t_objectlist	*ChgOtLog_list;
	int			ChgOtLog_count;
	int			ChgOtLog_alloc;
	tlog_t_objectlist	*ExecOn_list;
	int			ExecOn_count;
	int			ExecOn_alloc;
	tlog_t_objectlist	*ExecOff_list;
	int			ExecOff_count;
	int			ExecOff_alloc;
	int			sts;
	int			i, first;
	tlog_t_objectlist	*list_ptr;
	char			filename[120];
	pwr_sClass_User 	*UserObject;
	char			timestr[64];
	pwr_tOName		name;
	char			text[120];
	double			scantime;

	filectx.outfile = 0;
	filectx.open_time.tv_sec = 0;
	filectx.open_time.tv_nsec = 0;
	selectlist_count = 0;

#ifdef OS_ELN
	/* Wait for the plcpgm has flagged initizated */
	plc_UtlWaitForPlc();
#endif

	/* Init gdh */
	sts = gdh_Init("rs_tlog");
	if (EVEN(sts)) LogAndExit( sts);


	/* Get the config object */
	sts = tlog_getconfigobject( &tlog_TLogConf, &UserObject);
	if (EVEN(sts)) LogAndExit( sts);

	scantime = tlog_TLogConf->CycleTime;
	if ( scantime < 0.01)
	  scantime = 2;

	sts = tlog_alarm_connect ( tlog_TLogConf->UserObject);
	if (EVEN(sts)) LogAndExit( sts);

	/* Direct link to the TLogOpen, TLogModify, DtLog and AtLog */
	TLogOpen_count = 0;
	TLogModify_count = 0;
	DtLog_count = 0;
	AtLog_count = 0;
	CAtLog_count = 0;
	CDtLog_count = 0;
	COtLog_count = 0;
	ChgAtLog_count = 0;
	ChgOtLog_count = 0;
	ExecOn_count = 0;
	ExecOff_count = 0;

	sts = tlog_object_add( pwr_cClass_TLogOpen, &TLogOpen_list, 
		&TLogOpen_count, &TLogOpen_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_TLogModify, &TLogModify_list, 
		&TLogModify_count, &TLogModify_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_DtLog, &DtLog_list, 
		&DtLog_count, &DtLog_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_AtLog, &AtLog_list, 
		&AtLog_count, &AtLog_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_CAtLog, &CAtLog_list, 
		&CAtLog_count, &CAtLog_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_CDtLog, &CDtLog_list, 
		&CDtLog_count, &CDtLog_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_COtLog, &COtLog_list, 
		&COtLog_count, &COtLog_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_ChgAtLog, &ChgAtLog_list, 
		&ChgAtLog_count, &ChgAtLog_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_ChgOtLog, &ChgOtLog_list, 
		&ChgOtLog_count, &ChgOtLog_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_TLogExecAllOn, &ExecOn_list, 
		&ExecOn_count, &ExecOn_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	sts = tlog_object_add( pwr_cClass_TLogExecAllOff, &ExecOff_list, 
		&ExecOff_count, &ExecOff_alloc);
	if (EVEN(sts)) LogAndExit( sts);

	while(1)
	{
	  if ( !tlog_TLogConf->On)
	  {
	    tlog_sleep( scantime);
	    continue;
	  }

	  /* Reset all Signals in the objects */
	  list_ptr = TLogOpen_list;
	  for ( i = 0; i < TLogOpen_count; i++)
	  {
	    ((pwr_sClass_TLogOpen *)(list_ptr->object_ptr))->OpenSignal = 0;
	    ((pwr_sClass_TLogOpen *)(list_ptr->object_ptr))->CloseSignal = 0;
	    ((pwr_sClass_TLogOpen *)(list_ptr->object_ptr))->FileOpen = 0;
	   list_ptr++;
	  }
	  list_ptr = TLogModify_list;
	  for ( i = 0; i < TLogModify_count; i++)
	  {
	    ((pwr_sClass_TLogModify *)(list_ptr->object_ptr))->ModifySignal = 0;
	    ((pwr_sClass_TLogModify *)(list_ptr->object_ptr))->UnModifySignal = 0;
	    list_ptr++;
	  }
	  list_ptr = DtLog_list;
	  for ( i = 0; i < DtLog_count; i++)
	  {
	    ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->UpSignal = 0;
	    ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->DownSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = AtLog_list;
	  for ( i = 0; i < AtLog_count; i++)
	  {
	    ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->UpSignal = 0;
	    ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->DownSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = CAtLog_list;
	  for ( i = 0; i < CAtLog_count; i++)
	  {
	    ((pwr_sClass_CAtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = CDtLog_list;
	  for ( i = 0; i < CDtLog_count; i++)
	  {
	    ((pwr_sClass_CDtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = COtLog_list;
	  for ( i = 0; i < COtLog_count; i++)
	  {
	    ((pwr_sClass_COtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = ChgAtLog_list;
	  for ( i = 0; i < ChgAtLog_count; i++)
	  {
	    ((pwr_sClass_ChgAtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = ChgOtLog_list;
	  for ( i = 0; i < ChgOtLog_count; i++)
	  {
	    ((pwr_sClass_ChgOtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = ExecOn_list;
	  for ( i = 0; i < ExecOn_count; i++)
	  {
	    ((pwr_sClass_TLogExecAllOn *)(list_ptr->object_ptr))->ExecOnSignal = 0;
	    list_ptr++;
	  }
	  list_ptr = ExecOff_list;
	  for ( i = 0; i < ExecOff_count; i++)
	  {
	    ((pwr_sClass_TLogExecAllOff *)(list_ptr->object_ptr))->ExecOffSignal = 0;
	    list_ptr++;
	  }
	  sts = tlog_alarm_update();

	  /* Start the main loop */
	  first = 0;
	  log_alloc = 0;
	  log_count = 0;
	  while( 1)
	  {
	    sts = tlog_extselectlist_reset( &filectx);

	    /* Check if its time to open or close file */
	    if ( filectx.outfile == 0)
	    {
	      list_ptr = TLogOpen_list;
	      for ( i = 0; i < TLogOpen_count; i++)
	      {
	        if (((pwr_sClass_TLogOpen *)(list_ptr->object_ptr))->OpenSignal)
	        {
		  /* Open the file */
		  tlog_getfilename( list_ptr->objid, filename, tlog_TLogConf);
	          dcli_translate_filename( filename, filename);
	          filectx.outfile = fopen( filename, "w");
		  if ( filectx.outfile == 0)
	          {
	            errh_CErrLog( TLOG__FILEOPEN, errh_ErrArgAF(filename), NULL);
	            goto fatal_error;
	          }
		  else
	  	  {
		    filectx.object_ptr = 
				(pwr_sClass_TLogOpen *) list_ptr->object_ptr;
		    filectx.objid = list_ptr->objid;
		    filectx.object_ptr->OpenSignal = 0;
		    filectx.object_ptr->OpenTime.tv_sec = 0;
		    filectx.object_ptr->OpenTime.tv_nsec = 0;
		    filectx.object_ptr->FileOpen = 1;
		    sts = tlog_extselectlist_modify( &filectx,
			filectx.objid,
			UserObject,
			filectx.object_ptr->TLogSelectList,
			filectx.object_ptr->AlarmSelectList);
		    first = 1;
		  }
		  break;
	        }
	        list_ptr++;
	      }
	    }
	    else
	    {
	      if ( first)
	      {
	        if ( filectx.object_ptr->OpenTime.tv_sec != 0 || 
			filectx.object_ptr->OpenTime.tv_nsec != 0)
	        {
	          filectx.open_time = filectx.object_ptr->OpenTime;
	  	  tlog_fulltimestring( &filectx.object_ptr->OpenTime,
			timestr);
		  fprintf( filectx.outfile, "TestLog started %s\n\n", timestr);
	          first = 0;
	        }
	        else
	        {
	          tlog_sleep( scantime);
		  continue;
	        }
	      }
	
	      /* Check if any modify object is active */
	      list_ptr = TLogModify_list;
	      for ( i = 0; i < TLogModify_count; i++)
	      {
	        if (((pwr_sClass_TLogModify *)(list_ptr->object_ptr))->ModifySignal)
	        {
		  /* Modify */
		  if (
		     cdh_ObjidIsEqual( ((pwr_sClass_TLogModify *)
		     (list_ptr->object_ptr))->TLogOpenObject, filectx.objid) ||
		      cdh_ObjidIsNull(((pwr_sClass_TLogModify *)
		     (list_ptr->object_ptr))->TLogOpenObject))
	          {
	  	    sts = tlog_extselectlist_modify( &filectx,
			list_ptr->objid,
			UserObject,
	        	((pwr_sClass_TLogModify*)
				(list_ptr->object_ptr))->TLogSelectList,
	        	((pwr_sClass_TLogModify*)
				(list_ptr->object_ptr))->AlarmSelectList);
	            if (EVEN(sts)) LogAndExit( sts);
	            sts = gdh_ObjidToName ( list_ptr->objid, name, sizeof( name), cdh_mNName);
	            if (EVEN(sts)) LogAndExit( sts);
		    sts = tlog_loglist_add( &filectx,
		  	list_ptr->objid,
		  	tlog_logtype_Modify,
		  	(pwr_tString80 *) "TLogModify selectlist active",
		  	NULL,
		  	(pwr_tString80 *) name,
			&((pwr_sClass_TLogModify *)
				(list_ptr->object_ptr))->ModifyTime,
		  	&log_list, &log_count, &log_alloc);
		    if (EVEN(sts)) LogAndExit( sts);
	          }
	          ((pwr_sClass_TLogModify *)(list_ptr->object_ptr))->ModifySignal
			= 0;
	        }
	        if (((pwr_sClass_TLogModify *)(list_ptr->object_ptr))->UnModifySignal)
	        {
		  /* UnModify */
		  if ( cdh_ObjidIsEqual(((pwr_sClass_TLogModify *)
		     (list_ptr->object_ptr))->TLogOpenObject, filectx.objid) ||
		      cdh_ObjidIsNull(((pwr_sClass_TLogModify *)
		     (list_ptr->object_ptr))->TLogOpenObject))
	          {
		    sts = tlog_extselectlist_unmodify( &filectx, list_ptr->objid);
	            sts = gdh_ObjidToName ( list_ptr->objid, name, sizeof( name), cdh_mNName);
		    if (EVEN(sts)) LogAndExit( sts);
		    sts = tlog_loglist_add( &filectx,
		  	list_ptr->objid,
		  	tlog_logtype_UnModify,
		  	(pwr_tString80 *) "TLogModify selectlist inactive", 
		  	NULL,
		  	(pwr_tString80 *) name, 
			&((pwr_sClass_TLogModify *)
				(list_ptr->object_ptr))->UnModifyTime, 
		  	&log_list, &log_count, &log_alloc);
	 	    if (EVEN(sts)) LogAndExit( sts);
	          }
	          ((pwr_sClass_TLogModify *)(list_ptr->object_ptr))->UnModifySignal
			= 0;
	        }
	        list_ptr++;
	      }
	    }

	    /* Check if any ExecOn to log and execute */
	    list_ptr = ExecOn_list;
	    for ( i = 0; i < ExecOn_count; i++)
	    {
	      if (((pwr_sClass_TLogExecAllOn *)(list_ptr->object_ptr))->ExecOnSignal)
	      {
	        sts = gdh_ObjidToName ( list_ptr->objid, name, sizeof( name), cdh_mNName);
	        if (EVEN(sts)) LogAndExit( sts);
	        sts = tlog_loglist_add( &filectx, 
		  list_ptr->objid,
		  tlog_logtype_ExecOn,
		  (pwr_tString80 *) "Execution all PlcPgms set on",
		  NULL, 
		  (pwr_tString80 *) name, 
		  &((pwr_sClass_TLogExecAllOn *)(list_ptr->object_ptr))->LogTime, 
		  &log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);
	        sts = tlog_exec_on_all( list_ptr->objid);
	        if (EVEN(sts)) LogAndExit( sts);
	        ((pwr_sClass_TLogExecAllOn *)(list_ptr->object_ptr))->ExecOnSignal = 0;
	        ((pwr_sClass_TLogExecAllOn *)(list_ptr->object_ptr))->ExecOn = 1;
	      }
	      list_ptr++;
	    }
	    /* Check if any ExecOff to log and execute */
	    list_ptr = ExecOff_list;
	    for ( i = 0; i < ExecOff_count; i++)
	    {
	      if (((pwr_sClass_TLogExecAllOff *)(list_ptr->object_ptr))->ExecOffSignal)
	      {
	        sts = gdh_ObjidToName ( list_ptr->objid, name, sizeof( name), cdh_mNName);
	        if (EVEN(sts)) LogAndExit( sts);
	        sts = tlog_loglist_add( &filectx, 
		  list_ptr->objid,
		  tlog_logtype_ExecOff,
		  (pwr_tString80 *) "Execution all PlcPgms set off",
		  NULL, 
		  (pwr_tString80 *) name, 
		  &((pwr_sClass_TLogExecAllOff *)(list_ptr->object_ptr))->LogTime, 
		  &log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);
	        sts = tlog_exec_off_all( list_ptr->objid);
	        if (EVEN(sts)) LogAndExit( sts);
	        ((pwr_sClass_TLogExecAllOff *)(list_ptr->object_ptr))->ExecOffSignal = 0;
	        ((pwr_sClass_TLogExecAllOff *)(list_ptr->object_ptr))->ExecOff = 1;
	      }
	      list_ptr++;
	    }


	    /* Check if any DtLog to log */
	    list_ptr = DtLog_list;
	    for ( i = 0; i < DtLog_count; i++)
	    {
	      if (((pwr_sClass_DtLog *)(list_ptr->object_ptr))->UpSignal)
	      {
	        if ( ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->UpText[0] 
			!= 0)
	        {
	          sts = tlog_loglist_add( &filectx, 
		  	list_ptr->objid,
		  	tlog_logtype_DtLogUp,
		  	(pwr_tString80 *) ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->UpText, 
		  	&((pwr_sClass_DtLog *)(list_ptr->object_ptr))->Attribute,
		  	NULL, 
		  	&((pwr_sClass_DtLog *)(list_ptr->object_ptr))->UpTime, 
		  	&log_list, &log_count, &log_alloc);
	          if (EVEN(sts)) LogAndExit( sts);
	        }
	        ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->UpSignal = 0;
	      }
	      if (((pwr_sClass_DtLog *)(list_ptr->object_ptr))->DownSignal)
	      {
	        if ( ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->DownText[0] 
			!= 0)
	        {
	          sts = tlog_loglist_add( &filectx, 
		  	list_ptr->objid,
		  	tlog_logtype_DtLogDown,
		  	(pwr_tString80 *) ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->DownText, 
		  	&((pwr_sClass_DtLog *)(list_ptr->object_ptr))->Attribute,
		  	NULL, 
		  	&((pwr_sClass_DtLog *)(list_ptr->object_ptr))->DownTime, 
		  	&log_list, &log_count, &log_alloc);
	          if (EVEN(sts)) LogAndExit( sts);
	        }
	        ((pwr_sClass_DtLog *)(list_ptr->object_ptr))->DownSignal = 0;
	      }
	      list_ptr++;
	    }

	    /* Check if any AtLog to log */
	    list_ptr = AtLog_list;
	    for ( i = 0; i < AtLog_count; i++)
	    {
	      if (((pwr_sClass_AtLog *)(list_ptr->object_ptr))->UpSignal)
	      {
	        if ( ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->UpText[0] 
			!= 0)
	        {
	          sts = tlog_loglist_add( &filectx, 
		  	list_ptr->objid,
		  	tlog_logtype_AtLogUp,
		  	(pwr_tString80 *) ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->UpText, 
		  	&((pwr_sClass_AtLog *)(list_ptr->object_ptr))->Attribute,
		  	NULL, 
		  	&((pwr_sClass_AtLog *)(list_ptr->object_ptr))->UpTime, 
		  	&log_list, &log_count, &log_alloc);
	          if (EVEN(sts)) LogAndExit( sts);
	        }
	        ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->UpSignal = 0;
	      }
	      if (((pwr_sClass_AtLog *)(list_ptr->object_ptr))->DownSignal)
	      {
	        if ( ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->DownText[0] 
			!= 0)
	        {
	          sts = tlog_loglist_add( &filectx, 
		  	list_ptr->objid,
		  	tlog_logtype_AtLogDown,
		  	(pwr_tString80 *) ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->DownText, 
		  	&((pwr_sClass_AtLog *)(list_ptr->object_ptr))->Attribute,
		  	NULL, 
		  	&((pwr_sClass_AtLog *)(list_ptr->object_ptr))->DownTime, 
		  	&log_list, &log_count, &log_alloc);
	          if (EVEN(sts)) LogAndExit( sts);
	        }
	        ((pwr_sClass_AtLog *)(list_ptr->object_ptr))->DownSignal = 0;
	      }
	      list_ptr++;
	    }

	    /* Check if any CAtLog to log */
	    list_ptr = CAtLog_list;
	    for ( i = 0; i < CAtLog_count; i++)
	    {
	      if (((pwr_sClass_CAtLog *)(list_ptr->object_ptr))->LogSignal)
	      {
	        sprintf( text, "%g  ", 
		  ((pwr_sClass_CAtLog *)(list_ptr->object_ptr))->LogValue);
	        strcat( text, 
		  ((pwr_sClass_CAtLog *)(list_ptr->object_ptr))->LogText);
	        sts = tlog_loglist_add( &filectx, 
		    list_ptr->objid,
		    tlog_logtype_CAtLog,
		    (pwr_tString80 *) text,
		    &((pwr_sClass_CAtLog *)(list_ptr->object_ptr))->Attribute,
		    NULL, 
		    &((pwr_sClass_CAtLog *)(list_ptr->object_ptr))->LogTime, 
		    &log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);
	        ((pwr_sClass_CAtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	      }
	      list_ptr++;
	    }
	    /* Check if any CDtLog to log */
	    list_ptr = CDtLog_list;
	    for ( i = 0; i < CDtLog_count; i++)
	    {
	      if (((pwr_sClass_CDtLog *)(list_ptr->object_ptr))->LogSignal)
	      {
	        sprintf( text, "%1d  ", 
		  ((pwr_sClass_CDtLog *)(list_ptr->object_ptr))->LogValue);
	        strcat( text, 
		  ((pwr_sClass_CDtLog *)(list_ptr->object_ptr))->LogText);
	        sts = tlog_loglist_add( &filectx, 
		    list_ptr->objid,
		    tlog_logtype_CDtLog,
		    (pwr_tString80 *) text,
		    &((pwr_sClass_CDtLog *)(list_ptr->object_ptr))->Attribute,
		    NULL, 
		    &((pwr_sClass_CDtLog *)(list_ptr->object_ptr))->LogTime, 
		    &log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);
	        ((pwr_sClass_CDtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	      }
	      list_ptr++;
	    }
	    /* Check if any COtLog to log */
	    list_ptr = COtLog_list;
	    for ( i = 0; i < COtLog_count; i++)
	    {
	      if (((pwr_sClass_COtLog *)(list_ptr->object_ptr))->LogSignal)
	      {
	        sts = gdh_ObjidToName ( 
		  ((pwr_sClass_COtLog *)(list_ptr->object_ptr))->LogValue,
		  text, sizeof( text), cdh_mNName);
	        if ( EVEN(sts))
	        {
	  	  strcpy( text, "Undefined Objid");
	        }
	        strcat( text, " ");
	        strcat( text, 
		  ((pwr_sClass_COtLog *)(list_ptr->object_ptr))->LogText);
	        sts = tlog_loglist_add( &filectx, 
		    list_ptr->objid,
		    tlog_logtype_COtLog,
		    (pwr_tString80 *) text,
		    &((pwr_sClass_COtLog *)(list_ptr->object_ptr))->Attribute,
		    NULL, 
		    &((pwr_sClass_COtLog *)(list_ptr->object_ptr))->LogTime, 
		    &log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);
	        ((pwr_sClass_COtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	      }
	      list_ptr++;
	    }

	    /* Check if any ChgAtLog to log */
	    list_ptr = ChgAtLog_list;
	    for ( i = 0; i < ChgAtLog_count; i++)
	    {
	      if (((pwr_sClass_ChgAtLog *)(list_ptr->object_ptr))->LogSignal)
	      {
	        sprintf( text, "%g  ", 
		  ((pwr_sClass_ChgAtLog *)(list_ptr->object_ptr))->LogValue);
	        strcat( text, 
		  ((pwr_sClass_ChgAtLog *)(list_ptr->object_ptr))->LogText);
	        sts = tlog_loglist_add( &filectx, 
		    list_ptr->objid,
		    tlog_logtype_ChgAtLog,
		    (pwr_tString80 *) text,
		    &((pwr_sClass_ChgAtLog *)(list_ptr->object_ptr))->Attribute,
		    NULL, 
		    &((pwr_sClass_ChgAtLog *)(list_ptr->object_ptr))->LogTime, 
		    &log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);
	        ((pwr_sClass_ChgAtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	      }
	      list_ptr++;
	    }

	    /* Check if any ChgOtLog to log */
	    list_ptr = ChgOtLog_list;
	    for ( i = 0; i < ChgOtLog_count; i++)
	    {
	      if (((pwr_sClass_ChgOtLog *)(list_ptr->object_ptr))->LogSignal)
	      {
	        sts = gdh_ObjidToName ( 
		  ((pwr_sClass_ChgOtLog *)(list_ptr->object_ptr))->LogValue,
		  text, sizeof( text), cdh_mNName);
	        if ( EVEN(sts))
	        {
	  	  strcpy( text, "Undefined Objid");
	        }
	        strcat( text, " ");
	        strcat( text, 
		  ((pwr_sClass_ChgOtLog *)(list_ptr->object_ptr))->LogText);
	        sts = tlog_loglist_add( &filectx, 
		    list_ptr->objid,
		    tlog_logtype_ChgOtLog,
		    (pwr_tString80 *) text,
		    &((pwr_sClass_ChgOtLog *)(list_ptr->object_ptr))->Attribute,
		    NULL, 
		    &((pwr_sClass_ChgOtLog *)(list_ptr->object_ptr))->LogTime, 
		    &log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);
	        ((pwr_sClass_ChgOtLog *)(list_ptr->object_ptr))->LogSignal = 0;
	      }
	      list_ptr++;
	    }

	    /* Receive alarms */
	    sts = tlog_alarm_update();

	    if ( filectx.outfile != 0)
	    {
	      /* Check if file should be closed */
	      if ( filectx.object_ptr->CloseSignal)
	      {
	        /* Close */
	        sts = tlog_extselectlist_unmodify( &filectx, filectx.objid);
	        sts = tlog_loglist_add( &filectx,
		  	filectx.objid,
		  	tlog_logtype_Close,
		  	(pwr_tString80 *) "File closed",
		  	NULL,
		  	(pwr_tString80 *) "", 
			&filectx.object_ptr->CloseTime,
		  	&log_list, &log_count, &log_alloc);
	        if (EVEN(sts)) LogAndExit( sts);

	        filectx.object_ptr->CloseSignal = 0;
	        filectx.object_ptr->FileOpen = 0;
	      }

	      /* Print logging on log file */
	      sts = tlog_loglist_print( &filectx, log_list, &log_count,
			UserObject);
	      if (EVEN(sts)) LogAndExit( sts);
	    }
	    tlog_sleep( scantime);
	  }

fatal_error:
	  tlog_TLogConf->On = 0;

	}
}

