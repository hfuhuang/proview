/* rt_cmvolsm.c -- Cached volumes

   PROVIEW/R
   Copyright (C) 1996-2002 by Cell Network AB.

   This module contains the meta cache handling routines for the server monitor.  */


#ifdef	OS_ELN
# include $vaxelnc
#else
# include <stdio.h>
# include <string.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "co_time.h"
#include "rt_qcom.h"
#include "rt_gdh_msg.h"
#include "rt_pool.h"
#include "rt_vol.h"
#include "rt_net.h"
#include "rt_cmvolsm.h"




/** Handle GetCclass message */
void
cmvolsm_GetCclass (
  qcom_sGet		*get
  )
{
  pwr_tStatus		sts;
  pwr_tStatus		lsts;
  int			size;
  net_sGetCclassR	*rmp = NULL;
  net_sGetCclass	*mp = get->data;
  qcom_sPut		put;
  gdb_sAttribute        *ap;
  net_sCattribute       *cap;
  gdb_sClass            *cp;
  gdb_sObject           *cop;
  int                   i;  
  pwr_tBoolean          equal = FALSE;
  const int		maxacnt = ((net_cSizeLarge - sizeof(*rmp)) / sizeof(rmp->attr[0])) + 1;  
  int			maxaidx;
  int			acnt;

  gdb_AssumeUnlocked;

  gdb_ScopeLock {
    
    cp = hash_Search(&sts, gdbroot->cid_ht, &mp->cid);
    if (cp == NULL) 
      break;
    
    cop = pool_Address(NULL, gdbroot->pool, cp->cor);
    if (time_Acomp(&mp->time, &cop->u.n.time) == 0) {
      equal = TRUE;
      size = sizeof(*rmp);
    } else {
      acnt = MIN(maxacnt, cp->acount - mp->aidx);
      maxaidx = acnt + mp->aidx;
      size = sizeof(*rmp) + MAX(0, (acnt - 1)) * sizeof(rmp->attr[0]);
    }
  

    rmp = net_Alloc(&sts, &put, size, net_eMsg_getCclassR);
    if (rmp == NULL) {
      errh_Error("cmvolsm_GetCclass. net_Alloc, size %d, cid %d, error %m", size, mp->cid, sts);
      break;
    }
    
    rmp->ver = net_cVersion;
    rmp->sts = 1;
    rmp->equal = equal;      


    if (equal) {
      rmp->acntmsg = 0;
      rmp->cclass.size = 0;
      rmp->cclass.acount = 0;

      break;
    }    

    rmp->cclass.cid = mp->cid;
    rmp->cclass.time = cop->u.n.time;
    rmp->cclass.size = cp->size;
    rmp->cclass.acount = cp->acount;

    for (i = mp->aidx, ap = &cp->attr[mp->aidx], cap = rmp->attr; 
         i < maxaidx; 
         i++, ap++, cap++
    ) {
      cap->aix = ap->aix;
      cap->flags = ap->flags;
      cap->type = ap->type;
      cap->offs = ap->offs;
      cap->size = ap->size;
      cap->elem = ap->elem;
      cap->moffset = ap->moffset;
    }

    rmp->acntmsg = acnt;
    rmp->saidx = mp->aidx;
    rmp->naidx = (i == cp->acount) ? ULONG_MAX : i;

  } gdb_ScopeUnlock;


  if (EVEN(sts)) {    
    rmp = net_Alloc(&lsts, &put, sizeof(*rmp), net_eMsg_getCclassR);
    if (EVEN(lsts)) {
      errh_Error("cmvolsm_GetCclass. net_Alloc, size %d, cid %d, error %m", sizeof(*rmp), mp->cid, lsts);
      return;
    }
    rmp->ver = net_cVersion;
  }  


  if (EVEN(sts))
    rmp->sts = sts;
  else 
    rmp->sts = 1;

  net_Reply(&lsts, get, &put);  
}



