/* 
 * Proview   $Id: rt_cvol.c,v 1.3 2005-09-01 14:57:55 claes Exp $
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

/* rt_cvol.c -- Cached volumes
   This module contains the cache handling routines.  */

#ifndef OS_ELN
# include <stdio.h>
# include <string.h>
#endif

#if defined(OS_ELN)
# include $vaxelnc
#elif defined(OS_VMS)
# include <descrip.h>
# include <lib$routines.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "rt_gdh_msg.h"
#include "rt_pool.h"
#include "rt_vol.h"
#include "rt_net.h"
#include "rt_cvol.h"
#include "rt_cvolcm.h"
#include "rt_errh.h"



/* Allocate space for an object header, fill it with data,
   link it into the 'cacheNew' touch list, and fill
   in all the other local linkage.

   If the object already is in the cache, then this operation
   is a noop.  */

gdb_sObject *
cvol_AllocObject (
  pwr_tStatus		*sts,
  gdb_sNode		*np,
  gdb_sVolume		*vp
)
{
  gdb_sObject		*op;
  pool_sQlink		*ol;

  gdb_AssumeLocked;

  do {
    int trimcount = 0;

    ol = pool_Qpred(sts, gdbroot->pool, &gdbroot->db->cacheFree.lh);
    if (ol == NULL || ol == &gdbroot->db->cacheFree.lh) {
      while (1) {
	pwr_Assert(trimcount < 2);
	trimcount++;      
	if (gdbroot->db->log.b.cacheTrim) {
	  printf("cacheFree is exhausted: %d < %d < %d, start trim...\n",
	    gdbroot->db->cacheFree.lc_min, gdbroot->db->cacheFree.lc,
	    gdbroot->db->cacheFree.lc_max);
	}
	cvolcm_TrimOld();
	if (gdbroot->db->log.b.cacheTrim) {
	  printf("cacheFree trimmed %d < %d < %d\n",
	    gdbroot->db->cacheFree.lc_min, gdbroot->db->cacheFree.lc,
	    gdbroot->db->cacheFree.lc_max);
	}
	if (gdbroot->db->cacheFree.lc > 0) break;
	cvol_QforcedTrim(&np->cacheNode);
	cvolcm_TrimOld();
	if (gdbroot->db->cacheFree.lc > 0) break;
	cvol_QforcedTrim(&vp->u.c.cacheVol);
	cvolcm_TrimOld();
	if (gdbroot->db->cacheFree.lc > 0) break;
        cvol_Qtrim(&gdbroot->db->cacheNew);
	break;
      }
    }
  } while (ol == NULL || ol == &gdbroot->db->cacheFree.lh);

  op = pool_Qitem(ol, gdb_sObject, u.c.cache_ll);
  pool_Qremove(NULL, gdbroot->pool, ol);
  gdbroot->db->cacheFree.lc--;
  memset(op, 0, sizeof(*op));

  pwr_Return(op, sts, GDH__SUCCESS);
}

/* .  */

void
cvol_FlushObject (
  gdb_sObject		*op
)
{
  gdb_sVolume		*vp = NULL;

  gdb_AssumeLocked;

  /* Make sure it is linked.  */

  pwr_Assert(op->u.c.flags.m & gdb_mCo_inTouchList);
  pwr_Assert(pool_QisLinked(NULL, gdbroot->pool, &op->u.c.cache_ll));

  cvol_Qremove(op);
  cvol_QmoveSucc(op, NULL, &gdbroot->db->cacheFree);

  vp = pool_Address(NULL, gdbroot->pool, op->l.vr);
  cvol_UnlinkObject(NULL, vp, op, vol_mLink_cacheFlush);
  vol_UnlinkObject(NULL, vp, op, vol_mLink_cacheFlush);

}

/* .  */

