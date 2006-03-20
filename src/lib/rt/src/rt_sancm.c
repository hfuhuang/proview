/* 
 * Proview   $Id: rt_sancm.c,v 1.3 2006-03-20 07:17:55 claes Exp $
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

/* rt_sancm.c -- Subscribed alarm (and block) notification, client monitor calls. */

#ifdef OS_ELN
# include stdlib
# include stdio
#else
# include <stdio.h>
# include <stdlib.h>
#endif

#include "pwr.h"
#include "co_cdh.h"
#include "rt_net.h"
#include "rt_pool.h"
#include "rt_hash.h"
#include "rt_gdb.h"
#include "rt_san.h"
#include "rt_san_msg.h"
#include "rt_sancm.h"
#include "rt_vol.h"
#include "rt_cvol.h"
#include "rt_errh.h"


void
sancm_Add (
  pwr_tStatus		*status,
  gdb_sNode		*np
)
{
  net_sSanAdd		*ap; 
  pool_sQlink		*ol;
  gdb_sObject		*op;
  int			count;
  int			i;
  qcom_sQid		tgt;

  if (status != NULL) *status = 1;
  
  gdb_AssumeLocked;

  if (np->sancAdd_lc == 0) return;
  count = MIN(np->sancAdd_lc, net_cSanMaxAdd);
  ap = malloc(sizeof(net_sSanAdd) + (count - 1) * sizeof(ap->sane[0]));
  if (ap == NULL) return;

  tgt.nid = np->nid;
  tgt.qix = net_cProcHandler;

  for (
    i = 0, ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancAdd_lh);
    ol != &np->sancAdd_lh;
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancAdd_lh)
  ) {
    op = pool_Qitem(ol, gdb_sObject, u.c.sanc_ll);

    ap->sane[i].oid = op->g.oid;
    ap->sane[i].sid = op->u.c.sanid;
    i++;
    pwr_Assert(op->u.c.flags.b.sancAdd);
    pool_Qremove(NULL, gdbroot->pool, &op->u.c.sanc_ll);
    op->u.c.flags.b.sancAdd = 0;
    pwr_Assert((op->u.c.flags.m & gdb_mCo_inSancList) == 0);
    pwr_Assert(np->sancAdd_lc != 0);
    np->sancAdd_lc--;
    pwr_Assert(!op->u.c.flags.b.sancAct);
    pool_QinsertPred(NULL, gdbroot->pool, &op->u.c.sanc_ll, &np->sancAct_lh);
    op->u.c.flags.b.sancAct = 1;
    np->sancAct_lc++;
    if (i >= count) {
      ap->count = count;
      gdb_Unlock;
	net_Put(NULL, &tgt, ap, net_eMsg_sanAdd, 0, pwr_Offset(ap, sane[i]));
      gdb_Lock;
      i = 0;
    }
  }

  pwr_Assert(np->sancAdd_lc == 0);

  /* Send remaining san entries.  */

  if (i > 0) {
    ap->count = i;
    gdb_Unlock;
      net_Put(NULL, &tgt, ap, net_eMsg_sanAdd, 0, pwr_Offset(ap, sane[i]));
    gdb_Lock;
  }

  free(ap);
}

void
sancm_FlushNode (
  pwr_tStatus		*status,
  gdb_sNode		*np
)
{
  pool_sQlink		*ol;
  gdb_sObject		*op;

  if (status != NULL) *status = 1;

  gdb_AssumeLocked;

  for (
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancAdd_lh);
    ol != &np->sancAdd_lh;
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancAdd_lh)
  ) {
    op = pool_Qitem(ol, gdb_sObject, u.c.sanc_ll);

    pwr_Assert(op->u.c.flags.b.sancAdd);
    pool_Qremove(NULL, gdbroot->pool, &op->u.c.sanc_ll);
    op->u.c.flags.b.sancAdd = 0;
    pwr_Assert((op->u.c.flags.m & gdb_mCo_inSancList) == 0);
    pwr_Assert(np->sancAdd_lc != 0);
    np->sancAdd_lc--;
  }
  pwr_Assert(np->sancAdd_lc == 0);

  for (
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancAct_lh);
    ol != &np->sancAct_lh;
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancAct_lh)
  ) {
    op = pool_Qitem(ol, gdb_sObject, u.c.sanc_ll);

    pwr_Assert(op->u.c.flags.b.sancAct);
    pool_Qremove(NULL, gdbroot->pool, &op->u.c.sanc_ll);
    op->u.c.flags.b.sancAct = 0;
    pwr_Assert((op->u.c.flags.m & gdb_mCo_inSancList) == 0);
    pwr_Assert(np->sancAct_lc != 0);
    np->sancAct_lc--;
  }
  pwr_Assert(np->sancAct_lc == 0);

  for (
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancRem_lh);
    ol != &np->sancRem_lh;
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancRem_lh)
  ) {
    op = pool_Qitem(ol, gdb_sObject, u.c.sanc_ll);

    pwr_Assert(op->u.c.flags.b.sancRem);
    pool_Qremove(NULL, gdbroot->pool, &op->u.c.sanc_ll);
    op->u.c.flags.b.sancRem = 0;
    pwr_Assert((op->u.c.flags.m & gdb_mCo_inSancList) == 0);
    pwr_Assert(np->sancRem_lc != 0);
    np->sancRem_lc--;
  }
  pwr_Assert(np->sancRem_lc == 0);

  return;
}

