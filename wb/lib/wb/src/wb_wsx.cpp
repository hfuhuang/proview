/* 
 * Proview   $Id: wb_wsx.cpp,v 1.1 2007-01-04 07:29:04 claes Exp $
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
 **/

/* wb_wsx.c -- wb syntax control 
   Wb syntax control.
   This module control the syntax for objects.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "pwr_systemclasses.h"
#include "co_msgwindow.h"
#include "co_cdh.h"
#include "co_msg.h"
#include "wb_ldh.h"
#include "wb_trv.h"
#include "wb_wsx.h"
#include "wb_wsx_msg.h"
#include "wb_session.h"

static int wsx_object_count ( 
  pwr_tAttrRef	*aref,
  void		*count,
  void		*dum1,
  void		*dum2,
  void		*dum3,
  void		*dum4
);

/*_define _______________________________________________________*/

/*_Local procedues_______________________________________________________*/

static int wsx_object_count ( 
  pwr_tAttrRef	*aref,
  void		*count,
  void		*dum1,
  void		*dum2,
  void		*dum3,
  void		*dum4
)
{
	(*(int *)count)++;
	return WSX__SUCCESS;
}

/*_Backcalls for the controlled gredit module______________________________*/


/*_Methods defined for this module_______________________________________*/

/*************************************************************************
*
* Name:		wsx_error_msg()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*	Prints a error or warning message for an object and increments the 
*	errorcount or warningcount.
*
**************************************************************************/

pwr_tStatus wsx_error_msg( 
    ldh_tSesContext	sesctx,
    pwr_tStatus 	sts,
    pwr_tAttrRef	aref,
    int			*errorcount,
    int			*warningcount
)
{
  static char 	msg[256];
  int		status, size;
  pwr_tAName	name;
  char		*namep;
  FILE 		*logfile;
  
  logfile = NULL;
  
  if (EVEN(sts)) {
    
    msg_GetMsg( sts, msg, sizeof(msg));
    
    if (logfile != NULL)
      fprintf(logfile, "%s\n", msg);
    else
      printf("%s\n", msg);
    if ( cdh_ObjidIsNull( aref.Objid))
      MsgWindow::message( co_error(sts), 0, 0);
    else {
      /* Get the full hierarchy name for the node */
      status = ldh_AttrRefToName( sesctx, &aref,
				  cdh_mNName, &namep, &size);
      if( EVEN(status)) return status;
      
      strncpy( name, namep, sizeof(name));
      if (logfile != NULL)
	fprintf(logfile, "        in object  %s\n", name);
      else
	printf("        in object  %s\n", name);
      MsgWindow::message( co_error(sts), "   in object", name, aref.Objid);
    }
    if ( (sts & 2) && !(sts & 1))
      (*errorcount)++;
    else if ( !(sts & 2) && !(sts & 1))
      (*warningcount)++;
  }
  return WSX__SUCCESS;
}

pwr_tStatus wsx_error_msg_str(
    ldh_tSesContext	sesctx,
    const char 		*str,
    pwr_tAttrRef	aref,
    int			severity,
    int			*errorcount,
    int			*warningcount
)
{
  int		status, size;
  pwr_tAName	name;
  char		*namep;
  FILE 		*logfile;
  char 		msg[200];

  logfile = NULL;

  snprintf( msg, sizeof(msg), "%%WSX-%c-MSG, %s", severity, str);
  if (logfile != NULL)
    fprintf(logfile, "%s\n", msg);
  else
    printf("%s\n", msg);
  
  
  if ( cdh_ObjidIsNull( aref.Objid))
    MsgWindow::message( severity, msg, "", 0);
  else {
    /* Get the full hierarchy name for the node */
    status = ldh_AttrRefToName( sesctx, &aref,
				cdh_mNName, &namep, &size);
    if( EVEN(status)) return status;
    
    strncpy( name, namep, sizeof(name));
    if (logfile != NULL)
      fprintf(logfile, "        in object  %s\n", name);
    else
      printf("        in object  %s\n", name);
    MsgWindow::message( severity, msg, "   in object", name, aref.Objid);
  }
  if ( severity == 'E' || severity == 'F')
    (*errorcount)++;
  else if ( severity == 'W')
    (*warningcount)++;

  return WSX__SUCCESS;
}