void
cvol_FreeObject (
  pwr_tStatus		*sts,
  gdb_sVolume		*vp,
  gdb_sObject		*op,
  pwr_tBitMask		link
)
{
  gdb_sObject		*pop = NULL;

  gdb_AssumeLocked;

  /* Make sure it is linked.  */

  pwr_Assert(op->u.c.flags.m & gdb_mCo_inTouchList);
  pwr_Assert(pool_QisLinked(NULL, gdbroot->pool, &op->u.c.cache_ll));

  if (op->u.c.nChild > 0) {	/* We have children, move to cachePend.  */
    if (gdbroot->db->log.b.cacheTrim)
      printf("Object: %s %s, moved to cache pending\n", op->g.f.name.orig, cdh_ObjidToString(NULL, op->g.oid, 0));
    cvol_QmoveSucc(op, cvol_Qget(op), &gdbroot->db->cachePend);
    return;
  }

  if (gdbroot->db->log.b.cacheTrim)
    printf("Object: %s %s, removed from cache\n", op->g.f.name.orig, cdh_ObjidToString(NULL, op->g.oid, 0));
  if (op->l.por != pool_cNRef) {
    pop = pool_Address(NULL, gdbroot->pool, op->l.por);
    pop->u.c.nChild--;
  }

  cvol_Qremove(op);

  cvol_UnlinkObject(NULL, vp, op, link);
  vol_UnlinkObject(NULL, vp, op, link);

  cvol_QmoveSucc(op, NULL, &gdbroot->db->cacheFree);

  if (pop != NULL && pop->u.c.flags.b.cachePend && pop->u.c.nChild == 0) {
    cvol_FreeObject(NULL, vp, pop, link);
  }
}

/* .  */

gdb_sObject *
cvol_LinkObject	(
  pwr_tStatus		*sts,
  gdb_sVolume		*vp,
  gdb_sObject		*op,
  pwr_tBitMask		link
)
{
  gdb_sObject		*pop;

  gdb_AssumeLocked;

  /* Make sure it is not linked already.  */

  pwr_Assert(!(op->u.c.flags.m & gdb_mCo_inTouchList));
  pwr_Assert(!pool_QisLinked(NULL, gdbroot->pool, &op->u.c.cache_ll));

  if (op->l.por != pool_cNRef) {
    pop = pool_Address(NULL, gdbroot->pool, op->l.por);
    pop->u.c.nChild++;
  }
  
  cvol_QmoveSucc(op, NULL, &gdbroot->db->cacheNew);

  return op;
}

/* Allocate space for an object header, fill it with data,
   link it into the 'cacheNew' touch list, and fill
   in all the other local linkage.

   If the object already is in the cache, then this operation
   is a noop.  */

gdb_sObject *
cvol_LoadObject (
  pwr_tStatus		*sts,
  gdb_sNode		*np,  
  gdb_sVolume		*vp,
  net_sGobject		*gop
)
{
  gdb_sObject		*op;
  pwr_tStatus		lsts;

  gdb_AssumeLocked;

  pwr_Assert(cdh_ObjidIsNotNull(gop->oid));

  op = hash_Search(sts, gdbroot->oid_ht, &gop->oid);
  if (op != NULL) return op;

  op = cvol_AllocObject(sts, np, vp);
  if (op == NULL) errh_Bugcheck(2, "");

  op->g = *gop;

  op = vol_LinkObject(&lsts, vp, op, vol_mLink_load);
  if (op == NULL) errh_Bugcheck(lsts, "vol_LinkObject");

  op = cvol_LinkObject(&lsts, vp, op, vol_mLink_load);
  if (op == NULL) errh_Bugcheck(lsts, "cvol_LinkObject");

  if (!np->cclassSupport) {
    op->u.c.flags.b.classChecked = 1;
    op->u.c.flags.b.classEqual = 1;
  }
  

  pwr_Return(op, sts, GDH__SUCCESS);
}

/* Force trim of one object header.
   If a cache touch queue has reached its maximum
   remove from tail and insert at head in next queue.
   Do this until no next queue is defined.
   This function can be used to trim any of the following queues:

      cacheVol, cacheNode, cacheCom and cacheNew

      cacheVol -> cacheNode -> cacheCom -> cacheOld
      cacheNew -> cacheOld

   cacheOld is trimmed with TrimDelete and
   cachePending is trimmed implicitly */

