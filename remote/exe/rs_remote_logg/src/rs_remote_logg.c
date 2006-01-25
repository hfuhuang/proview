/* 
 * Proview   $Id: rs_remote_logg.c,v 1.1 2006-01-12 06:39:33 claes Exp $
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

/*************************************************************************
*
* 	PROGRAM		rs_remote_logg
*
*       Modifierad
*		950301	Claes Sj�fors	Skapad
*		010402	Claes Jurstrand	Qcom istf DMQ fr�n v3.0b
*		040630	Claes Jurstrand	v4.0.0
*
*
*	Funktion:
*		Tar emot transar med loggar fr�n remote och loggar p� fil.
*
**************************************************************************/

/*_Include filer_________________________________________________________*/
#ifdef __DECC
#pragma message disable GLOBALEXT
#endif

#ifdef OS_VMS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <descrip.h>
#include <libdef.h>
#include <starlet.h>
#include <lib$routines.h>
/*
#include <pams_c_entry_point.h>
#include <pams_c_process.h>
#include <pams_c_type_class.h>
#include <pams_c_return_status.h>
#include <sbs_msg_def.h>
*/
#endif

#ifdef OS_ELN
#include stdio
#include stdlib
#include string
#include math
#include float
#include descrip
#include libdef
#include lib$routines
#include starlet
#endif

#if defined OS_LYNX || defined OS_LINUX
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#endif

#include "pwr.h"
#include "pwr_baseclasses.h"
#include "pwr_remoteclasses.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "rt_gdh_msg.h"
#include "rt_hash_msg.h"
#include "rt_ini_event.h"
#include "rt_qcom.h"
#include "rt_qcom_msg.h"
#include "remote.h"
#include "rs_remote_msg.h"

/*_Globala variabler______________________________________________________*/

qcom_sQid 	remlogg_qid = {rs_pwr_logg_qix, 0};
qcom_sQattr 	remlogg_qattr;
qcom_sGet	remlogg_get;

/* Global functions________________________________________________________*/

#define ODD(a)	(((int)(a) & 1) != 0)
#define EVEN(a)	(((int)(a) & 1) == 0)
#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#ifndef __ALPHA
#define abs(Dragon) ((Dragon) >= 0 ? (Dragon) : (-(Dragon)))
#endif
#define	LogAndExit( status) \
{\
  errh_CErrLog(REM__LOGGEXIT, errh_ErrArgMsg(status), NULL);\
  exit( status);\
}

#define	LogAndReturn( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2), NULL);\
  return status2;\
}

#define	Log( status1, status2) \
{\
  errh_CErrLog(status1, errh_ErrArgMsg(status2), NULL);\
}

#define LOGG_FILE_EXT ".log"
#define LOGG_MAX_SIZE 32000


typedef struct {
	pwr_tObjid		objid;
	pwr_sClass_LoggConfig	*loggconf;
	gdh_tDlid		subid;
	FILE			*outfile;
	int			file_open;
	int			wait_count;
	} logg_t_loggconf_list;

typedef struct {
	logg_t_loggconf_list		*loggconflist;
	int				loggconf_count;
	} *logg_ctx;

/*_Local functions________________________________________________________*/
static pwr_tStatus	logg_loggconflist_add( 
			logg_ctx			loggctx,
			pwr_tObjid 			objid, 
			logg_t_loggconf_list 		**loggconflist,
			int				*loggconflist_count);
static int	logg_get_filename(
			char	*inname,
			char	*outname,
			char	*ext);
static pwr_tStatus	logg_open_file( logg_t_loggconf_list	*conflist_ptr,
					int			first_time);
static pwr_tStatus	logg_print(	logg_ctx	loggctx,
				   	pwr_tUInt32	ident,
				   	char		*msg);
static pwr_tStatus	logg_get_message( 	logg_ctx	loggctx,
						pwr_tUInt32	*ident,
						char		**msg);
static pwr_tStatus	logg_init( logg_ctx	loggctx);


/****************************************************************************
* Name:		exit_hdlr()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Called at exit
*
**************************************************************************/
void			exit_hdlr()
{

	gdh_DLUnrefObjectInfoAll();
	errh_Info("Exiting\n");
	return;
}


/****************************************************************************
* Name:		interrupt_hdlr()
*
* Type		void
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Called at interrupt
*
**************************************************************************/
void			interrupt_hdlr()
{
	exit(0);
	return;
}


