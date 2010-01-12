/* 
 * Proview   $Id: rt_cvolsm.c,v 1.5 2006-09-14 14:16:07 claes Exp $
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

/* rt_cvolsm.c -- Cached volumes, server monitor API.
   This module contains the cache handling routines.  */

#if defined(OS_ELN)
# include $vaxelnc
#else
# include <stdio.h>
# include <string.h>
#endif

#include "pwr.h"
#include "rt_errh.h"
#include "rt_gdh_msg.h"
#include "rt_gdb.h"
#include "rt_vol.h"
#include "rt_cvol.h"
#include "rt_cvolsm.h"
#include "rt_net.h"
#include "rt_ndc.h"

static void
respondError (
  qcom_sGet		*msg,
  pwr_tObjid		oid,
  pwr_tStatus		rspsts
);

static void
respondObject (
  qcom_sGet		*msg,
  gdb_sObject		*op,
  pwr_tInt32		lcount,
  pwr_tInt32		rcount
);


/* Common cache error responder.  */

static void
respondError (
  qcom_sGet		*get,
  pwr_tObjid		oid,
  pwr_tStatus		rspsts
)
{
  pwr_tStatus		sts;
  net_sObjectR		*rsp;
  qcom_sPut		put;

  rsp = net_Alloc(&sts, &put, sizeof(*rsp), net_eMsg_objectR);
  if (rsp == NULL) errh_Bugcheck(sts, "net_Alloc");

  rsp->sts    = rspsts;
  rsp->oid    = oid;
  rsp->count  = 0;

  if (!net_Reply(&sts, get, &put, 0))
    errh_Bugcheck(sts, "net_Reply");
}
 
/* Send parent path and sibling environment of an object.  */

static void
respondObject (
  qcom_sGet		*get,
  gdb_sObject		*op,
  pwr_tInt32		lcount,
  pwr_tInt32		rcount
)
{
  pwr_tStatus		sts;
  qcom_sPut		put;
  gdb_sObject		*pop;
  net_sObjectR		*rsp;
  net_sGobject		*gop;
  net_sGobject		*go[net_cObjectMaxCount];
  pwr_tUInt32		count;
  pwr_tUInt32		pcount;
  pwr_tUInt32		size;
  pwr_tInt32		i;
  pool_sQlink		*sol;
  gdb_sObject		*sop;
      
  gdb_AssumeLocked;

  /* The parents must be first in the message, so the receiver can link
     the cache objects. They must be in top-down-order. */

  pop = op;
  pcount = 0;
  count = 0;
  while (pop->g.oid.oix != pwr_cNObjectIx && count < net_cObjectMaxCount - 1) {
    pop = pool_Address(NULL, gdbroot->pool, pop->l.por);
    go[count++] = &pop->g;
  }

  pcount = count;
  pop = pool_Address(NULL, gdbroot->pool, op->l.por);

  if (pop != NULL) {
    /* Left siblings. (At most lcount of them.) */

    for (
      i = 0, sol = pool_Qpred(NULL, gdbroot->pool, &op->u.n.sib_ll);
      i < lcount && sol != &pop->u.n.sib_lh && count < net_cObjectMaxCount - 1;
      i++, sol = pool_Qpred(NULL, gdbroot->pool, sol)
    ) {
      sop = pool_Qitem(sol, gdb_sObject, u.n.sib_ll);
      go[count++] = &sop->g;
    }

    /* Right siblings. (At most rcount of them.) */

    for (
      i = 0, sol = pool_Qsucc(NULL, gdbroot->pool, &op->u.n.sib_ll);
      i < rcount && sol != &pop->u.n.sib_lh && count < net_cObjectMaxCount - 1;
      i++, sol = pool_Qsucc(NULL, gdbroot->pool, sol)
    ) {
      sop = pool_Qitem(sol, gdb_sObject, u.n.sib_ll);
      go[count++] = &sop->g;
    }
  }

  /* Build response message.  */

  size = sizeof(net_sObjectR) + count * sizeof(net_sGobject);
  rsp = net_Alloc(&sts, &put, size, net_eMsg_objectR);
  if (rsp == NULL) {
    printf("NETH: could not allocate pams buffer for Cache send response, sts: %d\n", sts);
    respondError(get, op->g.oid, sts);
    return;
  }
  gop = &rsp->g[0];

  /* Copy parent objects.  */
  for (i = pcount - 1; i >= 0; i--)
    *gop++ = *(go[i]);

  /* Copy target object.  */
  *gop++ = op->g;

  /* Copy sibling objects.  */
  for (i = pcount; i < count; i++)
    *gop++ = *(go[i]);

  rsp->sts    = sts;
  rsp->oid    = op->g.oid;
  rsp->count  = count + 1;

  if (!net_Reply(&sts, get, &put, 0))
    errh_Bugcheck(sts, "net_Reply");
}