/*************************************************************************
*
* Name:		wsx_CheckCard()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext sesctx	I	ldh session context.
* pwr_tObjid	objid		I	card objid.
*
* Description:
*	Check the syntax of a card.
**************************************************************************/

pwr_tStatus wsx_CheckCard( 
	ldh_tSesContext	sesctx,
	pwr_tAttrRef	aref,
	int		*errorcount,
	int		*warningcount,
	wsx_mCardOption options
)
{
	int		sts;
	int		size;
	char		*buf_ptr;
	pwr_tUInt16	chan_max;
	int		chan_number_array[256];
	int		chan_count;
	pwr_tUInt16 	number;
	pwr_tObjid	chan_objid;
	pwr_tClassId	cid;

	/* Check ErrorSoftLimit */
	sts = ldh_GetAttrObjectPar( sesctx,
			&aref, 
			"RtBody",
			"ErrorSoftLimit",
			(char **)&buf_ptr, &size);
	if (EVEN(sts)) return sts;
	if ( *(int *) buf_ptr == 0)
	  wsx_error_msg( sesctx, WSX__CARDERRSOFTLIM, aref, errorcount, warningcount);
	free((char *) buf_ptr);
	
	/* Check ErrorHardLimit */
	sts = ldh_GetAttrObjectPar( sesctx,
			&aref, 
			"RtBody",
			"ErrorHardLimit",
			(char **)&buf_ptr, &size);
	if (EVEN(sts)) return sts;
	if ( *(int *) buf_ptr == 0)
	  wsx_error_msg( sesctx, WSX__CARDERRHARDLIM, aref, errorcount, warningcount);
	free((char *) buf_ptr);

	/* Get MaxNoOfChannels */
	sts = ldh_GetAttrObjectPar( sesctx,
			&aref, 
			"RtBody",
			"MaxNoOfChannels",
			(char **)&buf_ptr, &size);
	if (EVEN(sts)) return sts;
	
	chan_max = *(pwr_tUInt16 *) buf_ptr;
	free((char *) buf_ptr);

	if ( chan_max > 256)
	{
	  wsx_error_msg( sesctx, WSX__MAXCHAN, aref, errorcount, warningcount);
	  return WSX__SUCCESS;
	}
	/* Check that Number in channel-objects are unique within the card */
	memset( chan_number_array, 0, sizeof( chan_number_array));
	chan_count = 0;
	sts = ldh_GetChild( sesctx, aref.Objid, &chan_objid);
	while (ODD(sts))
	{
	  sts = ldh_GetObjectClass ( sesctx, chan_objid, &cid);
	  if ( EVEN(sts)) return sts;
	  switch ( cid)
	  {
	    case pwr_cClass_ChanDi:
	    case pwr_cClass_ChanDo:
	    case pwr_cClass_ChanAi:
	    case pwr_cClass_ChanAit:
	    case pwr_cClass_ChanAo:
	      chan_count++;
	      sts = ldh_GetObjectPar( sesctx,
			chan_objid, 
			"RtBody",
			"Number",
			(char **)&buf_ptr, &size);
	      if (EVEN(sts)) return sts;
	      number = * (pwr_tUInt16 *) buf_ptr;
	      free((char *) buf_ptr);
	      /* Check than number is within limits */
	      if ( number >= chan_max)
	      {
	        wsx_error_msg( sesctx, WSX__NUMRANGE, cdh_ObjidToAref(chan_objid), 
			       errorcount, warningcount);
	        break;
	      }
	      if ( chan_number_array[ number])
	        /* Number is occupied */
	        wsx_error_msg( sesctx, WSX__NUMNOTUNIQUE, cdh_ObjidToAref(chan_objid), 
			       errorcount, warningcount);
	      else
	        chan_number_array[ number] = 1;
	      break;
	    default:
	      wsx_error_msg( sesctx, WSX__MISPLACED, aref, errorcount, warningcount);
	  }
	  sts = ldh_GetNextSibling( sesctx, chan_objid, &chan_objid);
	}
	if ( chan_count > chan_max)
	  wsx_error_msg( sesctx, WSX__CHANCOUNT, aref, errorcount, warningcount);
	
	return WSX__SUCCESS;
}

