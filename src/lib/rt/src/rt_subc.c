/* 
 * Proview   $Id: rt_subc.c,v 1.8 2008-04-11 08:15:55 claes Exp $
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

/* rt_subc.c -- Object data subscription, client side

   This module contains routines to handle subscription.
   Those routines can be called from the GDH API.  */

#if defined(OS_ELN)
# include $vaxelnc
#else 
# include <stdlib.h>
# include <string.h>
#endif

#include "pwr.h"
#include "rt_gdh_msg.h"
#include "rt_hash_msg.h"
#include "co_tree.h"
#include "rt_errh.h"
#include "rt_gdb.h"
#include "rt_hash.h"
#include "rt_pool.h"
#include "rt_vol.h"
#include "rt_mvol.h"
#include "rt_cmvolc.h"
#include "rt_ndc.h"
#include "rt_net.h"
#include "rt_sub.h"
#include "rt_subc.h"

typedef struct {
  tree_sNode		node;
  pwr_tNodeId		nid;
  unsigned int		cnt;
  net_sSubAdd		*msg;
} sAdd;    

typedef struct {
  tree_sNode		node;
  pwr_tNodeId		nid;
  unsigned int		cnt;
  net_sSubRemove	*msg;
} sRemove;    

static pwr_tUInt32	default_dt = 1000;   /* Default value of update delta time */
				   /* Cannot be zero */

static pwr_tUInt32	default_tmo = 10000; /* Default value of timeout */
				   /* Zero means infinite, i.e. no timeout */


static void
activateTimeoutWatch (
  sub_sClient		*cp
);

static void
cancelTimeoutWatch (
  sub_sClient		*cp
);

static void
deleteClient (
  sub_sClient		*cp
);

static gdb_sNode *
testClient (
  pwr_tStatus		*sts,
  sub_sClient		*cp
);


/* This routine posts a subscription client entry for timeout watching. The
   tmo field of the entry must be filled in. The routine
   detects zero = infinite timeouts, and takes approprate action.  */

static void
activateTimeoutWatch (
  sub_sClient		*cp
)
{

  gdb_AssumeLocked;

  /* Handle only remotely located objects with a tmo != 0.  */

  if ((!cp->tmoactive) && (cp->nid != pwr_cNNodeId) && (cp->tmo != 0)) {
    pool_QinsertPred(NULL, gdbroot->pool, &cp->subt_ll, &gdbroot->db->subt_lh);
    cp->tmoactive = TRUE;
    gdbroot->db->subt_lc++;
  }
}

/* Remove a subscription client entry from timeout watching.  */

static void
cancelTimeoutWatch (
  sub_sClient		*cp
)
{

  gdb_AssumeLocked;

  if (cp->tmoactive) {
    pool_Qremove(NULL, gdbroot->pool, &cp->subt_ll);
    cp->tmoactive = FALSE;
    gdbroot->db->subt_lc--;
  }
}

/* Delete a subcli entry, and its associated data.
   It does not terminate any outstanding subscritions, though.
   The database must be locked.  */

static void
deleteClient (
  sub_sClient		*cp
)
{
  pwr_tStatus		sts;
  gdb_sNode		*np;

  gdb_AssumeLocked;

  cancelTimeoutWatch(cp);

  subc_RemoveFromMessage(cp);

  hash_Remove(&sts, gdbroot->subc_ht, cp);
  if (EVEN(sts)) errh_Bugcheck(sts, "remove client from hash table");

  np = hash_Search(&sts, gdbroot->nid_ht, &cp->nid);
  if (np == NULL) errh_Bugcheck(sts, "node not found");

  pool_Qremove(&sts, gdbroot->pool, &cp->subc_ll);
  np->subc_lc--;

  if (cp->userdata != pool_cNRef) {
    pool_FreeReference(NULL, gdbroot->rtdb, cp->userdata);
  }

  if (cp->cclass != pool_cNRef) {
    gdb_sCclass	*ccp;
    
    ccp = pool_Address(NULL, gdbroot->pool, cp->cclass);
    if (ccp == NULL) errh_Bugcheck(GDH__WEIRD, "cached class address");
    cmvolc_UnlockClass(NULL, ccp);
    cp->cclass = pool_cNRef;
  }

  pool_Free(NULL, gdbroot->pool, cp);

}

