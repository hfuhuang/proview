/* rt_subsm.c -- Object data subscription, server monitor functions.

   PROVIEW/R
   Copyright (C) 1997-98 by Mandator AB.

   This module contains routines to handle subscriptions.
   Those routines can NOT be called in a job context other than
   that of the nethandler.  */

#if defined(OS_ELN)
# include $vaxelnc
#else
# include <stdlib.h>
# include <string.h>
#endif

#include "pwr.h"
#include "co_cdh.h"
#include "co_time.h"
#include "rt_gdh_msg.h"
#include "rt_errh.h"
#include "rt_pool.h"
#include "rt_gdb.h"
#include "rt_sub.h"
#include "rt_net.h"
#include "rt_ndc.h"
#include "rt_vol.h"
#include "rt_subs.h"
#include "rt_subsm.h"


/* Find/allocate a 'buffer' entry.
   Database must be locked by caller.  */

static void
associateBuffer (
  sub_sServer		*sp
)
{
  pwr_tStatus		sts;
  sub_sBuffer		*bp;
  pwr_tBoolean		found;
  gdb_sNode		*np;
  pool_sQlink		*bl;
  mvol_sAttribute	Attribute;
  mvol_sAttribute	*ap;

  gdb_AssumeLocked;

  np = hash_Search(&sts, gdbroot->nid_ht, &sp->nid);
  if (np == NULL) errh_Bugcheck(sts, "node not found");

  /* Search (linear!) for a suitable buffer.  */

  for (
    found = FALSE,
    bl = pool_Qsucc(NULL, gdbroot->pool, &np->nodsubb_lh);
    bl != &np->nodsubb_lh;
    bl = pool_Qsucc(NULL, gdbroot->pool, bl)
  ) {
    bp = pool_Qitem(bl, sub_sBuffer, nodsubb_ll);

    if (bp->totsize + sp->aref.Size < sub_cMsgSize) {
      /* Within size limit */
      if ((bp->dt < sp->dt * 2) && (bp->dt * 2 > sp->dt)) {
	/* Time is within reasonable range.
	   Pick this SUBBUF! */

	found = TRUE;
	break;
      }
    }
  }

  /* Allocate a new buffer if none was found.  */

  if (!found) {
    bp = pool_Alloc(NULL, gdbroot->pool, sizeof(*bp));

    pool_Qinit(NULL, gdbroot->pool, &bp->subb_ll);
    pool_Qinit(NULL, gdbroot->pool, &bp->nodsubb_ll);
    pool_Qinit(NULL, gdbroot->pool, &bp->bufsubs_lh);

    bp->nid = sp->nid;
    bp->dt  = sp->dt;
    pool_QinsertSucc(NULL, gdbroot->pool, &bp->subb_ll, &gdbroot->db->subb_lh);
    gdbroot->db->subb_lc++;
    pool_QinsertSucc(NULL, gdbroot->pool, &bp->nodsubb_ll, &np->nodsubb_lh);

    subsm_ActivateBuffer(bp, 200);

  }

  /* Tie buffer and server structures together.
     Initiate data pointer and increment the refrenced object's sub counter.  */

  do {
    sp->data = pool_cNRef;

    ap = vol_ArefToAttribute(&sp->sts, &Attribute, &sp->aref, gdb_mLo_owned, vol_mTrans_alias);
    if (ap == NULL || ap->op == NULL) break;

    sp->data = vol_AttributeToReference(&sp->sts, ap);
    if (sp->data == pool_cNRef) break;

    ap->op->u.n.subcount++;
  } while (0);

  pool_QinsertPred(NULL, gdbroot->pool, &sp->bufsubs_ll, &bp->bufsubs_lh);

  sp->br = pool_ItemReference(NULL, gdbroot->pool, bp);
  bp->totsize += sp->aref.Size;
  bp->bufsubs_lc++;
}

/* Add this buffer to the timer monitor queue.
   Caller must lock database.
   The timer is set to 0.2 seconds for this first transmission.
   Ideally, the buffer should be sent off right away, but there
   is a very large probability for more servers to be put in
   this buffer.
   If the buffer is already queued then this is a noop.  */

