/* plc_thread.c -- Run a PLC thread

   PROVIEW/R
   Copyright (C) 1999 by Mandator AB.

   Run a PLC thread.  */

#if defined(OS_ELN)
# include $vaxelnc
#else
# include <string.h>
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
  plc_sProcess *pp = tp->pp;

  /* Phase 1.  */
  
  tp->init(1, tp);

  que_Put(&sts, &tp->q_out, &tp->event, (void *)1);
  phase = (int)que_Get(&sts, &tp->q_in, NULL, NULL);
  pwr_Assert(phase == 2);

  /* Phase 2.  */

  tp->init(2, tp);

  que_Put(&sts, &tp->q_out, &tp->event, (void *)2);
  phase = (int)que_Get(&sts, &tp->q_in, NULL, NULL);
  pwr_Assert(phase == 3);

  /* Phase 3.  */

  thread_SetPrio(&tp->tid, tp->prio);

  rel_vec = ((tp->pp->PlcProcess->ChgCount - 1) % 2) + 1;
  sts = io_init(io_mProcess_Plc, tp->aref.Objid, &tp->plc_io_ctx, rel_vec, tp->f_scan_time);
  if (EVEN(sts)) {
    errh_Error("Failed to inititalize io, %m", sts);
    pp->IOHandler->IOReadWriteFlag = FALSE;
  }

  tp->init(0, tp);

  que_Put(&sts, &tp->q_out, &tp->event, (void *)3);

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

  thread_MutexUnlock(&pp->io_copy_mutex);

  /* Execute all the PLC code */
  tp->exec(0, tp);

  if (pp->IOHandler->IOReadWriteFlag) {
    sts = io_write(tp->plc_io_ctx);
    if (EVEN(sts)) {
      pp->IOHandler->IOReadWriteFlag = FALSE;
      errh_Error("IO write, %m", sts);
    }
  }

  time_Uptime(&sts, &tp->after_scan, NULL);
  if (sts == TIME__CLKCHANGE) {
    tp->after_scan = tp->before_scan;
  }
  clock_gettime(CLOCK_REALTIME, &tp->after_scan_abs);
  pp->Node->SystemTime = tp->after_scan_abs;
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
	if (delay_action == 2)
	  pp->IOHandler->IOReadWriteFlag = FALSE;
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

}