/*************************************************************************
*
* Name:		wsx_CheckCoCard()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext sesctx	I	ldh session context.
* pwr_tObjid	objid		I	card objid.
*
* Description:
*	Check the syntax of a co card.
**************************************************************************/

pwr_tStatus wsx_CheckCoCard( 
	ldh_tSesContext	sesctx,
	pwr_tAttrRef	aref,
	int		*errorcount,
	int		*warningcount	
)
{
	int		sts;
	int		size;
	char		*buf_ptr;
	pwr_tUInt16	chan_max;
	int		chan_count;
	pwr_tObjid	chan_objid;
	pwr_tClassId	cid;

	
	/* Check ErrorSoftLimit */
	sts = ldh_GetAttrObjectPar( sesctx,
			&aref, 
			"RtBody",
			"ErrorSoftLimit",
			(char **)&buf_ptr, &size);
	if (EVEN(sts)) return sts;
	if ( *(int *) buf_ptr == 0)
	  wsx_error_msg( sesctx, WSX__CARDERRSOFTLIM, aref, errorcount, warningcount);
	free((char *) buf_ptr);
	
	/* Check ErrorHardLimit */
	sts = ldh_GetAttrObjectPar( sesctx,
			&aref, 
			"RtBody",
			"ErrorHardLimit",
			(char **)&buf_ptr, &size);
	if (EVEN(sts)) return sts;
	if ( *(int *) buf_ptr == 0)
	  wsx_error_msg( sesctx, WSX__CARDERRHARDLIM, aref, errorcount, warningcount);
	free((char *) buf_ptr);

	/* Get MaxNoOfChannels */
	sts = ldh_GetAttrObjectPar( sesctx,
			&aref, 
			"RtBody",
			"MaxNoOfCounters",
			(char **)&buf_ptr, &size);
	if (EVEN(sts)) return sts;
	
	chan_max = *(pwr_tUInt16 *) buf_ptr;
	free((char *) buf_ptr);

	chan_count = 0;
	sts = ldh_GetChild( sesctx, aref.Objid, &chan_objid);
	while (ODD(sts))
	{
	  sts = ldh_GetObjectClass ( sesctx, chan_objid, &cid);
	  if ( EVEN(sts)) return sts;
	  switch ( cid)
	  {
	    case pwr_cClass_ChanCo:
	      chan_count++;
	      break;
	    default:
	      wsx_error_msg( sesctx, WSX__MISPLACED, aref, errorcount, warningcount);
	  }
	  sts = ldh_GetNextSibling( sesctx, chan_objid, &chan_objid);
	}
	if ( chan_count > chan_max)
	  wsx_error_msg( sesctx, WSX__CHANCOUNT, aref, errorcount, warningcount);
	
	return WSX__SUCCESS;
}