/* Test the existence of the object requested, and return node id.
   The sts field is filled in and for a subscription by name,
   the attrref field is filled in.
   The nid field is not affected!
   A nid of 0 implies that the object was not available.
   The database must be locked.  */

static gdb_sNode *
testClient (
  pwr_tStatus		*sts,
  sub_sClient		*cp
)
{
  pwr_tStatus		lsts = GDH__SUCCESS;
  pwr_tSubid		sid = cp->sid;
  cdh_sParseName	parseName;
  cdh_sParseName	*pn;
  gdb_sObject		*op = NULL;
  sub_sClient		*rcp;
  gdb_sVolume		*vp;
  gdb_sNode		*np = NULL;
  mvol_sAttribute	attribute;
  mvol_sAttribute	*ap;
  pwr_sAttrRef		*arp;
  pwr_sAttrRef		*rarp;
  gdb_sCclass		*ccp;
  gdb_sCclass		*ccpLocked;
  pool_tRef		ccr;
  pwr_tUInt32		ridx;
  pwr_tBoolean		equal;
  pwr_tBoolean		fetched;
  


  gdb_AssumeLocked;

  do {
    if (cp->sub_by_name) {	/* Lookup by name.  */

      pn = cdh_ParseName(&lsts, &parseName, pwr_cNObjid, cp->name, 0);
      if (pn == NULL) break;

      do {        
        ap = vol_NameToAttribute(&lsts, &attribute, pn, gdb_mLo_global, vol_mTrans_all);
        if (ap == NULL) break;
        rcp = hash_Search(sts, gdbroot->subc_ht, &sid);
        if (rcp != cp) break;	/* Subscription client no longer exists! */
        arp = mvol_AttributeToAref(&lsts, ap, &cp->aref);
        if (arp == NULL) break;
        op = ap->op;

        /* */
        vp = pool_Address(NULL, gdbroot->pool, op->l.vr);
        np = hash_Search(&lsts, gdbroot->nid_ht, &vp->g.nid);
        if (np == NULL) {
          op = NULL;
          break;
        }        
      
        ccp = NULL;
        equal = 1;
        
        /* Get cached class if needed */
        if (!op->u.c.flags.b.classChecked || !op->u.c.flags.b.classEqual) {
          ccp = cmvolc_GetCachedClass(&lsts, np, vp, ap, &equal, &fetched, NULL);
          if (EVEN(lsts)) {
            np = NULL;
            op = NULL;
            break;
          }

          if (fetched) {
            rcp = hash_Search(sts, gdbroot->subc_ht, &sid);
            if (rcp != cp) break;	/* Subscription client no longer exists! */
          }
          

          if (equal) {
            if (cp->cclass != pool_cNRef) {
              ccp = pool_Address(NULL, gdbroot->pool, cp->cclass);
              if (ccp == NULL) errh_Bugcheck(GDH__WEIRD, "cached class address");
              cmvolc_UnlockClass(NULL, ccp);
              cp->cclass = pool_cNRef;
            }          
            ccp = NULL;
          } else {
            ccr = pool_ItemReference(NULL, gdbroot->pool, ccp);
            if (ccr == pool_cNRef) errh_Bugcheck(GDH__WEIRD, "cache class address to reference");
          

            if (ccr != cp->cclass) {
              if (cp->cclass != pool_cNRef) {              
                gdb_sCclass *cc2p = pool_Address(NULL, gdbroot->pool, cp->cclass);
                cmvolc_UnlockClass(NULL, cc2p);
              }
            
              cmvolc_LockClass(NULL, ccp);
              cp->cclass = ccr;
            }
          }
        

          /* If gdb has been unlocked, refresh pointers */
          /** @todo Check if we can do it more efficient, eg. vol_ArefToAttribute */
          if (fetched) {
            np = NULL;
            op = NULL;
            continue;
          }
        }

        break;
      } while (1);      

    } else {			/* Lookup by attribute reference.  */
      do {
        
        op = vol_OidToObject(&lsts, cp->aref.Objid, gdb_mLo_global, vol_mTrans_all, cvol_eHint_none);
        if (op == NULL) {
          lsts = GDH__NOSUCHOBJ;
          break;
        }
        rcp = hash_Search(sts, gdbroot->subc_ht, &sid);
        if (rcp != cp) break;	/* Subscription client no longer exists! */

        if (op->g.flags.b.isAliasServer) cp->aref.Objid = op->g.oid;
        /* This is a not to ugly fix, but it should be removed, LW.
           It's done to make Leif-G�ran Hansson happier. I.e. it makes
           the linksup program work. */
        cp->aref.Objid = op->g.oid;

        vp = pool_Address(NULL, gdbroot->pool, op->l.vr);
        np = hash_Search(&lsts, gdbroot->nid_ht, &vp->g.nid);
        if (np == NULL) {
          op = NULL;
          break;
        }        
      
        ccp = NULL;
        equal = 1;
        
        /* Get cached class if needed */
        if (!op->u.c.flags.b.classChecked || !op->u.c.flags.b.classEqual) {
	  ap = vol_ArefToAttribute(&lsts, &attribute, &cp->aref, gdb_mLo_global, vol_mTrans_all);
	  if (ap == NULL) break;
          ccp = cmvolc_GetCachedClass(&lsts, np, vp, ap, &equal, &fetched, NULL);
          if (EVEN(lsts)) {
            np = NULL;
            op = NULL;
            break;
          }

          if (fetched) {
            rcp = hash_Search(sts, gdbroot->subc_ht, &sid);
            if (rcp != cp) break;	/* Subscription client no longer exists! */
          }
          

          if (equal) {
            if (cp->cclass != pool_cNRef) {
              ccp = pool_Address(NULL, gdbroot->pool, cp->cclass);
              if (ccp == NULL) errh_Bugcheck(GDH__WEIRD, "cached class address");
              cmvolc_UnlockClass(NULL, ccp);
              cp->cclass = pool_cNRef;
            }          
            ccp = NULL;
          } else {
            ccr = pool_ItemReference(NULL, gdbroot->pool, ccp);
            if (ccr == pool_cNRef) errh_Bugcheck(GDH__WEIRD, "cache class address to reference");
          

            if (ccr != cp->cclass) {
              if (cp->cclass != pool_cNRef) {              
                gdb_sCclass *cc2p = pool_Address(NULL, gdbroot->pool, cp->cclass);
                cmvolc_UnlockClass(NULL, cc2p);
              }
            
              cmvolc_LockClass(NULL, ccp);
              cp->cclass = ccr;
            }
          }
        

          /* If gdb has been unlocked, refresh pointers */
          /** @todo Check if we can do it more efficient, eg. vol_ArefToAttribute */
          if (fetched) {
            np = NULL;
            op = NULL;
            continue;
          }
        }

        break;
      } while (1);
      
    }
  } while (0);

  /* There is a risk that the client has disappeared after these
     calls (can result in a cache lookup). Verify that it still exists...  */

  rcp = hash_Search(sts, gdbroot->subc_ht, &sid);
  if (rcp == cp) {	/* OK, it exists! */
    cp->sts = lsts;
    if (op != NULL) {
      if (gdbroot->db->log.b.sub) errh_Info("Test client %s", op->g.f.name.orig);
      vp = pool_Address(NULL, gdbroot->pool, op->l.vr);
      np = pool_Address(NULL, gdbroot->pool, vp->l.nr);

      if (!equal) {
        ccpLocked = ccp;
        rarp = ndc_NarefToRaref(sts, ap, arp, ccp, &ridx, &cp->raref, &equal, NULL, ccpLocked, vp, np );        
        if (rarp == NULL || equal) {
	  if (ccp->flags.b.cacheLock)
            cmvolc_UnlockClass(NULL, ccp);
          cp->cclass = pool_cNRef;
          if (rarp == NULL)
            np = gdbroot->no_node;
        } else {
          if (!ccp->flags.b.rnConv) {
            ndc_sRemoteToNative	*tbl;
            gdb_sClass 	*c = hash_Search(sts, gdbroot->cid_ht, &ccp->key.cid);
            if (c == NULL) errh_Bugcheck(GDH__WEIRD, "can't get class");

            tbl = pool_Alloc(sts, gdbroot->pool, sizeof(*tbl) * c->acount);

            ndc_UpdateRemoteToNativeTable(sts, tbl, c->acount, c, ccp, np->nid);
            if (ODD(*sts)) {
              ccp->rnConv = pool_Reference(NULL, gdbroot->pool, tbl);
              ccp->flags.b.rnConv = 1;
            } else {
              pool_Free(NULL, gdbroot->pool, tbl);
              cmvolc_UnlockClass(NULL, ccp);
              cp->cclass = pool_cNRef;
              np = gdbroot->no_node;
            }            
          }
        }
      } /* not equal class versions */

    } else {
      np = gdbroot->no_node;
    }
  }

  return np;
}