void
subsm_ActivateBuffer (
  sub_sBuffer		*bp,
  pwr_tUInt32		dt
)
{

  gdb_AssumeLocked;

  if (!bp->queued) {
    bp->tmonq.dt = MIN(bp->dt, dt);
    bp->tmonq.type = gdb_eTmon_subbCheck;
    pool_QinsertPred(NULL, gdbroot->pool, &bp->tmonq.ll, &gdbroot->db->tmonq_lh);
    bp->queued = TRUE;
    bp->sts = GDH__SUCCESS;
  }
}

/* Handle a subadd message.
   The database must NOT be locked upon entry.  */

void
subsm_Add (
  qcom_sGet		*get
)
{
  pwr_tInt32		i;
  pwr_tStatus		sts;
  sub_sServer		*sp;
  sub_sServer		*s2p;
  net_sSubSpec		*specp;
  gdb_sNode		*np;
  net_sSubAdd		*mp = get->data;

  gdb_AssumeUnlocked;

  gdb_ScopeLock {

    np = hash_Search(&sts, gdbroot->nid_ht, &get->sender.nid);
    if (np == NULL) {
      errh_Error("subsm_Add, node (%s), is not known",
	cdh_NodeIdToString(NULL, get->sender.nid, 1, 0));
      break;
    }
 
    specp = &mp->spec[0];
    for (i=0; i < mp->count; i++, specp++) {
      /* For all entries in the message.  */
      sp = pool_Alloc(NULL, gdbroot->pool, sizeof(*sp));

      sp->sid	      = specp->sid;
      sp->nid	      = np->nid;
      sp->sub_by_name = specp->sub_by_name;
      sp->aref	      = specp->aref;
      sp->dt	      = specp->dt;

      /* Enter server in list of all servers */

      pool_QinsertPred(NULL, gdbroot->pool, &sp->subs_ll, &gdbroot->db->subs_lh);
      gdbroot->db->subs_lc++;
      pool_QinsertPred(NULL, gdbroot->pool, &sp->nodsubs_ll, &np->nodsubs_lh);

      /* Enter server in the server table (keyed by pwr_tSubid) */

      do {
	s2p = hash_Insert(&sts, gdbroot->subs_ht, sp);
	if (s2p != NULL) break;

	s2p = hash_Search(&sts, gdbroot->subs_ht, &sp->sid);
	if (s2p == NULL)
	  errh_Bugcheck(sts, "server not found");
	else
	  subs_DeleteServer(s2p);
      } while (1);

      /* Associate the server entry with a buffer. */

      associateBuffer(sp);
    }

  } gdb_ScopeUnlock;

#if 0
  errh_Info("subsm_Add %d", i);
#endif
}

/* Delete buffers and servers associated with a certain node.  */

void
subsm_FlushNode (
  pwr_tStatus		*sts,
  gdb_sNode		*np
)
{
  pool_sQlink		*ssl;

  gdb_AssumeLocked;

  for (
    ssl = pool_Qsucc(NULL, gdbroot->pool, &np->nodsubs_lh);
    ssl != &np->nodsubs_lh;
    ssl = pool_Qsucc(NULL, gdbroot->pool, &np->nodsubs_lh)
  ) {
    subs_DeleteServer(pool_Qitem(ssl, sub_sServer, nodsubs_ll));
  }
  
}

/* Handle a subrem message.
   The database must NOT be locked upon entry.  */

void
subsm_Remove (
  qcom_sGet		*get
)
{
  pwr_tStatus		sts;
  pwr_tInt32		i;
  sub_sServer		*sp;
  net_sSubRemove	*mp = get->data;

  gdb_AssumeUnlocked;

  gdb_ScopeLock {

    for (i = 0; i < mp->count; i++) {
      sp = hash_Search(&sts, gdbroot->subs_ht, &mp->sid[i]);
      if (sp != NULL) {
	subs_DeleteServer(sp);
      }
    }

  } gdb_ScopeUnlock;

#if 0
  errh_Info("subsm_Remove %d", i);
#endif
}

/* Build & send data for a subbuf.
   YES is returned if the buffer should be queued
   for retransmission, NO else.
   The database must be locked upon entry.  */