/* .  */

gdb_sMountedOn *
cvolsm_AddMountedOn (
  pwr_tStatus		*sts,
  pwr_tVolumeId		vid,
  gdb_sNode		*np
)
{
  gdb_sMountedOn	*mop;
  gdb_sVolume		*vp;

  gdb_AssumeLocked;

  vp = hash_Search(sts, gdbroot->vid_ht, &vid);
  if (vp == NULL) return NULL;   /* !!! Todo !!! error handling.  */

  mop = (gdb_sMountedOn *) pool_Alloc(NULL, gdbroot->pool, sizeof(*mop));

  mop->nid = np->nid;
  mop->nr  = pool_Reference(NULL, gdbroot->pool, np);
  mop->vid = vid;
  mop->vr  = pool_Reference(NULL, gdbroot->pool, vp);
    
  pool_QinsertSucc(NULL, gdbroot->pool, &mop->nodmo_ll, &np->nodmo_lh);
  pool_QinsertSucc(NULL, gdbroot->pool, &mop->volmo_ll, &vp->u.n.volmo_lh);

  return mop;
}

/* Flush all server information for a node.

   All mounted on information
   All san servers are deleted.  */

void
cvolsm_FlushNode (
  pwr_tStatus		*sts,
  gdb_sNode		*np
)
{
  pool_sQlink		*mol;
  gdb_sMountedOn	*mop;

  gdb_AssumeLocked;

  for (
    mol = pool_Qsucc(NULL, gdbroot->pool, &np->nodmo_lh);
    mol != &np->nodmo_lh;
    mol = pool_Qsucc(NULL, gdbroot->pool, &np->nodmo_lh)
  ) {
    mop = pool_Qitem(mol, gdb_sMountedOn, nodmo_ll);
    cvolsm_RemoveMountedOn(NULL, mop);
  }
}

/* Handle GetObjectInfo message.  */

void
cvolsm_GetObjectInfo (
  qcom_sGet		*get
)
{
  pwr_tStatus		sts;
  mvol_sAttribute	Attribute;
  mvol_sAttribute	*ap;
  int			size;
  net_sGetObjectInfoR	*rmp;
  void			*p = NULL;
  net_sGetObjectInfo	*mp = get->data;
  qcom_sPut		put;
  gdb_sNode		*np;
  cdh_uTypeId		cid;
  gdb_sClass		*cp;

  gdb_AssumeUnlocked;

  size = mp->aref.Size + sizeof(net_sGetObjectInfoR) - sizeof(rmp->info);
  size = (size + 3) & ~3;   /* Size up to nearest multiple of 4.  */
  rmp = net_Alloc(&sts, &put, size, net_eMsg_getObjectInfoR);
  if (EVEN(sts)) return;

  gdb_ScopeLock {
    
    np = hash_Search(NULL, gdbroot->nid_ht, &get->sender.nid);
    pwr_Assert(np != NULL);

    memset( &Attribute, 0, sizeof(Attribute));
    ap = vol_ArefToAttribute(&sts, &Attribute, &mp->aref, gdb_mLo_owned, vol_mTrans_alias);
    if (ap == NULL || ap->op == NULL) break;

    p = vol_AttributeToAddress(&sts, ap);

  } gdb_ScopeUnlock;

  if (p != NULL) {
    size = mp->aref.Size;
    cid.pwr = mp->aref.Body;
    cid.c.bix = 0;	/* To get the class id.  */
    cp = hash_Search(&sts, gdbroot->cid_ht, &cid.pwr);
    if (cp != NULL)    
      ndc_ConvertData(&sts, np, cp, &mp->aref, rmp->info, p, (pwr_tUInt32 *)&size, ndc_eOp_encode, mp->aref.Offset, 0);
  }
  rmp->aref = mp->aref;
  rmp->sts  = sts;
  rmp->size = mp->aref.Size;

  net_Reply(&sts, get, &put, 0);
}