/****************************************************************************
* Name:		logg_loggconflist_add()
*
* Type		pwr_tStatus
*
* Type		Parameter	IOGF	Description
*
* Description:
*		Add a conversion config object to the list.
*
**************************************************************************/
static pwr_tStatus	logg_loggconflist_add( 
			logg_ctx			loggctx,
			pwr_tObjid 			objid, 
			logg_t_loggconf_list 		**loggconflist,
			int				*loggconflist_count)
{
	logg_t_loggconf_list	*loggconflist_ptr;
	logg_t_loggconf_list	*new_loggconflist;
	pwr_sAttrRef		attrref;
	int			i;
	pwr_tStatus		sts;
	pwr_sClass_LoggConfig	*loggconf;

	/* Syntax check */
	sts = gdh_ObjidToPointer ( objid, (pwr_tAddress *) &loggconf);
	if ( EVEN(sts)) return sts;

	/* Check that there is some filename */
	if ( !strcmp( loggconf->LoggFile, ""))
	  return REM__LOGGFILE;

	/* Check that identity is unique */
	loggconflist_ptr = *loggconflist;
	for ( i = 0; i < *loggconflist_count; i++)
	{
	  if ( loggconflist_ptr->loggconf->Identity == loggconf->Identity)
	    return REM__DUPLIDENT;
	  loggconflist_ptr++;
	}

	if ( *loggconflist_count == 0)
	{
	  *loggconflist = calloc( 1 , sizeof(logg_t_loggconf_list));
	  if ( *loggconflist == 0)
	    return REM__NOMEMORY;
	}
	else
	{
	  new_loggconflist = calloc( *loggconflist_count + 1,
			sizeof(logg_t_loggconf_list));
	  if ( new_loggconflist == 0)
	    return REM__NOMEMORY;
	  memcpy( new_loggconflist, *loggconflist, 
			*loggconflist_count * sizeof(logg_t_loggconf_list));
	  free( *loggconflist);
	  *loggconflist = new_loggconflist;
	}
	loggconflist_ptr = *loggconflist + *loggconflist_count;
	loggconflist_ptr->objid = objid;

	/* Direct link to the cell */
	memset( &attrref, 0, sizeof(attrref));
	attrref.Objid = objid;
	sts = gdh_DLRefObjectInfoAttrref ( &attrref,
		(pwr_tAddress *) &loggconflist_ptr->loggconf,
		&loggconflist_ptr->subid);
	if ( EVEN(sts)) return sts;

	(*loggconflist_count)++;

	return REM__SUCCESS;
}


/*************************************************************************
*
* Name:		logg_get_filename
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Adderar extention till filname om det saknas.
*
**************************************************************************/
static int	logg_get_filename(
			char	*inname,
			char	*outname,
			char	*ext)
{
	char	*s;
	char	*s2;
	char	timestr[80];
	char	comp_timestr[80];
	pwr_tTime time;

	strcpy( outname, inname);

	/* Look for extention in filename */
	if ( ext != NULL)
	{
	  s = strrchr( outname, ':');
	  if ( s == 0)
	    s = outname;

	  s2 = strrchr( s, '>');
	  if ( s2 == 0)
	  {
	    s2 = strrchr( s, ']');
	    if ( s2 == 0)
	      s2 = s;
	  }

	  s = strrchr( s2, '.');
	  if ( s != 0)
	    *s = 0;
	  strcat( outname, ext);
	}

#if defined OS_LYNX || defined OS_LINUX
	  /* Get current time to use as "version number" */

          clock_gettime( CLOCK_REALTIME, &time);
	  time_AtoAscii( &time, time_eFormat_ComprDateAndTime, timestr, 
			sizeof(timestr));
	  comp_timestr[0] = '.';
	  memcpy(&comp_timestr[1], &timestr[0], 2);
	  memcpy(&comp_timestr[3], &timestr[3], 2);
	  memcpy(&comp_timestr[5], &timestr[6], 2);
	  memcpy(&comp_timestr[7], &timestr[9], 2);
	  memcpy(&comp_timestr[9], &timestr[12], 2);
	  memcpy(&comp_timestr[11], &timestr[15], 2);
	  comp_timestr[13] = 0;
//	  strcat(outname, comp_timestr);

#endif

	return REM__SUCCESS;
}


/*************************************************************************
*
* Name:		logg_open_file
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Open current logg file and write a file header.
*
**************************************************************************/