void
cvol_QforcedTrim (
  gdb_sTouchQ		*fqp
)
{
  gdb_sTouchQ		*tqp;
  pool_sQlink		*ol;
  gdb_sObject		*op;

  gdb_AssumeLocked;

  if (fqp->next == pool_cNRef)
    return;

  if (gdbroot->db->log.b.cacheTrim)
    printf("Start forced trim %d, %d < %d < %d\n", fqp->flags.m, fqp->lc_min, fqp->lc, fqp->lc_max);
  tqp = pool_Address(NULL, gdbroot->pool, fqp->next);

  if (fqp->lc > 0) {
    ol = pool_Qpred(NULL, gdbroot->pool, &fqp->lh);
    pwr_Assert(ol != &fqp->lh);
    op = pool_Qitem(ol, gdb_sObject, u.c.cache_ll);
    pwr_Assert(op != NULL);
    cvol_QmoveSucc(op, fqp, tqp);
  }

  cvol_QforcedTrim(tqp);
  if (gdbroot->db->log.b.cacheTrim)
    printf("Stop forced trim %d, %d < %d < %d\n", fqp->flags.m, fqp->lc_min, fqp->lc, fqp->lc_max);
}

/* Return pointer to the cache queue an object is in.  */

gdb_sTouchQ *
cvol_Qget (
  gdb_sObject		*op
)
{
  gdb_sVolume		*vp;
  gdb_sNode		*np;

  gdb_AssumeLocked;

  if (op->u.c.flags.b.cacheVol) {
    vp = pool_Address(NULL, gdbroot->pool, op->l.vr);
    return &vp->u.c.cacheVol;
  } else if (op->u.c.flags.b.cacheNode) {
    vp = pool_Address(NULL, gdbroot->pool, op->l.vr);
    np = pool_Address(NULL, gdbroot->pool, vp->l.nr);
    return &np->cacheNode;
  } else if (op->u.c.flags.b.cacheCom) {
    return &gdbroot->db->cacheCom;
  } else if (op->u.c.flags.b.cacheOld) {
    return &gdbroot->db->cacheOld;
  } else if (op->u.c.flags.b.cachePend) {
    return &gdbroot->db->cachePend;
  } else if (op->u.c.flags.b.cacheNew) {
    return &gdbroot->db->cacheNew;
  } else if (op->u.c.flags.b.cacheFree) {
    return &gdbroot->db->cacheFree;
  } else if (op->u.c.flags.b.cacheLock) {
    vp = pool_Address(NULL, gdbroot->pool, op->l.vr);
    return &vp->u.c.cacheLock;
  } else {
    errh_Bugcheck(2, "Object cache queue inconsistency");
  }
}

/* Move a cached object from one touch queue to tail of another.
   If object is not in a que, it will be inserted only.  */

void
cvol_QmovePred (
  gdb_sObject		*op,
  gdb_sTouchQ		*fqp,
  gdb_sTouchQ		*tqp
)
{

  gdb_AssumeLocked;

  pwr_Assert(op != NULL);

  if (!op->l.flags.b.isCached || op->u.c.flags.b.cacheLock) return;

  if (fqp != NULL) {
    pool_Qremove(NULL, gdbroot->pool, &op->u.c.cache_ll);
    fqp->lc--;
    op->u.c.flags.m &= ~fqp->flags.m;   /* Remove bit for old que.  */
  } else {
    pwr_Assert(!(op->u.c.flags.m & gdb_mCo_inTouchList));
  }

  pool_QinsertPred(NULL, gdbroot->pool, &op->u.c.cache_ll, &tqp->lh);
  tqp->lc++;
  op->u.c.flags.m |= tqp->flags.m;    /* Set bit for new que.  */

}

/* Move a cached object from one touch queue to head of another.
   If object is not in a que, it will be inserted only.  */

void
cvol_QmoveSucc (
  gdb_sObject		*op,
  gdb_sTouchQ		*fqp,
  gdb_sTouchQ		*tqp
)
{

  gdb_AssumeLocked;

  pwr_Assert(op != NULL);

  if (!op->l.flags.b.isCached || op->u.c.flags.b.cacheLock) return;

  if (fqp != NULL) {
    pool_Qremove(NULL, gdbroot->pool, &op->u.c.cache_ll);
    fqp->lc--;
    op->u.c.flags.m &= ~fqp->flags.m;   /* Remove bit for old que.  */
  } else {
    pwr_Assert(!(op->u.c.flags.m & gdb_mCo_inTouchList));
  }

  pool_QinsertSucc(NULL, gdbroot->pool, &op->u.c.cache_ll, &tqp->lh);
  tqp->lc++;
  op->u.c.flags.m |= tqp->flags.m;    /* Set bit for new que.  */
}