/** Handle GetGclass message */
void
cmvolsm_GetGclass (
  qcom_sGet		*get
  )
{
  pwr_tStatus		sts;
  pwr_tStatus		lsts;
  int			size;
  net_sGetGclassR	*rmp = NULL;
  net_sGetGclass	*mp = get->data;
  qcom_sPut		put;
  pwr_tVolumeId		cvid;
  gdb_sVolume		*vp;
  pwr_sClassDef		*cbp;
  pwr_sObjBodyDef	*bbp;
  pwr_sParam		*abp;
  gdb_sAttribute        *ap;
  net_sGattribute       *gap;
  gdb_sClass            *cp;
  gdb_sObject           *cop;
  gdb_sObject           *bop;
  gdb_sObject           *aop;
  int                   i;  
  const int		maxacnt = ((net_cSizeLarge - sizeof(*rmp)) / sizeof(rmp->attr[0])) + 1;  
  int			maxaidx;
  int			acnt;
  


  gdb_AssumeUnlocked;

  gdb_ScopeLock {


    cp = hash_Search(&sts, gdbroot->cid_ht, &mp->cid);
    if (cp == NULL) 
      break;

    cvid = mp->cid >> 16;
    vp = hash_Search(&sts, gdbroot->vid_ht, &cvid);
    if (vp == NULL)
      break;
    
    cop = pool_Address(NULL, gdbroot->pool, cp->cor);
    acnt = MIN(maxacnt, cp->acount - mp->aidx);
    maxaidx = acnt + mp->aidx;
    size = sizeof(*rmp) + MAX(0, (acnt - 1)) * sizeof(rmp->attr[0]);
  

    rmp = net_Alloc(&sts, &put, size, net_eMsg_getGclassR);
    if (rmp == NULL) {
      errh_Error("cvolsm_GetGclass. net_Alloc, size %d, cid %d, error %m", size, mp->cid, sts);
      break;
    }
    rmp->ver = net_cVersion;
    
    strcpy(rmp->vname, vp->g.name.orig);
    rmp->gclass.time = cop->u.n.time;
    rmp->gclass.co = cop->g;
    rmp->gclass.dbsFlags = cop->u.n.lflags.m;

    if (mp->aidx == 0) {
      cbp = pool_Address(&sts, gdbroot->rtdb, cp->cbr);
      if (cbp == NULL)
        break;
      rmp->gclass.cb = *cbp;

      bop = pool_Address(&sts, gdbroot->pool, cp->bor);
      if (bop == NULL)
        break;
      rmp->gclass.bo = bop->g;
      
      bbp = pool_Address(&sts, gdbroot->rtdb, cp->bbr);
      if (bbp == NULL)
        break;
      rmp->gclass.bb = *bbp;
    }    

    rmp->gclass.size = cp->size;
    rmp->gclass.acount = cp->acount;
    
    for (i = mp->aidx, ap = &cp->attr[mp->aidx], gap = rmp->attr; 
         i < maxaidx; 
         i++, ap++, gap++
    ) {

      aop = pool_Address(&sts, gdbroot->pool, ap->aor);
      if (aop == NULL)
        break;
      gap->ao = aop->g;

      abp = pool_Address(&sts, gdbroot->rtdb, ap->abr);
      if (aop == NULL)
        break;
      gap->ab = *abp;
    }

    rmp->acntmsg = acnt;
    rmp->saidx = mp->aidx;
    rmp->naidx = (i == cp->acount) ? ULONG_MAX : i;

  } gdb_ScopeUnlock;


  if (EVEN(sts)) {
    if (rmp != NULL)
      net_Free(NULL, rmp);
    
    rmp = net_Alloc(&lsts, &put, sizeof(*rmp), net_eMsg_getGclassR);
    if (EVEN(lsts)) {
      errh_Error("cmvolsm_GetGclass. net_Alloc, size %d, cid %d, error %m", sizeof(*rmp), mp->cid, lsts);
      return;
    }
    rmp->ver = net_cVersion;
  }
  

  if (EVEN(sts))
    rmp->sts = sts;
  else 
    rmp->sts = 1;

  net_Reply(&lsts, get, &put);  
}

