/* 
 * Proview   $Id: rt_bck.c,v 1.3 2005-09-01 14:57:55 claes Exp $
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

/* rt_bck.c
   This module contains the methods that can be called by
   a user program, to control the backup.  */

#if defined OS_LYNX || defined OS_LINUX
#include <stdio.h>
#include "pwr.h"

void bck_ForceBackup (void **context)
{
    printf("bck_ForceBackup is not implemented\n");
}

pwr_tUInt32 bck_WaitBackup (
		void *context,
		pwr_tBoolean timeout)
{
    printf("bck_WaitBackup is not implemented\n");
    return 2;
}

#else

#ifdef OS_ELN
# include $vaxelnc
#include stdio
#include stdlib
#include descrip
#include string
#include errno
#include time
#include "sys$library:ssdef.h"
#endif

#ifdef OS_VMS
#include <stdio.h>
#include <stdlib.h>
#include <descrip.h>
#include <string.h>
#include <errno.h>
#include <ssdef.h>
#include <starlet.h>
#include <lib$routines.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "rt_bckdef.h"

#ifdef OS_ELN
static pwr_tBoolean areas_mapped = FALSE;
#endif

#ifdef OS_VMS
static pwr_tBoolean timed_out = FALSE;

static void astrtn ()
{
  timed_out = TRUE;
}
#endif


/************************************************************************
*
* Name: bck_ForceBackup
*
* Type:	void
*
* Type		Parameter	IOGF	Description
* void *	context		IO	a pointer to a context block
*                                       that gets allocated by this routine
*					If NULL is supplied, no such
*					allocation takes place
* Description:	
*	The routine forces both backup cycles to be activated. This
*	activation is done in a way so the backup lists get rebuilt.
*	
*************************************************************************/

void bck_ForceBackup (
		void **context)
{
  pwr_tInt32 sts;
  pwr_tTime *t;
#ifdef OS_VMS
  $DESCRIPTOR (efcname, BCK_EFC_NAME);
#endif

/*
 * Initialize
 */

#ifdef OS_ELN
  if (!areas_mapped) {
    BCK_MAP_AREAS;
    areas_mapped = TRUE;
  }
#endif

/*
 * Build the context block (i.e. current time)
 */

  if (context != NULL) {
    t = malloc (sizeof *t);
    time_GetTime(t);
    *context = t;
  }

/*
 * Kick the backup processes
 */

#ifdef OS_ELN
  ker$signal (NULL, bck_forced_activation);
#else
  sts = sys$ascefc (BCK_EFC, &efcname, 0, 0);
  if (EVEN (sts)) lib$signal (sts);
  sts = sys$setef (BCK_ACTIVATE);
  if (EVEN (sts)) lib$signal (sts);
#endif

} /* bck_ForceBackup */

/************************************************************************
*
* Name: bck_WaitBackup
*
* Type:	pwr_tUInt32 - status
*
* Type		Parameter	IOGF	Description
* void *	context		I	a pointer to a context block
*                                       that gets deallocated by this routine
*					This argument can optionally be
*					passed as NULL.
* pwr_tBoolean	timeout		I	Timeout wanted. This implies that
*					the WaitBackup function returns
*					a timeout error code, after 2 times
*					the slow backupcycle, if no
*					backup was done.
* Description:	
*	The routine waits for both backup cycles to be done, WITH every
*	backup object included.
*	
*************************************************************************/

pwr_tUInt32 bck_WaitBackup (
		void *context,
		pwr_tBoolean timeout)
{
  pwr_tUInt32 sts;
  pwr_tObjid objid;
  pwr_tInt32 c;
  pwr_tTime t;
  pwr_tVaxTime tmo;
  pwr_tVaxTime tmptime;
  pwr_sClass_Backup_Conf *backup_confp;	/* Backup_Conf object pointer */
  $DESCRIPTOR (timeunitdsc, "0 0:0:0.1");	/* 0.1 second units */
  int cycletime;

#ifdef OS_ELN
  pwr_tInt32 res;
  pwr_tVaxTime *tmop;
#endif

#ifdef OS_VMS
  $DESCRIPTOR (efcname, BCK_EFC_NAME);
#endif

/*
 * Initialize
 */

#ifdef OS_ELN
  if (!areas_mapped) {
    BCK_MAP_AREAS;
    areas_mapped = TRUE;
  }
#endif

/*
 * Find the local Backup_Conf object
 */

  sts = gdh_GetClassList (pwr_cClass_Backup_Conf, &objid);
  while (ODD (sts)) {
    sts = gdh_ObjidToPointer (objid, (pwr_tAddress *)&backup_confp);
    if (ODD (sts)) break;
    sts = gdh_GetNextObject (objid, &objid);
  }
  if (EVEN (sts)) return sts;		/* Something wrong, quit */

/*
 * Pick up argument information
 */

  if (context == NULL) time_GetTime(&t);
  else {
    t = *(pwr_tTime *)context;
    free (context);
  }

#ifdef OS_ELN
  tmop = NULL;
#else
  timed_out = FALSE;  
  sts = sys$ascefc (BCK_EFC, &efcname, 0, 0);
  if (EVEN (sts)) lib$signal (sts);			/* BUG */
#endif

  if (timeout) {
    cycletime = backup_confp->CycleSlow * 2;
    if (cycletime == 0) cycletime = BCK_DEFAULT_SLOW * 2;

#ifdef OS_ELN
    tmo = eln$time_value (&timeunitdsc);
#else
    sts = sys$bintim (&timeunitdsc, &tmo);
    if (EVEN (sts)) lib$signal (sts);		/* BUG, should not happen */
#endif

    lib$mult_delta_time (
		&cycletime,		/* multiplier */
		&tmo);			/* delta_time (modified) */
    sys$gettim (&tmptime);
    lib$add_times (&tmo, &tmptime, &tmo); /* Make absolute time */

#ifdef OS_ELN
    tmop = &tmo;
#else
    sts = sys$setimr (BCK_WRITE_DONE, &tmo, &astrtn, 4711, 0);
    if (EVEN (sts)) lib$signal (sts);			/* BUG */
#endif
  }

/*
 * Loop, and wait for things to happen
 */

  while (TRUE) {
#ifdef OS_ELN
    ker$clear_event (NULL, bck_write_done);
    ker$wait_any (NULL, &res, tmop, bck_write_done);

    /* Check for timeout */

    if (res == 0) return SS$_TIMEOUT;
#else

    sts = sys$clref (BCK_WRITE_DONE);
    if (EVEN (sts)) lib$signal (sts);			/* BUG */
    sts = sys$waitfr (BCK_WRITE_DONE);
    if (EVEN (sts)) lib$signal (sts);			/* BUG */

    /* Check for timeout */

    if (timed_out) return SS$_TIMEOUT;

#endif

    /* Check if both cycles done */

    if (time_Acomp(&backup_confp->ObjTimeSlow, &t) < 0) 
        continue;
    if (time_Acomp(&backup_confp->ObjTimeFast, &t) < 0) 
        continue;

    break;

  } /* Loop */

#ifdef OS_VMS
  sys$cantim (4711, 0);
#endif

  return 1;	/* Done. */

} /* bck_WaitBackup */

#endif