pwr_tStatus wsx_CheckIoDevice( 
	ldh_tSesContext	sesctx,
	pwr_tAttrRef	aref,
	int		*errorcount,
	int		*warningcount,
	wsx_mCardOption options
)
{
  wb_session *sp = (wb_session *)sesctx;
  pwr_tMask process;
  pwr_tOid thread;

  wb_attribute a = sp->attribute( &aref);
  if ( !a) return a.sts();

  // Check Process
  wb_attribute process_a( a, 0, "Process");
  if ( !process_a) return process_a.sts();
    
  process_a.value( &process);
  if ( !process_a) return process_a.sts();
  if ( process == 0)
    wsx_error_msg_str( sesctx, "Process is not specified", aref, 'W', errorcount, warningcount);
  else if ( process == 1) {

    // Check thread object
    wb_attribute thread_a( a, 0, "ThreadObject");
    if ( !thread_a) return thread_a.sts();
    
    thread_a.value( &thread);
    if ( !thread_a) return thread_a.sts();
    if ( cdh_ObjidIsNull( thread))
      wsx_error_msg_str( sesctx, "ThreadObject is not specified", aref, 'E', errorcount, warningcount);
    else {
      wb_object thread_o = sp->object( thread);
      if ( !thread_o)
	wsx_error_msg_str( sesctx, "Undefined ThreadObject", aref, 'E', errorcount, warningcount);
      else if ( thread_o.cid() != pwr_cClass_PlcThread)
	wsx_error_msg_str( sesctx, "Error in ThreadObject class", aref, 'E', errorcount, warningcount);
    }
  }

  if ( options & wsx_mCardOption_ErrorLimits) {
    pwr_tUInt32 limit;

    // Check SoftLimit
    wb_attribute softlimit_a( a, 0, "ErrorSoftLimit");
    if ( !softlimit_a) return softlimit_a.sts();
    
    softlimit_a.value( &limit);
    if ( !softlimit_a) return softlimit_a.sts();
    if ( limit == 0)
      wsx_error_msg_str( sesctx, "ErrorSoftLimit is not specified", aref, 'W', errorcount, warningcount);

    // Check HardLimit
    wb_attribute hardlimit_a( a, 0, "ErrorHardLimit");
    if ( !hardlimit_a) return hardlimit_a.sts();
    
    hardlimit_a.value( &limit);
    if ( !hardlimit_a) return hardlimit_a.sts();
    if ( limit == 0)
      wsx_error_msg_str( sesctx, "ErrorHardLimit is not specified", aref, 'E', errorcount, warningcount);

  }
  return WSX__SUCCESS;
}

/*************************************************************************
*
* Name:		wsx_CheckSigChanCon()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext sesctx	I	ldh session context.
* pwr_tObjid	objid		I	card objid.
*
* Description:
*	Check SigChanCon in a signal or a channel.
**************************************************************************/

pwr_tStatus wsx_CheckSigChanCon( 
	ldh_tSesContext	sesctx,
	pwr_tAttrRef	aref,
	int		*errorcount,
	int		*warningcount	
)
{
	int	sts;
	int	size;
	pwr_tAttrRef	*con_ptr;
	int		class_error;
	pwr_tClassId	cid;
	pwr_tClassId	con_class;

	sts = ldh_GetAttrRefTid ( sesctx, &aref, &cid);
	if ( EVEN(sts)) return sts;
	
	/* Check SigChanCon */
	sts = ldh_GetAttrObjectPar( sesctx,
			&aref, 
			"RtBody",
			"SigChanCon",
			(char **)&con_ptr, &size);
	if (EVEN(sts)) return sts;

	if ( cdh_ObjidIsNull(con_ptr->Objid)) {
	  wsx_error_msg( sesctx, WSX__SIGCHANCON, aref, errorcount, warningcount);
	  free((char *) con_ptr);
	  return WSX__SUCCESS;
	}
	
	/* Check object class of connected object */
	sts = ldh_GetAttrRefTid ( sesctx, con_ptr, &con_class);
	if ( EVEN(sts)) {
	  wsx_error_msg( sesctx, WSX__SIGCHANCON, aref, errorcount, warningcount);
	  free((char *) con_ptr);
	  return WSX__SUCCESS;
	}
	class_error = 0;
	switch ( cid) {
	case pwr_cClass_Di:
	  if ( con_class != pwr_cClass_ChanDi)
	    class_error = 1;
	  break;
	case pwr_cClass_Do:
	  if ( con_class != pwr_cClass_ChanDo)
	    class_error = 1;
	  break;
	case pwr_cClass_Po:
	  if ( con_class != pwr_cClass_ChanDo)
	    class_error = 1;
	  break;
	case pwr_cClass_Ai:
	  if ( !(con_class == pwr_cClass_ChanAi ||
		 con_class == pwr_cClass_ChanAit))
	    class_error = 1;
	  break;
	case pwr_cClass_Ao:
	  if ( con_class != pwr_cClass_ChanAo)
	    class_error = 1;
	  break;
	case pwr_cClass_Co:
	  if ( con_class != pwr_cClass_ChanCo)
	    class_error = 1;
	  break;
	case pwr_cClass_Io:
	  if ( con_class != pwr_cClass_ChanIo)
	    class_error = 1;
	  break;
	case pwr_cClass_Ii:
	  if ( con_class != pwr_cClass_ChanIi)
	    class_error = 1;
	  break;
	}

	if ( class_error) {
	  wsx_error_msg( sesctx, WSX__SIGCHANCONCLASS, aref, errorcount, warningcount);
	  free((char *) con_ptr);
	  return WSX__SUCCESS;
	}

	free((char *) con_ptr);
	return WSX__SUCCESS;
}

