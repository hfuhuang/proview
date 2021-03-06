/* 
 * Proview   $Id: rt_qos.c,v 1.3 2005-09-01 14:57:57 claes Exp $
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

/* rt_qos.c -- Queue Communication
   Contains functions that are heavily os-dependant.  */

#if !defined(OS_LYNX)
# error "This file is valid only for OS_LYNX"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "pwr.h"
#include "pwr_class.h"
#include "co_errno.h"
#include "rt_qdb_msg.h"
#include "rt_qcom_msg.h"
#include "rt_hash_msg.h"
#include "co_time.h"
#include "rt_errh.h"
#include "rt_qdb.h"
#include "rt_pool.h"
#include "rt_hash.h"


pwr_tBoolean
qos_WaitQue (
  pwr_tStatus		*status,
  qdb_sQue		*qp,
  int			tmo
)
{
  pwr_tDeltaTime	dtime;
  struct timespec	dtime_ts;
  sigset_t		newset;
  sigset_t		oldset;
  siginfo_t		info;
  int			ok;
  pwr_tBoolean		signal = FALSE;
  pwr_dStatus		(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  if (tmo == qcom_cTmoNone)
    return FALSE;

  qp->lock.waiting = TRUE;

  sigemptyset(&newset);
  sigaddset(&newset, qdb_cSigMsg);
  sigprocmask(SIG_BLOCK, &newset, &oldset);

  qp->lock.pid = BUILDPID(getpid(), pthread_self());

  qdb_Unlock;

    if (tmo != qcom_cTmoEternal) {
      time_MsToD(&dtime, tmo);
      dtime_ts.tv_sec = dtime.tv_sec;
      dtime_ts.tv_nsec = dtime.tv_nsec;
      ok = sigtimedwait(&newset, &info, &dtime_ts);  
    } else {
      ok = sigwaitinfo(&newset, &info);
    }
    
    if (ok == -1 && errno != EAGAIN) {
      errh_Error("waitQue (%d) %s", errno, strerror(errno));
    }

  qdb_Lock;

  if (qp->lock.waiting) {
    *sts = QCOM__TMO;
    qp->lock.waiting = FALSE;
  } else {
    signal = TRUE;
  }

  sigprocmask(SIG_SETMASK, &oldset, NULL);
  return signal;
}

pwr_tStatus
qos_SignalQue (
  pwr_tStatus	*status,
  qdb_sQue	*qp
)
{
  union sigval	value;
  int		ok;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  if (qp->lock.waiting) {
    value.sival_int = BUILDPID(getpid(), pthread_self());
    qp->lock.waiting = FALSE;

    ok = sigqueue(qp->lock.pid, qdb_cSigMsg, value);

    if (ok == -1) {
      *sts = errno_Status(errno);
    }
  }

  return TRUE;
}

qdb_sQlock *
qos_CreateQlock (
  pwr_tStatus	*sts,
  qdb_sQue	*qp
)
{

  qdb_AssumeLocked;

  return &qp->lock;
}

void
qos_DeleteQlock (
  pwr_tStatus	*sts,
  qdb_sQue	*qp
)
{

  qdb_AssumeLocked;

}