static pwr_tStatus	logg_open_file( logg_t_loggconf_list	*conflist_ptr,
					int			first_time)
{
	int			csts;
	char			filename[80];
	pwr_tTime		time;
	char			timestr[80];

	if ( !first_time)
	{
	  /* Check if it's time for a new try to open the file */
	  conflist_ptr->wait_count++;
	  if (conflist_ptr->wait_count < 10)
	    return REM__SUCCESS;
	  conflist_ptr->wait_count = 0;
	}

	/* Open file */
	logg_get_filename( conflist_ptr->loggconf->LoggFile,
			filename, LOGG_FILE_EXT);

#if defined OS_LYNX || defined OS_LINUX
	conflist_ptr->outfile = fopen( filename, "a+");
#else
	conflist_ptr->outfile = fopen( filename, "w+", "shr=get");
#endif
	if (conflist_ptr->outfile != NULL)
	{
	  /* Write a file header */
          clock_gettime( CLOCK_REALTIME, &time);
	  time_AtoAscii( &time, time_eFormat_DateAndTime, timestr, 
		sizeof(timestr));
	  csts = fprintf( conflist_ptr->outfile,
			"RemLogg file opened at %s\n\n", timestr);
	  if (csts >= 0)
	  {
	    conflist_ptr->loggconf->FileOpenCount++;
#if defined OS_ELN || defined OS_VMS
	    fgetname( conflist_ptr->outfile, filename);
#endif
	    errh_CErrLog( REM__LOGGFILEOPEN, errh_ErrArgAF(filename), NULL);
	    conflist_ptr->file_open = 1;
	    return REM__SUCCESS;
	  }
	  else
	  {
	    fclose( conflist_ptr->outfile);
	  }
	}
	errh_CErrLog( REM__LOGGFILE, errh_ErrArgAF(filename), NULL);
	return REM__LOGGFILE;
}


/*************************************************************************
*
* Name:		logg_print
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Skrivning av logg meddelande p� fil.
*
**************************************************************************/

static pwr_tStatus	logg_print(	logg_ctx	loggctx,
				   	pwr_tUInt32	ident,
				   	char		*msg)
{
	int			csts;
	int			found;
	int			i;
	logg_t_loggconf_list	*conflist_ptr;
	logg_t_loggconf_list	*garbage_conflist_ptr;
	int			garbage_loggconf_found;

	if ( loggctx->loggconf_count == 0)
	  /* Not configured */
	  return REM__SUCCESS;

	/* Identify the identity */
	found = 0;
	garbage_loggconf_found = 0;
	conflist_ptr = loggctx->loggconflist;
	for ( i = 0; i < loggctx->loggconf_count; i++)
	{
	  if ( conflist_ptr->loggconf->Identity == 0)
	  {
	    /* This is the garbage loggfile */
	    garbage_conflist_ptr = conflist_ptr;
	    garbage_loggconf_found = 1;
	  }
	  if ( conflist_ptr->loggconf->Identity == ident)
	  {
	    found = 1;
	    break;
	  }
	  conflist_ptr++;
	}
	if ( !found)
	{
	  if ( garbage_loggconf_found)
	  {
	    /* Write the logg on the garbage file */
	    conflist_ptr = garbage_conflist_ptr;
	  }
	  else
	  {
	    errh_CErrLog( REM__LOGGIDEN, NULL);
	    return REM__SUCCESS;
	  }
	}
/*	csts = fwrite( msg, min( strlen(msg),LOGG_MAX_SIZE), 1, 
		conflist_ptr->outfile);
	if (csts == 0)
*/
	csts = fprintf( conflist_ptr->outfile, "%s\n", msg);
	if ( csts < 0)
	{
	  /* File error, close file and try to open it later */
	  errh_CErrLog( REM__LOGGWRITE, NULL);
	  fclose( conflist_ptr->outfile);
	  conflist_ptr->file_open = 0;
	}
	else
	{
	  csts = fflush( conflist_ptr->outfile);
	  if ( csts != 0)	
	  {
	    /* File error, close file and try to open it later */
	    errh_CErrLog( REM__LOGGWRITE, NULL);
	    fclose( conflist_ptr->outfile);
	    conflist_ptr->file_open = 0;
	  }
	  else
	    conflist_ptr->loggconf->LoggCount++;
	}

	return REM__SUCCESS;
}


/*************************************************************************
*
* Name:		logg_get_message
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Reads message from qcom with timeout
*
**************************************************************************/

static pwr_tStatus	logg_get_message( 	logg_ctx	loggctx,
						pwr_tUInt32	*ident,
						char		**msg)
{
	pwr_tStatus	sts;
	unsigned int 	timeout = 200; /* 200 ms */

        memset(&remlogg_get, 0, sizeof(remlogg_get));
        remlogg_get.maxSize = LOGG_MAX_SIZE;

        qcom_Get(&sts, &remlogg_qid, &remlogg_get, timeout);

	if ( sts == QCOM__TMO)
	  return REM__TIMEOUT;
	else if (EVEN(sts) || (sts == QCOM__QEMPTY)) return sts;
	else
	{
	  if (remlogg_get.type.b == qcom_eBtype_event) {
	    qcom_sEvent  *ep = (qcom_sEvent*) remlogg_get.data;
	    ini_mEvent    new_event;
	    
	    if (remlogg_get.type.s == qcom_cIini) {
	      new_event.m = ep->mask;
	      if (new_event.b.terminate) {
	        exit(0);
	      }  
	    }
	  
	  }
	  else {
	    *ident = * (pwr_tUInt32 *) remlogg_get.data;
	    *msg = (char *) remlogg_get.data + sizeof(pwr_tUInt32);
	  }
	}
	return REM__SUCCESS;
}