/* This routine is applied to a list of non-activated
   clients. Each client is tested for object existence. If
   the object is reachable, the client is made active by
   moving it to the appropriate node-queue, and, for remote
   nodes, sending the SubAdd message and starting timeout watching.
   The database must be locked.  */

void
subc_ActivateList (
  pool_sQlink		*lh,
  pwr_tObjid		oid
)
{
  pwr_tStatus		sts;
  tree_sTable		*add;
#if 0
  tree_sTable		*remove;
  sRemove		*rep;
#endif
  net_sSubSpec		*specp;
  qcom_sQid		tgt;
  pool_sQlink		*my_lh;
  pwr_tUInt32		my_subc_lc;
  gdb_sNode		*np;
  gdb_sNode		*old_np;
  sub_sClient		*cp;
  pool_sQlink		*cl;
  sAdd			*aep;
  

  /* Test each client. If existing object, fill in nid field
     and move the client to the appropriate nodes's subc_lh list. Turn
     on timeouts. Put request in subadd message buffer.

     We must do it all in one pass, since a user can come in during
     a cache query for a later subcli and cancel the subscription
     for an earlier subcli.  */

  add = tree_CreateTable(&sts, sizeof(pwr_tNodeId), offsetof(sAdd, nid),
    sizeof(sAdd), 10, tree_Comp_nid);
#if 0
  remove = tree_CreateTable(&sts, sizeof(pwr_tNodeId), offsetof(sRemove, nid),
    sizeof(sRemove), 10, tree_eComp_nid);
#endif

  /* Move all objects to a new, temporary root */

  my_lh = pool_Qalloc(NULL, gdbroot->pool);
  my_subc_lc = 0;

  for (cl = pool_Qsucc(NULL, gdbroot->pool, lh); cl != lh;) {
    cp = pool_Qitem(cl, sub_sClient, subc_ll);
    cl = pool_Qsucc(NULL, gdbroot->pool, cl);
    
    if (cdh_ObjidIsNull(oid) || cdh_ObjidIsEqual(oid, cp->aref.Objid)) {
      pool_Qremove(NULL, gdbroot->pool, &cp->subc_ll);
      pool_QinsertPred(NULL, gdbroot->pool, &cp->subc_ll, my_lh);
      my_subc_lc++;
    }
  }

  /* Now start testing clients from 'my_lh', and move them to other lists
     Make sure the clients are still there after the test.  */

  for (cl = pool_Qsucc(NULL, gdbroot->pool, my_lh); cl != my_lh; ) {
    cp = pool_Qitem(cl, sub_sClient, subc_ll);
    cl = pool_Qsucc(NULL, gdbroot->pool, cl);
    
    np = testClient(&sts, cp);

    /* If an error is returned the client doesn't exist anymore. Some
       other user removed it while TestSubcli had the database unlocked.
       Just go on with the next client...  */

    if (np == NULL) continue;
   
    /* Move the client to the list for the node where the object resides.
       nid = pwr_cNNodeId is used for all objects we don't know.  */

    old_np = hash_Search(&sts, gdbroot->nid_ht, &cp->nid);
    if (old_np == NULL) errh_Bugcheck(GDH__WEIRD, "");

    pool_Qremove(NULL, gdbroot->pool, &cp->subc_ll);
    old_np->subc_lc--;
    pool_QinsertPred(NULL, gdbroot->pool, &cp->subc_ll, &np->subc_lh);
    np->subc_lc++;
    cp->nid = np->nid;

    /* If the object's nid changed, then take the appropriate actions.  */

    if (np != old_np) {
      if (np->nid != pwr_cNNodeId) {  /* Object is known.  */

        activateTimeoutWatch(cp);

	aep = tree_Insert(&sts, add, &np->nid);
	if (aep == NULL) errh_Bugcheck(GDH__WEIRD, "");

	if (aep->msg == NULL) {
	  aep->cnt = MIN(my_subc_lc, net_cSubMaxAdd);
	  aep->msg = malloc(sizeof(net_sSubAdd) + sizeof(net_sSubSpec) * aep->cnt);
	  aep->msg->count = 0;
	} /* If there was no message allocated */

	specp		    = &aep->msg->spec[aep->msg->count];
	specp->sid	    = cp->sid;
	specp->dt	    = cp->dt;
	specp->sub_by_name  = cp->sub_by_name;
	specp->aref	    = cp->cclass != pool_cNRef ? cp->raref : cp->aref;

	if (++aep->msg->count >= aep->cnt) {

	  /* The message buffer is full and must be sent.  */

	  tgt = np->handler;
	  pwr_Assert(tgt.nid != pwr_cNNodeId);

	  gdb_Unlock;

	    net_Put(NULL, &tgt, aep->msg, net_eMsg_subAdd, 0, pwr_Offset(aep->msg, spec[aep->msg->count]), 0);

	  gdb_Lock;

	  aep->msg->count = 0;

	}

      } else {	/* The object became unknown... */

        cancelTimeoutWatch(cp);
	subc_SetOld(cp);	/* Data gets old immediately */

#if 0
        /** @todo Maybe we should unlock the cached class? */
        if (cp->cclass != pwr_cNRefId) {
          ccp = pool_Address(NULL, gdbroot->pool, cp->cclass);
          if (ccp == NULL) errh_Bugcheck(GDH__WEIRD, "Cached class address");
          cmvolc_UnlockClass(NULL, ccp);
          cp->cclass = pwr_cNRefId;
        }
#endif        
          
#if 0
	if (old_np->nid != pwr_cNNodeId) {
	  rep = tree_Insert(&sts, remove, &old_np->nid);
	  if (rep == NULL) errh_Bugcheck(GDH__WEIRD, "");

	  if (rep->msg == NULL) {
	    rep->nid = old_np->nid;
	    rep->cnt = MIN(my_subc_lc, net_cSubMaxRemove);
	    rep->msg = malloc(sizeof(net_sSubRemove) + sizeof(rep->msg->sid[0]) * rep->cnt);
	    rep->msg->count = 0;
	  }

	  rep->msg->sid[rep->msg->count++] = cp->sid;

	  if (rep->msg->count >= net_cSubMaxRemove) {
	    np = hash_Search(&sts, gdbroot->nid_ht, &old_np->nid);
	    if (np == NULL) errh_Bugcheck(sts, "");
	    tgt.nid = rep->nid;
	    pwr_Assert(tgt.nid != pwr_cNNodeId);

	    gdb_Unlock;

	      net_Put(NULL, &tgt, rep->msg, net_eMsg_subRemove, 0, pwr_Offset(rep->msg, sid[rep->msg->count]), 0);

	    gdb_Lock;

	    rep->msg->count = 0;

	  }
	}
#endif
      }
    }
  }

  pool_Free(NULL, gdbroot->pool, my_lh);

  /* Now walk through the addmsg & remmsg and send all unsent messages.  */

  gdb_Unlock;

#if 0
    for (rep = tree_Minimum(remove); rep != NULL; rep = tree_Successor(remove, rep)) { 
      if (rep->msg != NULL) {
	if (rep->msg->count > 0) {
	  tgt.nid = rep->nid;
	  pwr_Assert(tgt.nid != pwr_cNNodeId);
	  net_Put(NULL, &tgt, rep->msg, net_eMsg_subRemove, 0, pwr_Offset(rep->msg, sid[rep->msg->count]), 0);
	}
	free(rep->msg);
      }
    }
#endif
    for (aep = tree_Minimum(&sts, add); aep != NULL; aep = tree_Successor(&sts, add, aep)) { 
      if (aep->msg != NULL) {
	if (aep->msg->count > 0) {

          np = hash_Search(&sts, gdbroot->nid_ht, &aep->nid);
          if (np == NULL) errh_Bugcheck(GDH__WEIRD, "");

	  tgt = np->handler;
	  pwr_Assert(tgt.nid != pwr_cNNodeId);
	  net_Put(NULL, &tgt, aep->msg, net_eMsg_subAdd, 0, pwr_Offset(aep->msg, spec[aep->msg->count]), 0);
	}
	free(aep->msg);
      }
    }

  gdb_Lock;

  tree_DeleteTable(&sts, add);
#if 0
  tree_DeleteTable(remove);
#endif
}

