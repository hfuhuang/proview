/* 
 * Proview   $Id: rt_plc_thread.c,v 1.12 2007-01-04 07:52:30 claes Exp $
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

/* plc_thread.c -- Run a PLC thread
   Run a PLC thread.  */

#if defined(OS_ELN)
# include $vaxelnc
#else
# include <string.h>
#endif

#if defined(OS_LINUX)
# include <pwd.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_time.h"
#include "co_time_msg.h"
#include "rt_errh.h"
#include "rt_gdh.h"
#include "rt_plc_msg.h"
#include "rt_plc_rt.h"
#include "rt_plc.h"
#include "rt_thread.h"
#include "rt_thread_msg.h"
#include "rt_que.h"
#include "rt_io_base.h"
#include "rt_csup.h"
#if 0
#include "rt_io_supervise.h"
#endif
#include "rt_plc_loop.h"
#include "rt_c_plcthread.h"
#include "rt_c_node.h"

#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))
#define MIN_SCANTIME 1e-9

/* When you use pthread_cond_timedwait is the shortest timeout always > 1 CLK_TCK
 * So if you want to run a PLC-program with the same frequency as the scheduler
 * you need to use a rt-timer. In this version there is no slip detection if
 * you use a rt-timer. You need to set USE_RT_TIMER to 1 in rt_plc_thread.c
 * and rt_plc_process.c
 */
#define USE_RT_TIMER 0


static void scan (plc_sThread*);



void
plc_thread (
  plc_sThread *tp
)
{
  pwr_tStatus sts;
  int rel_vec;
  int phase;
  uid_t ruid;
  plc_sProcess *pp = tp->pp;

  /* Phase 1.  */
  
  tp->init(1, tp);

  sts = thread_SetPrio(&tp->tid, tp->prio);
  if (EVEN(sts)) {
    errh_Error("Failed to set priority, plc thread %d ms, prio %d",
    			(int) (tp->PlcThread->ScanTime * 1000), tp->prio);
  }
  else {
    errh_Info("Priority set, plc thread %d ms, prio %d",
    			(int) (tp->PlcThread->ScanTime * 1000), tp->prio);
  }
  
  que_Put(&sts, &tp->q_out, &tp->event, (void *)1);
  phase = (int)que_Get(&sts, &tp->q_in, NULL, NULL);
  pwr_Assert(phase == 2);

  /* Once thread's has set it's priority don't run as root */

#if defined(OS_LINUX)
  struct passwd *pwd;
  
  ruid = getuid();
  
  if (ruid == 0) {
    pwd = getpwnam("pwrp");
    if (pwd != NULL) {
      setreuid(pwd->pw_uid, pwd->pw_uid);
    }
  }
  else 
    setreuid(ruid, ruid);
#endif

  /* Phase 2.  */

  tp->init(2, tp);

  que_Put(&sts, &tp->q_out, &tp->event, (void *)2);
  phase = (int)que_Get(&sts, &tp->q_in, NULL, NULL);
  pwr_Assert(phase == 3);

  /* Phase 3.  */

  rel_vec = ((tp->pp->PlcProcess->ChgCount - 1) % 2) + 1;
  sts = io_init(tp->PlcThread->IoProcess, tp->aref.Objid, &tp->plc_io_ctx, rel_vec, tp->f_scan_time);
  if (EVEN(sts)) {
    pp->IOHandler->IOReadWriteFlag = FALSE;
    errh_Error("Failed to inititalize io, %m", sts);
    errh_SetStatus( PLC__ERRINITIO);
  }

  tp->init(0, tp);
  
  if (tp->pp->IOHandler->IOReadWriteFlag) {
    sts = io_read(tp->plc_io_ctx);
    if (EVEN(sts)) {
      tp->pp->IOHandler->IOReadWriteFlag = FALSE;
      errh_Error("IO read, %m", sts);
      errh_SetStatus( PLC__IOREAD);
    }
  }

  thread_MutexLock(&tp->pp->io_copy_mutex);

  memcpy(tp->copy.ai_a.p, tp->pp->base.ai_a.p, tp->copy.ai_a.size);
  memcpy(tp->copy.ao_a.p, tp->pp->base.ao_a.p, tp->copy.ao_a.size);
  memcpy(tp->copy.av_a.p, tp->pp->base.av_a.p, tp->copy.av_a.size);
  memcpy(tp->copy.ca_a.p, tp->pp->base.ca_a.p, tp->copy.ca_a.size);
  memcpy(tp->copy.co_a.p, tp->pp->base.co_a.p, tp->copy.co_a.size);
  memcpy(tp->copy.di_a.p, tp->pp->base.di_a.p, tp->copy.di_a.size);
  memcpy(tp->copy.do_a.p, tp->pp->base.do_a.p, tp->copy.do_a.size);
  memcpy(tp->copy.dv_a.p, tp->pp->base.dv_a.p, tp->copy.dv_a.size);
  memcpy(tp->copy.ii_a.p, tp->pp->base.ii_a.p, tp->copy.ii_a.size);
  memcpy(tp->copy.io_a.p, tp->pp->base.io_a.p, tp->copy.io_a.size);
  memcpy(tp->copy.iv_a.p, tp->pp->base.iv_a.p, tp->copy.iv_a.size);

  thread_MutexUnlock(&tp->pp->io_copy_mutex);
  
  que_Put(&sts, &tp->q_out, &tp->event, (void *)3);
  phase = (int)que_Get(&sts, &tp->q_in, NULL, NULL);
  pwr_Assert(phase == 4);
  
  /* Phase 4.  */

  que_Put(&sts, &tp->q_out, &tp->event, (void *)4);

  pwrb_PlcThread_Zero(tp);

  time_Uptime(&sts, &tp->sync_time, NULL);

  tp->ActualScanTime = tp->f_scan_time;

  while (!tp->exit)
    scan(tp);

#if 0 /*defined(OS_ELN)*/
  if (wfp) {
    /* We have exited the PLC loop. Clean up watchdog object */
    wfp->delete(&sts, wp);
    if (EVEN(sts))
      errh_Error("Cleaning up watchdog, %m", sts);
  }
#endif

  /* Tell main we are done.  */
  que_Put(&sts, &tp->q_out, &tp->event, (void *)5);
}