/*************************************************************************
*
* Name:		logg_free_message
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Frees qcom message
*
**************************************************************************/

static pwr_tStatus	logg_free_message( 	void)
{
	pwr_tStatus sts;

	if (remlogg_get.data != 0) qcom_Free(&sts, remlogg_get.data);

	if (EVEN(sts)) return sts;

	return REM__SUCCESS;
}


/*************************************************************************
*
* Name:		logg_init
*
* Typ		int
*
* Typ		Parameter	IOGF	Beskrivning
*
* Beskrivning: 
*	Initiering av loggfunktionen.
*
**************************************************************************/

static pwr_tStatus	logg_init( logg_ctx	loggctx)
{
	pwr_tStatus	sts;
	pwr_tObjid	objid;
	logg_t_loggconf_list	*conflist_ptr;
	int		i;

	/* Get the logg config objects on this node */
	sts = gdh_GetClassList ( pwr_cClass_LoggConfig, &objid);
	while ( ODD(sts))
	{
          /* Store and direct link the LoggConfig objects */
          sts = logg_loggconflist_add( loggctx, objid,
		&loggctx->loggconflist,
		&loggctx->loggconf_count);
	  if (EVEN(sts)) Log( REM__CONFINIT, sts);

	  sts = gdh_GetNextObject( objid, &objid);
	}

	/* Open the files */
	conflist_ptr = loggctx->loggconflist;
	for ( i = 0; i < loggctx->loggconf_count; i++)
	{
	  sts = logg_open_file( conflist_ptr, 1);
	  conflist_ptr++;
	}

	return REM__SUCCESS;
}

int main()
{
	logg_ctx	loggctx;
	pwr_tStatus	sts;
	pwr_tUInt32	ident;
	char		*msg;
	logg_t_loggconf_list	*conflist_ptr;
	int		i;

#if defined OS_LYNX || defined OS_LINUX

	/* Exit handler */

	atexit(&exit_hdlr);
	signal(SIGINT, interrupt_hdlr);

#endif

	sts = gdh_Init("rs_remote_logg");
	if (EVEN(sts)) LogAndExit(sts);

	/* Errh init */

	errh_Init("rs_remote_logg", 0);

	/* Initialize qcom que attributes */

	remlogg_qattr.type = qcom_eQtype_private;
	remlogg_qattr.quota = 100;

	/* Delete the queue if it exists */

        qcom_DeleteQ(&sts, &remlogg_qid);
	
	/* Create the remlogg queue */

        if (!qcom_CreateQ(&sts, &remlogg_qid, &remlogg_qattr, "Logg")) {
	  LogAndExit(sts);
        }

	/* Bind it to rt_ini event-queue */
	
        if (!qcom_Bind(&sts, &remlogg_qid, &qcom_cQini)) {
          errh_Fatal("qcom_Bind, %m", sts);
//          errh_SetStatus( PWR__SRVTERM);
          exit(-1);
        }

	loggctx = calloc( 1 , sizeof( *loggctx));
	if ( loggctx == 0 ) LogAndExit( REM__NOMEMORY);

	sts = logg_init( loggctx);
	if ( EVEN(sts)) LogAndExit(sts);
	
	for (;;)
	{
	  /* Get logg message */
	  sts = logg_get_message( loggctx, &ident, &msg);
	  if ( EVEN(sts))
	  {
	    Log( REM__LOGGRCV, sts);
	  }
	  else if ( sts != REM__TIMEOUT)
	  {
	    logg_print( loggctx, ident, msg);
	    sts = logg_free_message();
	  }

	  /* Check if its time to open any file... */
	  conflist_ptr = loggctx->loggconflist;
	  for ( i = 0; i < loggctx->loggconf_count; i++)
	  {
	    if ( conflist_ptr->loggconf->NewVersion)
	    {
	      conflist_ptr->loggconf->NewVersion = 0;
	      if ( conflist_ptr->file_open)
	      {
	        fclose( conflist_ptr->outfile);
	        conflist_ptr->file_open = 0;
	        sts = logg_open_file( conflist_ptr, 1);
	      }
	    }
	    else if ( !conflist_ptr->file_open)
	    {
	      sts = logg_open_file( conflist_ptr, 0);
	    }
	    conflist_ptr++;
	  }
	}
}