/* rt_proc.c -- <short description>

   PROVIEW/R
   Copyright (C) 1999 by Mandator AB.

   .  */

#if !defined(OS_ELN)
# error "This file is valid only for OS_ELN"
#endif

#include $vaxelnc
#include $kernelmsg
#include stdio
#include stdlib
#include string
#include descrip

#include "pwr.h"
#include "co_errno.h"
#include "rt_errh.h"
#include "rt_proc.h"
#include "rt_proc_msg.h"


pwr_tStatus
proc_Load (
  proc_sProcess *p
)
{
  pwr_tStatus sts = PROC__SUCCESS;
  VARYING_STRING(255)	file;
  VARYING_STRING(41)	name;

  if (!p->flags.b.load)
    return sts;

  CSTRING_TO_VARYING(p->file, file);
  CSTRING_TO_VARYING(p->name, name);

  eln$load_program(&file, &name, p->flags.b.k_mode,
    p->flags.b.debug, FALSE, p->k_size, p->u_size,
    10, p->p_prio, p->t_prio, &sts);

  return sts;
}

pwr_tStatus
proc_Start (
  proc_sProcess *p
)
{
  pwr_tStatus sts = PROC__SUCCESS;
  PORT jobport;
  $DESCRIPTOR(name, "");
  $DESCRIPTOR(null, "");
  $DESCRIPTOR(arg, "");

  if (p->flags.b.load) {
    name.dsc$a_pointer = p->name;
    name.dsc$w_length = strlen(p->name);
  } else {
    name.dsc$a_pointer = p->file;
    name.dsc$w_length = strlen(p->file);
  }
  if (p->arg != NULL) {
    arg.dsc$a_pointer = p->arg;
    arg.dsc$w_length = strlen(p->arg);
  }

  ker$create_job(&sts, &jobport, &name, NULL, &null, &null, &null, &arg);

  return sts;
}

pwr_tStatus
proc_SetPriority (
  int prio
)
{
  pwr_tStatus sts = PROC__SUCCESS;

  ker$set_job_priority(&sts, prio);

  return sts;
}

pwr_tStatus
proc_UnloadProgram (
  proc_sProcess *p
)
{
  pwr_tStatus sts = PROC__SUCCESS;
  VARYING_STRING(41) name;

  CSTRING_TO_VARYING(p->name, name);

  eln$unload_program(&name, &sts);

  return sts;
}