/* Cancels a list of clients.
   The client entry is released, and a message is sent to the node
   holding the server entry corresponding to the client.
   The database should be locked on entry.  */

void
subc_CancelList (
  pool_sQlink		*lh
)
{
  qcom_sQid		tgt;
#if 0
  tree_sTable		*remove;
  sRemove		*rep;
  gdb_sNode		*np;
#endif
  pwr_tNodeId		nid;
  pwr_tSubid		sid;
  sub_sClient		*cp;
  pool_sQlink		*cl;

  gdb_AssumeLocked;

  tgt.qix = net_cProcHandler;

#if 0
  remove = tree_CreateTable(sizeof(pwr_tNodeId), offsetof(sRemove, nid),
    sizeof(sRemove), 10, tree_eComp_nid, NULL);
#endif

  for (cl = pool_Qsucc(NULL, gdbroot->pool, lh); cl != lh;) {
    cp = pool_Qitem(cl, sub_sClient, subc_ll);
    cl = pool_Qsucc(NULL, gdbroot->pool, cl);

    cancelTimeoutWatch(cp);

    /* Get rid of the client, make it invalid for other references
       that might happen when we are unlocked
       But first: save nid and sid which we need further down.  */

    nid = cp->nid;
    sid = cp->sid;
    deleteClient(cp);

#if 0
    if (nid != pwr_cNNodeId) {
      rep = tree_Insert(remove, &nid);
      if (rep == NULL) errh_Bugcheck(GDH__WEIRD, "");

      if (rep->msg == NULL) {
	np = hash_Search(NULL, gdbroot->nid_ht, &nid);
	if (np == NULL) errh_Bugcheck(GDH__WEIRD, "");
	rep->cnt = net_cSubMaxRemove;
	rep->msg = malloc(sizeof(net_sSubRemove) + sizeof(rep->msg->sid[0]) * rep->cnt);
	rep->msg->count = 0;
      }

      rep->msg->sid[rep->msg->count++] = sid;

      if (rep->msg->count >= net_cSubMaxRemove) {
	tgt.nid = rep->nid;

	gdb_Unlock;

	  net_Put(NULL, &tgt, rep->msg, net_eMsg_subRemove, 0, pwr_Offset(rep->msg, sid[rep->msg->count]), 0);

	gdb_Lock;

	rep->msg->count = 0;

      }
    } /* Remote */
#endif
  } /* For all clients in list */

#if 0
  /* Now send all unsent messages.  */

  gdb_Unlock;

    for (rep = tree_Minimum(remove); rep != NULL; rep = tree_Successor(remove, rep)) { 
      if (rep->msg != NULL) {
	if (rep->msg->count > 0) {
	  tgt.nid = rep->nid;
	  net_Put(NULL, &tgt, rep->msg, net_eMsg_subRemove, 0, pwr_Offset(rep->msg, sid[rep->msg->count]), 0);
	}
	free(rep->msg);
      }
    }

  gdb_Lock;

  tree_DeleteTable(remove);
#endif
}

