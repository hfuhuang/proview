/* 
 * Proview   $Id: rt_io_comm.c,v 1.6 2008-09-30 14:05:14 claes Exp $
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

/* rt_io_comm.c -- io handler */

#if defined(OS_ELN)
# include $vaxelnc
#else
# include <stdio.h>
# include <stddef.h>
# include <stdlib.h>
# include <string.h>
# include <math.h>
#endif

#include "pwr.h"
#include "co_time.h"
#include "rt_gdh.h"
#include "pwr_baseclasses.h"
#include "rt_io_base.h"
#include "rt_plc_utl.h"
#include "rt_errh.h"
#include "rt_csup.h"
#include "rt_qcom.h"
#include "rt_aproc.h"
#include "rt_qcom_msg.h"
#include "rt_ini_event.h"
#include "rt_pwr_msg.h"

#if defined(OS_ELN)
#include "rt_io_dioc.h"
#endif


#if defined(OS_ELN)
 extern EVENT    io_comm_terminate;
 EVENT           io_comm_terminate;
#endif

static pwr_sClass_IOHandler* init(qcom_sQid*, lst_sEntry**);

static void usage()
{
  printf( "\
rt_io_comm     I/O Communication program\n\
\n\
     -m        Print loaded I/O methods\n\
\n");
}

int main (int argc, char **argv)
{
  pwr_tStatus sts = 1;
  io_tCtx io_ctx;
  io_tCtx io_ctx_swap;
  pwr_sClass_IOHandler *ihp;
  int swap_io;
  int close_io;
  int init_io;
  qcom_sQid qid = qcom_cNQid;
  int tmo;
  char mp[2000];
  qcom_sGet get;
  pwr_tTime now;
  pwr_tTime next;
  pwr_tTime after;
  pwr_tDeltaTime cycle;
  lst_sEntry *csup_lh;
  int delay_action = 0;

  if ( argc > 1) {
    if ( strcmp( argv[1], "-m") == 0) {
      io_methods_print();
      exit(0);
    }
    if ( strcmp( argv[1], "-h") == 0) {
      usage();
      exit(0);
    }
  }

  ihp = init(&qid, &csup_lh);

  plc_UtlWaitForPlc();

  /* Prepare the swap context */
  sts = io_init_swap(io_mProcess_IoComm, pwr_cNObjid, &io_ctx_swap, 1, ihp->CycleTimeBus);

  for (close_io = swap_io = 0, init_io = 1;;) {

    if (init_io) {
      double f;
      
      sts = io_init(io_mProcess_IoComm, pwr_cNObjid, &io_ctx, 1, ihp->CycleTimeBus);
      if ( ODD(sts)) 
	errh_SetStatus( PWR__SRUN);
#if defined(OS_ELN)
      ker$clear_event( &sts, io_comm_terminate);
      io_dioc_init();
      io_dioc_start();
#endif
      init_io = 0;
      tmo = ihp->CycleTimeBus * 1000.;
      f = floor(ihp->CycleTimeBus);
      cycle.tv_sec = f;
      cycle.tv_nsec = (ihp->CycleTimeBus - f) * 1.0e9;
      cycle.tv_nsec++;
      time_GetTimeMonotonic(&next);
      time_Aadd(NULL, &next, &cycle);
    }

    get.maxSize = sizeof(mp);
    get.data = mp;
    qcom_Get(&sts,&qid, &get, tmo);
    if (sts == QCOM__TMO || sts == QCOM__QEMPTY) {
    
      sts = io_read(io_ctx);
      sts = io_write(io_ctx);

      if (swap_io) 
      {
        sts = io_swap(io_ctx_swap);
      }
      io_ScanSupLst( io_ctx->SupCtx);

      time_GetTime(&now);
      time_GetTimeMonotonic(&after);
      next = after;
      time_Aadd(NULL, &next, &cycle);
      delay_action = csup_Exec(&sts, csup_lh, (pwr_tDeltaTime *) &next,
                            (pwr_tDeltaTime *) &after, &now);
      if (delay_action == 2)
	ihp->IOReadWriteFlag = FALSE;

      aproc_TimeStamp();
    } else {
      ini_mEvent  new_event;
      qcom_sEvent *ep = (qcom_sEvent*) get.data;

      new_event.m  = ep->mask;
      if (new_event.b.oldPlcStop && !swap_io) {
        swap_io = 1;
	close_io = 1;
	errh_SetStatus(PWR__SRVRESTART);
      } else if (new_event.b.swapDone && swap_io) {
        swap_io = 0;
	init_io = 1;
      } else if (new_event.b.terminate) {
        exit(0);
      }
      if (close_io) {    
	io_close(io_ctx);
#if defined(OS_ELN)
	ker$signal( &sts, io_comm_terminate);
#endif
	close_io = 0;
      }
    }
  }
}

static pwr_sClass_IOHandler *
init (qcom_sQid *qid, lst_sEntry **csup_lh)
{
  pwr_tStatus sts = 1;
  pwr_sClass_IOHandler *ihp;
  qcom_sQattr qAttr;
  qcom_sQid qini;
  pwr_tObjid oid;

  errh_Init("pwr_io", errh_eAnix_io);

#if defined(OS_ELN)
  /* Event to kill dioc */
  ker$create_event(&sts, &io_comm_terminate, EVENT$CLEARED);
#endif

  if (!qcom_Init(&sts, 0, "pwr_io")) {
    errh_Fatal("qcom_Init, %m", sts);
    exit(sts);
  } 

  qAttr.type = qcom_eQtype_private;
  qAttr.quota = 100;
  if (!qcom_CreateQ(&sts, qid, &qAttr, "events")) {
    errh_Fatal("qcom_CreateQ, %m", sts);
    exit(sts);
  } 

  qini = qcom_cQini;
  if (!qcom_Bind(&sts, qid, &qini)) {
    errh_Fatal("qcom_Bind(Qini), %m", sts);
    exit(-1);
  }

  sts = gdh_Init("pwr_io");
  if (EVEN(sts)) {
    errh_Fatal("rt_io_comm aborted\n%m", sts);
    exit(sts);
  }

  sts = io_get_iohandler_object(&ihp, &oid);
  if (EVEN(sts)) {
    errh_Fatal("rt_io_comm aborted, no IoHandler object found\n%m", sts);
    exit(sts);
  }

  aproc_RegisterObject( oid);

  *csup_lh = csup_Init(&sts, oid, ihp->CycleTimeBus);

  return ihp;
}