static void
scan (
  plc_sThread	*tp
)
{
  pwr_tStatus	sts;
  plc_sProcess	*pp = tp->pp;
  int		delay_action = 0;

  time_Uptime(&sts, &tp->before_scan, NULL);  
  clock_gettime(CLOCK_REALTIME, &tp->before_scan_abs);
  pp->Node->SystemTime = tp->before_scan_abs;

  if (tp->loops > 0) {
    if (sts == TIME__CLKCHANGE) {
      time_Dadd(&tp->before_scan, &tp->one_before_scan, &tp->scan_time);
    }
    time_Dsub(&tp->delta_scan, &tp->before_scan, &tp->one_before_scan);
    time_DToFloat(&tp->ActualScanTime, &tp->delta_scan);
    if (tp->ActualScanTime < MIN_SCANTIME)
      tp->ActualScanTime = MIN_SCANTIME;
  }

  if (pp->IOHandler->IOReadWriteFlag) {
    sts = io_read(tp->plc_io_ctx);
    if (EVEN(sts)) {
      pp->IOHandler->IOReadWriteFlag = FALSE;
      errh_Error("IO read, %m", sts);
      errh_SetStatus( PLC__IOREAD);
    }
  }

  thread_MutexLock(&pp->io_copy_mutex);

  memcpy(tp->copy.ai_a.p, pp->base.ai_a.p, tp->copy.ai_a.size);
  memcpy(tp->copy.ao_a.p, pp->base.ao_a.p, tp->copy.ao_a.size);
  memcpy(tp->copy.av_a.p, pp->base.av_a.p, tp->copy.av_a.size);
  memcpy(tp->copy.ca_a.p, pp->base.ca_a.p, tp->copy.ca_a.size);
  memcpy(tp->copy.co_a.p, pp->base.co_a.p, tp->copy.co_a.size);
  memcpy(tp->copy.di_a.p, pp->base.di_a.p, tp->copy.di_a.size);
  memcpy(tp->copy.do_a.p, pp->base.do_a.p, tp->copy.do_a.size);
  memcpy(tp->copy.dv_a.p, pp->base.dv_a.p, tp->copy.dv_a.size);
  memcpy(tp->copy.ii_a.p, pp->base.ii_a.p, tp->copy.ii_a.size);
  memcpy(tp->copy.io_a.p, pp->base.io_a.p, tp->copy.io_a.size);
  memcpy(tp->copy.iv_a.p, pp->base.iv_a.p, tp->copy.iv_a.size);

  thread_MutexUnlock(&pp->io_copy_mutex);

  /* Execute all the PLC code */
  tp->exec(0, tp);

  if (pp->IOHandler->IOReadWriteFlag) {
    sts = io_write(tp->plc_io_ctx);
    if (EVEN(sts)) {
      pp->IOHandler->IOReadWriteFlag = FALSE;
      errh_Error("IO write, %m", sts);
      errh_SetStatus( PLC__IOWRITE);
    }
  }

  if ( tp->first_scan)
    tp->first_scan = 0;

  time_Uptime(&sts, &tp->after_scan, NULL);
  if (sts == TIME__CLKCHANGE) {
    tp->after_scan = tp->before_scan;
  }
  clock_gettime(CLOCK_REALTIME, &tp->after_scan_abs);
  if (tp->log)
    pwrb_PlcThread_Exec(tp);

  do {
    pwr_tDeltaTime delta;

    plc_timerhandler(tp); 
    time_Dadd(NULL, &tp->sync_time, &tp->scan_time);
    time_Dsub(&delta, &tp->sync_time, &tp->after_scan);
    if (time_Dcomp(&delta, NULL) > 0) {
      pwr_tStatus sts;
      int phase;

      if (tp->csup_lh != NULL) {
	pwr_tTime now;
	clock_gettime(CLOCK_REALTIME, &now);
	delay_action = csup_Exec(&sts, tp->csup_lh, &tp->sync_time, &tp->after_scan, &now);
	if (delay_action == 2) {
	  pp->IOHandler->IOReadWriteFlag = FALSE;
	  errh_SetStatus( PLC__IOSTALLED);
	}
      }

#if defined OS_LYNX && USE_RT_TIMER
      sem_wait(&tp->ScanSem);
      phase = 0;
#else
      phase = (int)que_Get(&sts, &tp->q_in, &delta, NULL);
#endif
      if (phase > 0) {
	tp->exit = TRUE;
      }
      break;

    } else
      tp->sliped++;

  } while (!tp->exit);

  tp->one_before_scan = tp->before_scan;
  tp->one_before_scan_abs = tp->before_scan_abs;

  if (++tp->loops == 2)
    tp->log = TRUE;

  if ( tp->loops % max( 1, (int)(1.0 / tp->PlcThread->ScanTime)) == 0)
    pwrs_Node_SupEmon();
}