/* Cancel all subscriptions for a particular user.
   The database should be locked on entry.  */

void
subc_CancelUser (
  pid_t			subscriber
)
{
  sub_sClient		*cp;
  pool_sQlink		*cl;
  gdb_sNode		*np;
  pool_sQlink		*nl;
  pool_sQlink		*lh;

  /* Allocate a temporary root */

  gdb_AssumeLocked;

  lh = pool_Qalloc(NULL, gdbroot->pool);

  /* Build a list with all clients to cancel */

  for (
    nl = pool_Qsucc(NULL, gdbroot->pool, &gdbroot->db->nod_lh);
    nl != &gdbroot->db->nod_lh;
    nl = pool_Qsucc(NULL, gdbroot->pool, nl)
  ) {
    np = pool_Qitem(nl, gdb_sNode, nod_ll);

    for (cl = pool_Qsucc(NULL, gdbroot->pool, &np->subc_lh); cl != &np->subc_lh; ) {
      cp = pool_Qitem(cl, sub_sClient, subc_ll);
      cl = pool_Qsucc(NULL, gdbroot->pool, cl);

      if (cp->subscriber == subscriber) {
        pool_Qremove(NULL, gdbroot->pool, &cp->subc_ll);
        pool_QinsertPred(NULL, gdbroot->pool, &cp->subc_ll, lh);
      }
    } /* For all clients of that node. */
  } /* For all nodes */

  /* Now cancel them all! */

  subc_CancelList(lh);

  /* Return the root to the pool */

  pool_Free(NULL, gdbroot->pool, lh);

}