pwr_tBoolean
subsm_SendBuffer (
  sub_sBuffer		*bp
)
{
  pwr_tStatus		sts = 1;
  net_sSubMessage	*mp;
  net_sSubData		*dp;
  sub_sServer		*sp;
  sub_sClient		*cp;
  gdb_sNode		*np;
  qcom_sQid		tgt;
  pwr_tUInt32		size;
  pwr_tInt32		subdatahdrsize;
  pwr_tInt32		i;
  pwr_tBoolean		remote = (bp->nid != gdbroot->db->nid);
  pool_sQlink		*sl;
  void			*data;
  gdb_sClass		*classp;
  cdh_uTypeId		cid;

  gdb_AssumeLocked;

  if (bp->bufsubs_lc == 0) {
    /* This buffer is not used and can be dequeued.  */

    pool_Qremove(NULL, gdbroot->pool, &bp->subb_ll);
    pool_Qremove(NULL, gdbroot->pool, &bp->nodsubb_ll);
    gdbroot->db->subb_lc--;
    pool_Free(NULL, gdbroot->pool, bp);
    return NO;
  }

  if (remote) {

    /* Build a buffer containing the update message */

    subdatahdrsize = sizeof(*dp) - sizeof(dp->data);
    size =  bp->totsize +				/* size of data */
	    bp->bufsubs_lc * subdatahdrsize +		/* size of data hdrs */
	    sizeof(*mp) - sizeof(mp->subdata);		/* size of msg hdr */

    size = (size + 3) & ~3;   /* Size up to nearest multiple of 4.  */

    mp = calloc(1, size);

    /* Fill in buffer */

    mp->count = 0;
    dp = (net_sSubData *)&mp->subdata;
  }

  np = hash_Search(NULL, gdbroot->nid_ht, &bp->nid);
  pwr_Assert(np != NULL);

  for (	/* For all subscription servers of this buffer.  */
    i = 0, sl = pool_Qsucc(NULL, gdbroot->pool, &bp->bufsubs_lh);
    sl != &bp->bufsubs_lh && i < bp->bufsubs_lc;    /* !!! To do !!! Redundant check.  */
    i++, sl = pool_Qsucc(NULL, gdbroot->pool, sl)
  ) {
    sp = pool_Qitem(sl, sub_sServer, bufsubs_ll);

    if (!remote) {
      /* If target is on same node we can optimize a lot... */

      cp = hash_Search(&sp->sts, gdbroot->subc_ht, &sp->sid);

      if (cp != NULL) {
	if (cp->userdata == pool_cNRef) {
	  /* If no userdata buffer, then we don't need to transfer */
	  ;
	} else {
	  /* Userdata buffer needs to be filled... */
	  if (sp->data == pool_cNRef) {
	    sp->sts = GDH__NOSUCHOBJ;
	  } else {
	    data = pool_Address(&sp->sts, gdbroot->rtdb, sp->data);
	  }
	  cp->sts = sp->sts;
	  if (ODD (cp->sts)) {
	    memcpy(pool_Address(NULL, gdbroot->rtdb, cp->userdata), data, MIN(cp->usersize, cp->aref.Size));
	  }	    
	}
	cp->old = FALSE;
	cp->count++;
	clock_gettime(CLOCK_REALTIME, &cp->lastupdate);
	sp->count++;
      }
    } else {
      /* Remote subscription.  */
      dp->sid = sp->sid;
      dp->size = sp->aref.Size;

      /* Verify the pwr_tObjid */

      data = pool_Address(&sp->sts, gdbroot->rtdb, sp->data);
      dp->sts = sp->sts;

      if (ODD(sp->sts)) {
        size = sp->aref.Size;
        cid.pwr = sp->aref.Body;
        cid.c.bix = 0;	/* To get the class id.  */
        classp = hash_Search(&sts, gdbroot->cid_ht, &cid.pwr);
        if (classp == NULL)
	  ndc_ConvertData(&dp->sts, np, classp, &sp->aref, dp->data, data, &size, ndc_eOp_encode, sp->aref.Offset, 0);
	sp->count++;
	mp->count++;
      }

      /* Advance pointer to next sdata slot */

      dp = (net_sSubData *)((pwr_tInt32)dp + sp->aref.Size + subdatahdrsize);
    }
  }

  /* Requeue the buffer */

  subsm_ActivateBuffer(bp, bp->dt);

  if (remote) {
    tgt.qix = net_cProcHandler;
    tgt.nid = np->nid;

    gdb_Unlock;
  
      net_Put(&sts, &tgt, mp, net_eMsg_subData, size);

    gdb_Lock;

    free(mp);
    bp->sts = sts;
  }

  if (ODD(sts)) clock_gettime(CLOCK_REALTIME, &bp->lastupdate);

  return YES;
}
