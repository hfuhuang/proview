/* 
 * Proview   $Id: rt_sysmon.cpp,v 1.2 2005-09-01 14:57:48 claes Exp $
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


#if not defined _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <sys/vfs.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#include "rt_sysmon.h"
#include "co_error.h"
#include "rt_smon_msg.h"
extern "C" {
#include "rt_gdh.h"
#include "co_cdh.h"
#include "co_errno.h"
#include "co_time.h"
#include "rt_errh.h"
#include "rt_qcom.h"
#include "rt_ini_event.h"
#include "rt_aproc.h"
#include "rt_pwr_msg.h"
#include "rt_qcom_msg.h"
#include "rt_mh_msg.h"
#include "rt_mh_net.h"
#include "rt_mh_appl.h"
}

void sysmon_object::open( double base_scantime)
{
  pwr_tStatus sts;

  // Link to object
  sts = gdh_DLRefObjectInfoAttrref( &aref, (void **)&p, &p_dlid); 
  if ( EVEN(sts)) throw co_error(sts);

  sts = gdh_GetAttrRefTid( &aref, &cid);
  if ( EVEN(sts)) throw co_error(sts);

  switch ( cid) {
  case pwr_cClass_DiskSup:
    if ( ((pwr_sClass_DiskSup *)p)->ScanTime)
      scan_div = int( ((pwr_sClass_DiskSup *)p)->ScanTime / base_scantime + 0.5);
    else
      scan_div = 1;
    break;
  default: ;
  }
  
}

pwr_tStatus sysmon_object::status()
{
  switch ( cid) {
  case pwr_cClass_DiskSup:
    return ((pwr_sClass_DiskSup *)p)->Status;
  default: ;
  }
  return SMON__UNKNOWN;
}

void sysmon_object::close()
{
  gdh_DLUnrefObjectInfo( p_dlid);
}

void sysmon_object::scan()
{
  scan_cnt++;
  if ( scan_cnt >= scan_div)
    scan_cnt = 0;
  else
    return;

  exec();
}

void rt_sysmon::open()
{
  pwr_tStatus sts;
  pwr_sAttrRef aref;
  pwr_tObjid oid;

  // Open server configuration object SysMonConfig
  sts = gdh_GetClassList( pwr_cClass_SysMonConfig, &oid);
  if ( ODD(sts)) {
    aref = cdh_ObjidToAref( oid);
    sts = gdh_DLRefObjectInfoAttrref( &aref, (void **)&conf, &conf_dlid); 
    if ( EVEN(sts)) throw co_error(sts);
  }

  if ( ODD(sts)) {
    scan_time = 1.0 / conf->ScanTime;
  }
  else {
    scan_time = 1;
    errh_Info( "No sysmon configuration, using base frequency 1 Hz");
    oid = pwr_cNObjid;
    conf = 0;
  }

  aproc_RegisterObject( oid);

  // Open DiskSup objects
  for ( sts = gdh_GetClassListAttrRef( pwr_cClass_DiskSup, &aref);
	ODD(sts);
	sts = gdh_GetNextAttrRef( pwr_cClass_DiskSup, &aref, &aref)) {
    disksup_object *o = new disksup_object( &aref);
    objects.push_back( o);
    try {
      o->open( scan_time);
      if ( conf)
	conf->DiskSupObjects[sysmon_cnt] = aref.Objid;
      sysmon_cnt++;
    }    
    catch ( co_error& e) {
      delete o;
      objects.pop_back();
      errh_Error( "DiskSup configuration error: &s", (char *)e.what().c_str());
    }
  }
}

void rt_sysmon::close()
{
  for ( int i = objects.size() - 1; i >= 0; i--) {
    objects[i]->close();
    delete objects[i];
    objects.pop_back();
  }
  sysmon_cnt = 0;
}

void rt_sysmon::scan()
{
  pwr_tStatus osts, sts;
  errh_eSeverity severity, oseverity;

  aproc_TimeStamp();

  // Find most severe status
  sts = PWR__SRUN;
  severity = errh_Severity( sts);
  for ( int i = 0; i < (int) objects.size(); i++) {
    objects[i]->scan();
    osts = objects[i]->status();
    oseverity = errh_Severity( osts);
    if ( oseverity > severity) {
      sts = osts;
      severity = oseverity;
    }
  }
  errh_SetStatus( sts);
}

void init( qcom_sQid *qid)
{
  qcom_sQid qini;
  qcom_sQattr qAttr;
  pwr_tStatus sts;

  sts = gdh_Init("rt_sysmon");
  if ( EVEN(sts)) {
    errh_Fatal( "gdh_Init, %m", sts);
    exit(sts);
  }

  errh_Init("pwr_sysmon", errh_eAnix_sysmon);
  errh_SetStatus( PWR__SRVSTARTUP);

  if (!qcom_Init(&sts, 0, "pwr_sysmon")) {
    errh_Fatal("qcom_Init, %m", sts); 
    errh_SetStatus( PWR__SRVTERM);
   exit(sts);
  } 

  qAttr.type = qcom_eQtype_private;
  qAttr.quota = 100;
  if (!qcom_CreateQ(&sts, qid, &qAttr, "events")) {
    errh_Fatal("qcom_CreateQ, %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(sts);
  } 

  qini = qcom_cQini;
  if (!qcom_Bind(&sts, qid, &qini)) {
    errh_Fatal("qcom_Bind(Qini), %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(-1);
  }
}

int main()
{
  pwr_tStatus sts;
  rt_sysmon sysmon;
  int tmo;
  char mp[2000];
  qcom_sQid qid = qcom_cNQid;
  qcom_sGet get;
  int swap = 0;
  bool first_scan = true;

  init( &qid);

  try {
    sysmon.open();
  }
  catch ( co_error& e) {
    errh_Error( (char *)e.what().c_str());
    errh_Fatal( "rt_sysmon aborting");
    errh_SetStatus( PWR__SRVTERM);
    exit(0);
  }

  aproc_TimeStamp();
  errh_SetStatus( PWR__SRUN);

  first_scan = true;
  for (;;) {
    if ( first_scan) {
      tmo = (int) (sysmon.scantime() * 1000 - 1);
    }

    get.maxSize = sizeof(mp);
    get.data = mp;
    qcom_Get( &sts, &qid, &get, tmo);
    if (sts == QCOM__TMO || sts == QCOM__QEMPTY) {
      if ( !swap)
	sysmon.scan();
    } 
    else {
      ini_mEvent  new_event;
      qcom_sEvent *ep = (qcom_sEvent*) get.data;

      new_event.m  = ep->mask;
      if (new_event.b.oldPlcStop && !swap) {
	errh_SetStatus( PWR__SRVRESTART);
        swap = 1;
	sysmon.close();
      } else if (new_event.b.swapDone && swap) {
        swap = 0;
	sysmon.open();
	errh_SetStatus( PWR__SRUN);
      } else if (new_event.b.terminate) {
	exit(0);
      }
    }
    first_scan = false;
  }

}

void disksup_object::exec()
{
  struct statfs buf;
  int sts;

  pwr_sClass_DiskSup *o = (pwr_sClass_DiskSup *)p;

  sts = statfs( o->DiskName, &buf);
  if ( sts != 0) {
    sts = errno_GetStatus();
    if ( o->Status != sts) {
      o->Status = sts;
      printf( "Can't find disk\n");
    }
    return;
  }

  o->CurrentUse = 100.0 - 100.0 * buf.f_bfree / buf.f_blocks;

  // Check if limit is exceeded
  if ( !(o->CurrentUse < o->UsedMaxLimit)) {
    if ( o->Status != SMON__DISKHIGHLIMIT) {
      o->Status = SMON__DISKHIGHLIMIT;
      if ( o->Action & pwr_mDiskSupActionMask_Alarm) {
	pwr_tOName name;

	sts = gdh_ObjidToName( aref.Objid, name, sizeof(name), cdh_mNName);
	rt_sysmon::alarm_send( aref.Objid, o->DetectText, name, o->EventPriority);
      }
      if ( o->Action & pwr_mDiskSupActionMask_Command) {
	if ( strcmp( o->Command, "") != 0) {
	  char msg[200];

	  system( o->Command);
	  sprintf( msg, "Command %s: %s", o->DiskName, o->Command);
	  errh_Info( msg);
	}
      }
    }
  }
  else
    o->Status = SMON__SUCCESS;
}


int rt_sysmon::connect_alarm() 
{
  static int	alarm_connected = 0;
  int		sts; 
  mh_eEvent	AbortEventType = mh_eEvent_Alarm;
  mh_eEventPrio   AbortEventPrio = mh_eEventPrio_A;
  pwr_tUInt32	NoOfActMessages;

  if ( alarm_connected)
    /* We are already connected */
    return SMON__SUCCESS;

  sts = mh_ApplConnect( pwr_cNObjid, (mh_mApplFlags)0,
	   "AbortEventName",
	   AbortEventType,
	   AbortEventPrio,
	   (mh_mEventFlags)(mh_mEventFlags_Bell | mh_mEventFlags_Ack | mh_mEventFlags_Return),
	   "AbortEventText",
	   &NoOfActMessages);
  if( EVEN(sts)) return sts;

  alarm_connected = 1;
  return SMON__SUCCESS;    
}

int rt_sysmon::alarm_send( pwr_tOid  oid, 
			      char	*alarm_text,
			      char	*alarm_name,
			      int	alarm_prio ) 
{
  mh_sApplMessage 	mh_msg;
  pwr_tUInt32	  	mh_id; 
  int			sts;
  
  sts = connect_alarm();
  if ( EVEN(sts)) return sts;	
  mh_msg.Object = oid;
  mh_msg.EventFlags = (mh_mEventFlags)(mh_mEventFlags_Returned |
    mh_mEventFlags_NoObject | 
    mh_mEventFlags_Bell);
  time_GetTime( &mh_msg.EventTime);

  mh_msg.SupObject = pwr_cNObjid;
  mh_msg.Outunit = pwr_cNObjid;
  strcpy ( mh_msg.EventName , alarm_name);
  strcpy ( mh_msg.EventText , alarm_text);
  mh_msg.EventType = mh_eEvent_Alarm;
  mh_msg.SupInfo.SupType = mh_eSupType_None; 
  mh_msg.EventPrio = (mh_eEventPrio) alarm_prio;
  sts = mh_ApplMessage( &mh_id, &mh_msg);
  if( EVEN(sts)) return sts;
  
  return SMON__SUCCESS;
} 