/* Allocate and fill in one client entry.
   Either name or attrref must be supplied.
   The subcli is linked into a list
   The database must be locked.  */

sub_sClient *
subc_Create (
  char			*name,			/* Input or NULL */
  pwr_sAttrRef		*arp,			/* Input or NULL */
  pool_sQlink		*lh			/* List header. */
)
{
  sub_sClient		*cp;
  sub_sClient		*rcp;
  pwr_tInt32		s;
  pwr_tStatus		sts;

  gdb_AssumeLocked;

  s = sizeof(*cp) + (name == NULL ? 0 : strlen (name));
  cp = pool_Alloc(NULL, gdbroot->pool, s);

  cp->sub_by_name = (name != NULL);
  if (cp->sub_by_name)
    strcpy(cp->name, name);
  else
    cp->aref = *arp;

  cp->subscriber  = gdbroot->my_pid;
  cp->dt	  = default_dt;
  cp->tmo	  = default_tmo;
  cp->old	  = TRUE;	/* No data yet */

  /* The client is always counted in a np->subc_lc quota
     and must always be in a pool_sQlink list.  */

  pool_QinsertPred(NULL, gdbroot->pool, &cp->subc_ll, lh);
  gdbroot->no_node->subc_lc++;

  do {
    gdbroot->db->subcid.rix++;
    cp->sid = gdbroot->db->subcid;

    rcp = hash_Insert(&sts, gdbroot->subc_ht, cp);
    if (rcp != cp && sts != HASH__DUPLICATE)
      errh_Bugcheck(sts, "hash_Insert");
  } while (rcp != cp);

  return cp;
}