//
// Check if an attrref attribute contains an invalid attrref
//

pwr_tStatus wsx_CheckAttrRef( ldh_tSesContext	sesctx,
			      pwr_tAttrRef	aref,
			      const pwr_tObjName attribute,
			      pwr_tCid		*cid_vect,
			      int		null_is_ok,
			      int		*errorcount,
			      int		*warningcount)
{
  pwr_tAttrRef value;
  wb_session *sp = (wb_session *)sesctx;

  wb_attribute a = sp->attribute( &aref);
  if ( !a) return a.sts();

  wb_attribute a_attr( a, 0, attribute);
  if ( !a_attr) return a_attr.sts();
    
  a_attr.value( &value);
  if ( !a_attr) return a_attr.sts();

  if ( !null_is_ok && cdh_ObjidIsNull( value.Objid)) {
    char msg[80];
    sprintf ( msg, "Attribute reference is null in \"%s\"", attribute);
    wsx_error_msg_str( sesctx, msg, aref, 'E', errorcount, warningcount);
  }
  if ( cdh_ObjidIsNotNull( value.Objid)) {
    wb_attribute a_value = sp->attribute( &value);
    if ( !a_value) {
      char msg[80];
      sprintf ( msg, "Undefined attribute reference in \"%s\"", attribute);
      wsx_error_msg_str( sesctx, msg, aref, 'E', errorcount, warningcount);
    }
    else if ( cid_vect) {
      // Check attribute reference class
      bool found = false;
      for ( int i = 0; cid_vect[i]; i++) {
	if ( cid_vect[i] == a_value.tid()) {
	  found = true;
	  break;
	}
      }
      if ( !found) {
	char msg[80];
	sprintf ( msg, "Invalid class of attribute reference in \"%s\"", attribute);
	wsx_error_msg_str( sesctx, msg, aref, 'E', errorcount, warningcount);
      }
    }
  }
  return WSX__SUCCESS;
}

//
// Check if an Z attrref attribute contains an invalid attrref and points back
//