/* .  */

void
cvol_Qremove (
  gdb_sObject		*op
)
{
  gdb_sTouchQ		*fqp;

  gdb_AssumeLocked;

  pwr_Assert(op != NULL);
  pwr_Assert((op->u.c.flags.m & gdb_mCo_inTouchList) != 0);
  
  fqp = cvol_Qget(op);

  pool_Qremove(NULL, gdbroot->pool, &op->u.c.cache_ll);
  fqp->lc--;
  op->u.c.flags.m &= ~fqp->flags.m;   /* Remove bit for que.  */
}

/* Trim the cache touch queues.
   If a cache touch queue has reached its maximum
   remove from tail and insert at head in next queue.
   Do this until no next queue is defined.
   This function can be used to trim any of the following queues:

      cacheVol, cacheNode, cacheCom and cacheNew

      cacheVol -> cacheNode -> cacheCom -> cacheOld
      cacheNew -> cacheOld

   cacheOld is trimmed with TrimDelete and
   cachePending is trimmed implicitly */

void
cvol_Qtrim (
  gdb_sTouchQ		*fqp
)
{
  gdb_sTouchQ		*tqp;
  pool_sQlink		*ol;
  gdb_sObject		*op;

  gdb_AssumeLocked;

  if (fqp->next == pool_cNRef || fqp->lc <= fqp->lc_max)
    return;

  if (gdbroot->db->log.b.cacheTrim)
    printf("Start trim %d, %d < %d < %d\n", fqp->flags.m, fqp->lc_min, fqp->lc, fqp->lc_max);
  tqp = pool_Address(NULL, gdbroot->pool, fqp->next);

  while (fqp->lc > fqp->lc_max) {
    ol = pool_Qpred(NULL, gdbroot->pool, &fqp->lh);
    if (ol == &fqp->lh) break;
    pwr_Assert(fqp->lc > 0);
    op = pool_Qitem(ol, gdb_sObject, u.c.cache_ll);
    if (op == NULL) errh_Bugcheck(2, "");
    cvol_QmoveSucc(op, fqp, tqp);
  }

  cvol_Qtrim(tqp);
  if (gdbroot->db->log.b.cacheTrim)
    printf("Stop trim %d, %d < %d < %d\n", fqp->flags.m, fqp->lc_min, fqp->lc, fqp->lc_max);
}

#if 0
/* .  */

void
cvol_RemoveObject (
  gdb_sObject		*op,
  pwr_tBoolean		flush
)
{
  gdb_sObject		*pop = NULL;
  gdb_sVolume		*vp = NULL;

  /* Make sure it is linked.  */

  pwr_Assert(op->u.c.flags.m & gdb_mCo_inTouchList);
  pwr_Assert(pool_QisLinked(NULL, gdbroot->pool, &op->u.c.cache_ll));

  if (!flush && op->u.c.nChild > 0) {	/* We have children, move to cachePend.  */
    cvol_QmoveSucc(op, cvol_Qget(op), &gdbroot->db->cachePend);
    return;
  }

  if (!flush && op->l.por != pool_cNRef) {
    pop = pool_Address(NULL, gdbroot->pool, op->l.por);
    pop->u.c.nChild--;
  }
  vp = pool_Address(NULL, gdbroot->pool, op->l.vr);

  cvol_UnlinkObject(NULL, vp, op, 0);
  vol_UnlinkObject(NULL, vp, op, 0);

  cvol_Qremove(op);
  memset(op, 0, sizeof(*op));
  cvol_QmoveSucc(op, NULL, &gdbroot->db->cacheFree);

  if (!flush && pop->u.c.flags.b.cachePend && pop->u.c.nChild == 0) {
    cvol_UnlinkObject(NULL, vp, pop, vol_mLink_cacheFlush);
  }
}
#endif

/* .  */

void
cvol_UnlinkObject (
  pwr_tStatus		*sts,
  gdb_sVolume		*pp,
  gdb_sObject		*op,
  pwr_tBitMask		link
)
{

/* TODO */
  return;
}