void
sancm_MoveExpired (
  pwr_tStatus		*status,
  gdb_sNode		*np
)
{
  pool_sQlink		*ol;
  gdb_sObject		*op;

  if (status != NULL) *status = 1;
  
  gdb_AssumeLocked;

  if (np->sancAct_lc == 0) return;

  for (
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancAct_lh);
    ol != &np->sancAct_lh;
  ) {
    op = pool_Qitem(ol, gdb_sObject, u.c.sanc_ll);
    ol = pool_Qsucc(NULL, gdbroot->pool, ol);

    pwr_Assert(op->u.c.flags.b.sancAct);

    if (op->u.c.sanexp) {
      pool_Qremove(NULL, gdbroot->pool, &op->u.c.sanc_ll);
      op->u.c.flags.b.sancAct = 0;
      pwr_Assert((op->u.c.flags.m & gdb_mCo_inSancList) == 0);
      pwr_Assert(np->sancAct_lc != 0);
      np->sancAct_lc--;

      pool_QinsertPred(NULL, gdbroot->pool, &op->u.c.sanc_ll, &np->sancRem_lh);
      op->u.c.flags.b.sancRem = 1;
      np->sancRem_lc++;
      op->u.c.sanexp = 0;
    } else if (!op->l.flags.b.isMountServer)
      op->u.c.sanexp = 1;
  }
}

void
sancm_Remove (
  pwr_tStatus		*status,
  gdb_sNode		*np
)
{
  net_sSanRemove	*rp; 
  pool_sQlink		*ol;
  gdb_sObject		*op;
  int			count;
  int			i;
  qcom_sQid		tgt;

  if (status != NULL) *status = 1;
  
  gdb_AssumeLocked;

  if (np->sancRem_lc == 0) return;

  count = MIN(np->sancRem_lc, net_cSanMaxRemove);
  rp = malloc(sizeof(net_sSanRemove) + (count - 1) * sizeof(rp->sid[0]));
  if (rp == NULL) return;

  tgt.nid = np->nid;
  tgt.qix = net_cProcHandler;

  for (
    i = 0, ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancRem_lh);
    ol != &np->sancRem_lh;
    ol = pool_Qsucc(NULL, gdbroot->pool, &np->sancRem_lh)
  ) {
    op = pool_Qitem(ol, gdb_sObject, u.c.sanc_ll);

    rp->sid[i] = op->u.c.sanid;
    i++;
    pwr_Assert(op->u.c.flags.b.sancRem);
    pool_Qremove(NULL, gdbroot->pool, &op->u.c.sanc_ll);
    op->u.c.flags.b.sancRem = 0;
    pwr_Assert((op->u.c.flags.m & gdb_mCo_inSancList) == 0);
    pwr_Assert(np->sancRem_lc != 0);
    np->sancRem_lc--;
#if 0
    cvolc_TrimObject(op);
#endif
    if (i >= count) {
      rp->count = count;
      gdb_Unlock;
	net_Put(NULL, &tgt, rp, net_eMsg_sanRemove, 0, pwr_Offset(rp, sid[i]));
      gdb_Lock;
      i = 0;
    }
  }

  pwr_Assert(np->sancRem_lc == 0);

  /* Send remaining san removes.  */

  if (i > 0) {
    rp->count = count;
    gdb_Unlock;
      net_Put(NULL, &tgt, rp, net_eMsg_sanRemove, 0, pwr_Offset(rp, sid[i]));
    gdb_Lock;
  }

  free(rp);
}

void
sancm_Update (
  qcom_sGet		*get
)
{
  gdb_sObject		*op;
  int			i;
  int			error;
  net_sSanUpdate	*up = get->data;
  
  gdb_AssumeUnlocked;

  gdb_ScopeLock {

    error = 0;
    if (0) errh_Info("sancm_Update, recieved %d", up->count);
    for (i = 0; i < up->count; i++) {
      op = hash_Search(NULL, gdbroot->oid_ht, &up->data[i].sane.oid);
      if (op == NULL || cdh_RefIdIsNotEqual(op->u.c.sanid, up->data[i].sane.sid)) {
        error++;
        continue;
      }

      up->data[i].sane.sid.rix = 0; /* Sid is ok!  */
      vol_UpdateAlarm(NULL, op, up->data[i].al);
    }
  
    /* Send remove for errors.  */
  
    if (error > 0) {
      net_sSanRemove	*rp;
      int		size = sizeof(net_sSanRemove) + (error - 1) * sizeof(rp->sid[0]);
      int		i;
      gdb_sNode		*np = hash_Search(NULL, gdbroot->nid_ht, &up->hdr.nid);
      qcom_sQid		tgt;
      
      tgt.nid = np->nid;
      tgt.qix = net_cProcHandler;
  
      rp = calloc(1, size);
      if (rp != NULL) {
	rp->count = error;
        for (i = 0; i < up->count; i++) {
	  if (up->data[i].sane.sid.rix == 0) continue;
	  rp->sid[--error] = up->data[i].sane.sid;
        }
        gdb_Unlock;
	errh_Info("sancm_Update, sent removed count %d", rp->count); 
  	net_Put(NULL, &tgt, rp, net_eMsg_sanRemove, 0, size);
        gdb_Lock;
        free(rp);
      }
    }  

  } gdb_ScopeUnlock;
  
}
