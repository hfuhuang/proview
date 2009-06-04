/* 
 * Proview   $Id: rt_mh_util.c,v 1.2 2005-09-01 14:57:56 claes Exp $
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

#if defined OS_ELN
# include $vaxelnc
# include $kernelmsg
# include $kerneldef
# include descrip
# include ssdef
# include stdio
#elif defined OS_VMS
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <starlet.h>
# include <descrip.h>
# include <ssdef.h>
#elif defined OS_LYNX
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <semaphore.h>
#elif defined OS_LINUX
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <fcntl.h>
# include "rt_semaphore.h"
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "co_time.h"
#include "co_errno.h"
#include "co_xdr.h"
#include "rt_errh.h"
#include "pwr_baseclasses.h"
#include "rt_mh_outunit.h"
#include "rt_mh_def.h"
#include "rt_mh_util.h"
#include "rt_qcom.h"

/* Local defines */

#define PWR_WAIT_MH 96

/* Local definitions */

#if defined OS_ELN
  typedef struct {
    pwr_tBoolean isStarted;
  } sInfo;

  static AREA lMhUtilSyncArea;
  static sInfo *lInfo;
#endif


#if defined OS_LYNX
# define SEM_NAME "/pwr_mh_sem_"
  static sem_t *sem = (sem_t *)-1;
#elif defined OS_LINUX
# define SEM_NAME "/tmp/pwr_mh_sem_"
  static sem_t *sem = (sem_t *)-1;
#endif


/* Local function prototypes */

static pwr_tStatus sendMessage(qcom_sQid *, mh_sHead*);


#if defined OS_VMS || defined OS_ELN

static pwr_tStatus
map ()
{
  pwr_tStatus sts;
#ifdef OS_ELN
  static $DESCRIPTOR(name, "MH_UTL_SYNC_AREA");

#elif defined OS_VMS
  static $DESCRIPTOR(name, "PWR_WAIT_EF");
#endif

  static pwr_tBoolean isMapped = FALSE;

  /* map MH_UTL_SYNC_AREA */


#ifdef OS_ELN
  if (isMapped) return;
  ker$create_area_event(&sts, &lMhUtilSyncArea,
    &lInfo, sizeof(*lInfo), &name, EVENT$CLEARED, NULL);
  if (sts != KER$_AREA_EXISTS)
    memset(lInfo, 0, sizeof(lInfo));
  isMapped = TRUE;

#elif defined OS_VMS
  sts = sys$ascefc(PWR_WAIT_MH, &name, 0, 0);

#endif

  return (sts);
}
#endif /* OS_ELN || OS_VMS */


#if defined OS_LYNX || defined OS_LINUX

static char *
SemName ()
{
  static char id[32];
  static char *str = NULL;

  if (str == NULL) {
    if ((str = getenv(pwr_dEnvBusId)) == NULL)
      return NULL;
    sprintf(id, "%s%.*s", SEM_NAME, (int)(sizeof(id) - strlen(SEM_NAME) - 1), str);
    str = id;       
  }
  return str;
}

static pwr_tUInt32
map ()
{
  char *name;
  int  oflags = 0;

  if (sem != (sem_t *) -1)
    return 1;
  if ((name = SemName()) == NULL)
    return 2;
#if defined OS_LYNX
  sem = sem_open(name, oflags);
#elif defined OS_LINUX
  sem = posix_sem_open(name, oflags);
#endif
  if (sem == (sem_t *) -1) {
    errh_Error("rt_mh_utl: map. sem_open(\"%s\") failed, %m", name, errno_GetStatus());
    return 2;
  }
  return 1;
}