pwr_tStatus wsx_CheckXAttrRef( ldh_tSesContext	sesctx,
			      pwr_tAttrRef	aref,
			      const pwr_tObjName attribute,
			      const pwr_tObjName back_attribute,
			      pwr_tCid		*cid_vect,
			      int		null_is_ok,
			      int		*errorcount,
			      int		*warningcount)
{
  pwr_tAttrRef value;
  pwr_tAttrRef back_aref;
  wb_session *sp = (wb_session *)sesctx;

  wb_attribute a = sp->attribute( &aref);
  if ( !a) return a.sts();

  wb_attribute a_attr( a, 0, attribute);
  if ( !a_attr) return a_attr.sts();
    
  a_attr.value( &value);
  if ( !a_attr) return a_attr.sts();

  if ( !null_is_ok && cdh_ObjidIsNull( value.Objid)) {
    char msg[80];
    sprintf ( msg, "Attribute reference is null in \"%s\"", attribute);
    wsx_error_msg_str( sesctx, msg, aref, 'E', errorcount, warningcount);
  }
  if ( cdh_ObjidIsNotNull( value.Objid)) {
    wb_attribute a_value = sp->attribute( &value);
    if ( !a_value) {
      char msg[80];
      sprintf ( msg, "Undefined attribute reference in \"%s\"", attribute);
      wsx_error_msg_str( sesctx, msg, aref, 'E', errorcount, warningcount);
      return WSX__SUCCESS;
    }
    if ( cid_vect) {
      // Check attribute reference class
      bool found = false;
      for ( int i = 0; cid_vect[i]; i++) {
	if ( cid_vect[i] == a_value.tid()) {
	  found = true;
	  break;
	}
      }
      if ( !found) {
	char msg[80];
	sprintf ( msg, "Invalid class of attribute reference in \"%s\"", attribute);
	wsx_error_msg_str( sesctx, msg, aref, 'E', errorcount, warningcount);
	return WSX__SUCCESS;
      }
    }

    // Check back attrref
    wb_attribute a_back( a_value, 0, back_attribute);
    if ( !a_back) return a_back.sts();

    a_back.value( &back_aref);
    if ( !a_back) return a_back.sts();

    if ( !(cdh_ObjidIsEqual( back_aref.Objid, aref.Objid) &&
	   back_aref.Offset == aref.Offset)) {
      char msg[80];
      sprintf ( msg, "Reference is not mutual \"%s\"", attribute);
      wsx_error_msg_str( sesctx, msg, aref, 'E', errorcount, warningcount);
      return WSX__SUCCESS;
    }
  }
  return WSX__SUCCESS;
}


/*************************************************************************
*
* Name:		wsx_CheckVolume()
*
* Type		int
*
* Type		Parameter	IOGF	Description
* ldh_tSesContext sesctx	I	ldh session context.
* pwr_tObjid	objid		I	card objid.
*
* Description:
*	Check the syntax of a volume.
**************************************************************************/

