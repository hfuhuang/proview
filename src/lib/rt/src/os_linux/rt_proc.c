/* 
 * Proview   $Id: rt_proc.c,v 1.3 2005-09-01 14:57:57 claes Exp $
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

#if !defined(OS_LYNX) && !defined(OS_LINUX)
# error "This file is valid only for OS_LYNX and OS_LINUX"
#endif

#include <string.h>
#include <sched.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include "pwr.h"
#include "co_cdh.h"
#include "co_errno.h"
#include "co_strtoargv.h"
#include "rt_proc.h"
#include "rt_proc_msg.h"
#include "rt_errh.h"


pwr_tStatus
proc_Load (
  proc_sProcess *p
)
{
  pwr_tStatus sts = PROC__SUCCESS;

  return sts;
}

pwr_tStatus
proc_Start (
  proc_sProcess *p
)
{
  pwr_tStatus sts = PROC__SUCCESS;
  char **argv;

  p->pid = fork();
  if (p->pid) {
    if (p->pid == -1) {
      errh_Error("Could not start %s, %m\nfile: %s", p->name, errno_GetStatus(), p->file);	
    } else {
      errh_Info("Started %s, prio: %d, pid: %d\nfile: %s", p->name, p->p_prio, (int)p->pid, p->file);	
    }
  } else {
    sts = proc_SetPriority(p->p_prio);
    if (EVEN(sts)) errh_Warning("%s: error setprio, %m\nfile: %s", p->name, sts, p->file);
    argv = co_StrToArgv(p->file, p->arg);
    execvp(p->file, argv);
    errh_Error("%s: error execvp, %m\nfile: %s", p->name, errno_GetStatus(), p->file);
    exit(EXIT_FAILURE);
  }

  return sts;
}

pwr_tStatus
proc_SetPriority (
  int prio
)
{
//  struct sched_param param;
//  int rc;
  int  pid;
  char set[100];
  pwr_tStatus sts = PROC__SUCCESS;

  pid = getpid();
//  rc = sched_getparam((pid_t)0, &param);
//  if (rc != 0)
//    return errno_GetStatus();
//  param.sched_priority = prio;
//  rc = sched_setscheduler((pid_t)0, SCHED_RR, &param);
//  if (rc != 0)
//    return errno_GetStatus();

  sprintf(set, "rt_prio -rp %d %d", prio, pid);
//  system(set);
  return sts;  
}

pwr_tStatus
proc_UnloadProgram (
  proc_sProcess *p
)
{
  pwr_tStatus sts = PROC__SUCCESS;

  return sts;
}