static pwr_tUInt32
unmap ()
{

  if (sem == (sem_t *) -1)
    return 1;
#if defined OS_LYNX
  if (sem_close(sem) == -1) {
#elif defined OS_LINUX
  if (posix_sem_close(sem) == -1) {
#endif
    perror("rt_mh_utl: unmap. sem_close failed, ");
    sem = (sem_t *) -1; /* If you can't close it, it cannot be valid" */
    return 2;
  }
  sem = (sem_t *) -1;

  return 1;
}
#endif

static pwr_tStatus
sendMessage (
  qcom_sQid *target,
  mh_sHead *head
)
{
  pwr_tStatus sts;
  qcom_sPut put;


  put.data =(char *) head;
  put.type.b = mh_cMsgClass;
  put.type.s = 0;
  put.reply.qix = 0;
  put.reply.nid = 0;
  put.size = sizeof(*head);

  qcom_Put(&sts, target, &put);

  return sts;
}


/* Create the event semaphore.  */ 

pwr_tStatus
mh_UtilCreateEvent ()
{
#if defined OS_LYNX || OS_LINUX
  char *name;
  int  oflags = O_CREAT | O_EXCL;
  mode_t mode = S_IRWXU | S_IRWXG;

  if (sem != (sem_t *) -1)
    return 2;

  name = SemName();
  if (name == NULL)
    return 2;

# if defined OS_LYNX
  sem_unlink(name); /* Don't care about status */
  sem = sem_open(name, oflags, mode, 0);
  if (sem == (sem_t *) -1)
    return 2;

  sem_close(sem);

# elif defined OS_LINUX
  posix_sem_unlink(name); /* Don't care about status */
  sem = posix_sem_open(name, oflags, mode, 0);

  if (sem == (sem_t *) -1)
    return 2;

  posix_sem_close(sem);
# endif

  sem = (sem_t *) -1;
#endif

  return 1;
}

/*  Destroy the event semaphore.  */ 

pwr_tStatus
mh_UtilDestroyEvent ()
{
#if defined OS_LYNX || defined OS_LINUX

  char *name = SemName();

  if (name == NULL)
    return 2;

  if (sem != (sem_t *) -1)
# if defined OS_LYNX
    sem_close(sem);
# elif defined OS_LINUX
    posix_sem_close(sem);
# endif
  sem = (sem_t *) -1;

# if 1
#   if defined OS_LYNX
  if (sem_unlink(name) == -1) {
    perror("mh_UtilDestroyEvent: sem_unlink");
    return 2;
  }  
#   elif defined OS_LINUX
  if (posix_sem_unlink(name) == -1) {
    perror("mh_UtilDestroyEvent: sem_unlink");
    return 2;
  }  
#   endif
# endif
#endif
  return 1;
}
/* Is the event monitor to started. */ 

pwr_tBoolean
mh_UtilIsStartedMh ()
{
  pwr_tStatus sts;
#ifdef OS_VMS
  pwr_tUInt32 state;
#endif

  sts = map();

#ifdef OS_ELN

  if (EVEN(sts))
    return FALSE;

  return lInfo->isStarted;

#elif defined OS_VMS

  if (EVEN(sts))
    return FALSE;

  sts = sys$readef(PWR_WAIT_MH, &state);
  return ((pwr_tBoolean) (sts == SS$_WASSET));

#elif defined OS_LYNX
  if (EVEN(sts))
    return FALSE;
  sts = sem_trywait(sem);
  if (sts == 0) {
    sem_post(sem);
    unmap();
  } 

  return (sts == 0);

#elif defined OS_LINUX
  if (EVEN(sts))
    return FALSE;
  sts = posix_sem_trywait(sem);
  if (sts == 0) {
    posix_sem_post(sem);
    unmap();
  } 

  return (sts == 0);

#endif
}


/* Inform Handler on local node that the list with supevise objects should be
   rebuilt and the scanning of supervise object restarted.

   Process must have done qcom_Init. */

pwr_tStatus
mh_UtilStartScanSup (
  qcom_sQid	source
)
{
  pwr_tStatus	sts;
  mh_sHead	head;
  qcom_sQid	target;

  head.xdr	= FALSE;
  head.ver	= mh_cVersion;
  sts = time_GetTime(&head.birthTime);
  head.qid	= source;
  co_GetOwnPlatform(&head.platform);
  head.source	= mh_eSource_Pcm;
  head.type	= mh_eMsg_StartScanSup;

  /* Send message to local handler */

  target.nid = 0;
  target.qix = mh_cProcHandler;

  sts = sendMessage(&target, &head);

  return sts;      
}

/* Inform Handler on local node that scanning of
   supervise object should be stopped.

   Process must have done qcom_Init. */

pwr_tStatus
mh_UtilStopScanSup (
  qcom_sQid	source
)
{
  pwr_tStatus	sts;
  mh_sHead	head;
  qcom_sQid	target;

  head.xdr = FALSE;
  head.ver = mh_cVersion;
  sts = time_GetTime(&head.birthTime);
  head.qid = source;
  co_GetOwnPlatform(&head.platform);
  head.source = mh_eSource_Pcm;
  head.type = mh_eMsg_StopScanSup;

  /* Send message to local handler */

  target.nid = 0;
  target.qix = mh_cProcHandler;

  sts = sendMessage(&target, &head);

  return sts;
}

/*  Wait for the Plc to start. */ 

pwr_tStatus
mh_UtilWaitForMh ()
{
  pwr_tStatus sts;

  sts = map();

  if (EVEN(sts))
    return sts;

#ifdef OS_ELN
  ker$wait_any(NULL, NULL, NULL, lMhUtilSyncArea);

#elif defined OS_VMS
  sys$waitfr(PWR_WAIT_MH);

#elif defined OS_LYNX
  sem_wait(sem);
  sem_post(sem);
  unmap();

#elif defined OS_LINUX
  posix_sem_wait(sem);
  posix_sem_post(sem);
  unmap();

#endif

  return 1;
}

/* Wake all waiting for the event monitor to start. */ 

pwr_tStatus
mh_UtilWake ()
{
  pwr_tStatus sts;
#ifdef OS_VMS
  pwr_tUInt32 State;
#endif

  sts = map();
  if (EVEN(sts))
    return (sts);

#if defined OS_ELN
  lInfo->isStarted = TRUE;
  ker$signal(NULL, lMhUtilSyncArea);

#elif defined OS_VMS
  sys$setef(PWR_WAIT_MH);
#elif defined OS_LYNX
  sem_post(sem);
  unmap();
#elif defined OS_LINUX
  posix_sem_post(sem);
  unmap();
#endif        

  return 1;
}
