#ifndef rt_aproc_h
#define rt_aproc_h

/* rt_aproc.h -- <short description>

   PROVIEW/R
   Copyright (C) 1999 by Mandator AB.

   .  */

#if defined __cplusplus
extern "C" {
#endif

#include "pwr.h"
#include "rt_errh.h"

pwr_tStatus	aproc_RegisterObject(pwr_tOid);
pwr_tStatus	aproc_TimeStamp();

#if defined __cplusplus
}
#endif

#endif