/* Handle a NameToObject message.  */

void
cvolsm_NameToObject (
  qcom_sGet		*get
)
{
  pwr_tStatus		sts;
  gdb_sObject		*op = NULL;
  cdh_sParseName	ParseName;
  cdh_sParseName	*pn;
  net_sNameToObject	*mp = get->data;

  gdb_AssumeUnlocked;

  pn = cdh_ParseName(&sts, &ParseName, mp->poid, mp->name, 0);
  if (pn == NULL) {
    respondError(get, pwr_cNObjid, sts);
    return;
  }

  gdb_ScopeLock {

    /* Don't allow mount translation.  */

    op = vol_NameToObject(&sts, pn, gdb_mLo_owned, mp->trans & ~vol_mTrans_mount);
    if (op == NULL) {
      respondError(get, pwr_cNObjid, sts);
    } else {
      respondObject(get, op, mp->lcount, mp->rcount);
    }

  } gdb_ScopeUnlock;
}

/* Handle OidToObject message.  */

void
cvolsm_OidToObject (
  qcom_sGet		*get
)
{
  pwr_tStatus		sts;
  gdb_sObject		*op;
  net_sOidToObject	*mp = get->data;

  gdb_AssumeUnlocked;

  gdb_ScopeLock {

    op = hash_Search(&sts, gdbroot->oid_ht, &mp->oid);
    if (op == NULL || !op->l.flags.b.isOwned) {
      respondError(get, pwr_cNObjid, GDH__NOSUCHOBJ);
    } else {
      respondObject(get, op, mp->lcount, mp->rcount);
    }

  } gdb_ScopeUnlock;
}

/* .  */

void
cvolsm_RemoveMountedOn (
  pwr_tStatus		*sts,
  gdb_sMountedOn	*mop
)
{

  gdb_AssumeLocked;

  pool_Qremove(NULL, gdbroot->pool, &mop->nodmo_ll);
  pool_Qremove(NULL, gdbroot->pool, &mop->volmo_ll);

  pool_Free(NULL, gdbroot->pool, mop);
}

/* Handle SetObjectInfo message.  */

void
cvolsm_SetObjectInfo (
  qcom_sGet		*get
)
{
  pwr_tStatus		sts;
  mvol_sAttribute	Attribute;
  mvol_sAttribute	*ap;
  void			*p;
  net_sSetObjectInfoR	*rmp;
  qcom_sPut		put;
  net_sSetObjectInfo	*mp = get->data;
  gdb_sNode		*np;
  int                   size;
  cdh_uTypeId		cid;
  gdb_sClass		*cp;

  gdb_AssumeUnlocked;

  rmp = net_Alloc(&sts, &put, sizeof(*rmp), net_eMsg_setObjectInfoR);
  if (EVEN(sts)) return;

  gdb_ScopeLock {
    
    np = hash_Search(NULL, gdbroot->nid_ht, &get->sender.nid);
    pwr_Assert(np != NULL);

    
    memset( &Attribute, 0, sizeof(Attribute));
    ap = vol_ArefToAttribute(&sts, &Attribute, &mp->aref, gdb_mLo_owned, vol_mTrans_alias);
    if (ap == NULL || ap->op == NULL) break;

    p = vol_AttributeToAddress(&sts, ap);

  } gdb_ScopeUnlock;

  if (p != NULL) {
    size = mp->aref.Size;
    cid.pwr = mp->aref.Body;
    cid.c.bix = 0;	/* To get the class id.  */
    cp = hash_Search(&sts, gdbroot->cid_ht, &cid.pwr);
    if (cp != NULL)    
      ndc_ConvertData(&sts, np, cp, &mp->aref, p, mp->info, (pwr_tUInt32 *)&size, ndc_eOp_decode, mp->aref.Offset, 0);
  }

  rmp->aref = mp->aref;
  rmp->sts  = sts;

  net_Reply(&sts, get, &put, 0);
}