/* Decrease the charging of a 'message'.
   If the 'message' is charged with 0 clients then it is disposed.

   The database should be locked.  */

void
subc_RemoveFromMessage (
  sub_sClient		*cp
)
{
  sub_sMessage		*mp;

  if (cp->submsg == pool_cNRef)
    return;

  gdb_AssumeLocked;

  mp = pool_Address(NULL, gdbroot->pool, cp->submsg);
  if (--mp->msg.count == 0) {
    pool_Qremove(NULL, gdbroot->pool, &mp->subm_ll);
    gdbroot->db->subm_lc--;

    pool_Free(NULL, gdbroot->pool, mp);

  } /* Last reference to that SUBMSG, dispose it */

  cp->submsg  = pool_cNRef;
  cp->subdata = pool_cNRef;
}

/* Changes the defaults for dt and tmo.  */

void
subc_SetDefaults (
  pwr_tInt32		dt,
  pwr_tInt32		tmo
)
{
  if (dt >= 0) {
    if (dt != 0)
      default_dt = dt;
    else
      default_dt = 1000;
  }
  if (tmo >= 0) {
    if (tmo != 0)
      default_tmo = tmo;
    else
      default_tmo = 10000;
  }
}

/* Set the data in a client as timed out.

   If the data of the subscription is to be blank when 
   it gets `old', then invoke subc_RemoveFromMessage here.
   Doing this also prevents the pool from beeing exhausted 
   since `old' data locks the 'message' in the pool.  */


void
subc_SetOld (
  sub_sClient		*cp
)
{
  void   *adrs;

  cp->old = TRUE;

  if (cp->userdata != pool_cNRef) {
    adrs = pool_Address(NULL, gdbroot->rtdb, cp->userdata);
    if (adrs != NULL) {
      memset(adrs, 0, cp->usersize);
    }
  }
  
  subc_RemoveFromMessage(cp);

}