pwr_tStatus wsx_CheckVolume( 
	ldh_tSesContext	sesctx,
	pwr_tObjid	objid,
	int		*errorcount,
	int		*warningcount	
)
{
	pwr_tStatus sts;
	pwr_tUInt32 *opsys_ptr;
	pwr_tUInt32 opsys;
	pwr_tUInt32 opsys_sum;
	int size;
	int i;
	pwr_tClassId cid;
	pwr_tClassId	class_vect[2];
	int node_object_count;  
	
	sts = ldh_GetObjectClass ( sesctx, objid, &cid);
	if ( EVEN(sts)) return sts;

	/* Check number of $Node objects in the volume */
	class_vect[0] = pwr_cClass_Node;
	class_vect[1] = 0;
	node_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&node_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;

#if 0
	/* Dynamic size of valuebase objects !!! */
	int di_object_count;  
	int do_object_count;  
	int dv_object_count;  
	int ai_object_count;  
	int ao_object_count;  
	int av_object_count;  
	int co_object_count;  

	pwr_sClass_di_value_base *di_valuebase;
	pwr_sClass_do_value_base *do_valuebase;
	pwr_sClass_dv_value_base *dv_valuebase;
	pwr_sClass_ai_value_base *ai_valuebase;
	pwr_sClass_ao_value_base *ao_valuebase;
	pwr_sClass_av_value_base *av_valuebase;
	pwr_sClass_co_value_base *co_valuebase;


	/* Check number of signals objects in the volume */
	class_vect[0] = pwr_cClass_Di;
	class_vect[1] = 0;
	di_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&di_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;
	if ( di_object_count > 
	    sizeof( di_valuebase->DIValue) / sizeof( di_valuebase->DIValue[0]))
  	  wsx_error_msg( sesctx, WSX__DICOUNT, objid, errorcount, warningcount);

	class_vect[0] = pwr_cClass_Do;
	class_vect[1] = 0;
	do_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&do_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;
	if ( do_object_count > 
	    sizeof( do_valuebase->DOValue) / sizeof( do_valuebase->DOValue[0]))
  	  wsx_error_msg( sesctx, WSX__DOCOUNT, objid, errorcount, warningcount);

	class_vect[0] = pwr_cClass_Dv;
	class_vect[1] = 0;
	dv_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&dv_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;
	if ( dv_object_count > 
	    sizeof( dv_valuebase->DVValue) / sizeof( dv_valuebase->DVValue[0]))
  	  wsx_error_msg( sesctx, WSX__DVCOUNT, objid, errorcount, warningcount);

	class_vect[0] = pwr_cClass_Ai;
	class_vect[1] = 0;
	ai_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&ai_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;
	if ( ai_object_count > 
	    sizeof( ai_valuebase->AIValue) / sizeof( ai_valuebase->AIValue[0]))
  	  wsx_error_msg( sesctx, WSX__AICOUNT, objid, errorcount, warningcount);

	class_vect[0] = pwr_cClass_Ao;
	class_vect[1] = 0;
	ao_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&ao_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;
	if ( ao_object_count > 
	    sizeof( ao_valuebase->AOValue) / sizeof( ao_valuebase->AOValue[0]))
  	  wsx_error_msg( sesctx, WSX__AOCOUNT, objid, errorcount, warningcount);

	class_vect[0] = pwr_cClass_Av;
	class_vect[1] = 0;
	av_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&av_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;
	if ( av_object_count > 
	    sizeof( av_valuebase->AVValue) / sizeof( av_valuebase->AVValue[0]))
  	  wsx_error_msg( sesctx, WSX__AVCOUNT, objid, errorcount, warningcount);

	class_vect[0] = pwr_cClass_Co;
	class_vect[1] = 0;
	co_object_count = 0;
	sts = trv_get_objects_hier_class_name( sesctx, pwr_cNObjid,
		class_vect, NULL,
		&wsx_object_count,
		&co_object_count, 0, 0, 0, 0);
	if (EVEN(sts)) return sts;
	if ( co_object_count > 
	    sizeof( co_valuebase->COValue) / sizeof( co_valuebase->COValue[0]))
  	  wsx_error_msg( sesctx, WSX__COCOUNT, objid, errorcount, warningcount);
#endif
	switch (cid)
 	{
	  case pwr_cClass_RootVolume:
	  case pwr_cClass_SubVolume:

	    if ( cid == pwr_cClass_RootVolume)
	    {
	      if ( node_object_count != 1)
  	        wsx_error_msg( sesctx, WSX__NODECOUNT, cdh_ObjidToAref(objid), errorcount, warningcount);
	    }
	    else
	    {
	      if ( node_object_count != 0)
  	        wsx_error_msg( sesctx, WSX__NODECOUNT, cdh_ObjidToAref(objid), errorcount, warningcount);
	    }

	    /* Check OperatingSystem */
	    sts = ldh_GetObjectPar( sesctx,
  			objid,
			"SysBody",
			"OperatingSystem",
			(char **)&opsys_ptr, &size);
	    if ( EVEN(sts)) return sts;

	    opsys_sum = 0;
	    for ( i = 0;; i++)
	    {
    	      opsys = 1 << i;
	      opsys_sum |= opsys;
  	      if ( opsys >= pwr_mOpSys_)
  	        break;
	    }

  	    if ( *opsys_ptr & ~opsys_sum)
  	      wsx_error_msg( sesctx, WSX__OSINVALID, cdh_ObjidToAref(objid), errorcount, warningcount);

  	    free( (char *) opsys_ptr);
	    break;

	  default:
	    if ( node_object_count != 0)
  	      wsx_error_msg( sesctx, WSX__NODECOUNT, cdh_ObjidToAref(objid), errorcount, warningcount);
	}
	return WSX__SUCCESS;
}

