/* 
 * Proview   $Id: rt_qexp.c,v 1.2 2005-09-01 14:57:48 claes Exp $
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

/* rt_qexp.c -- Qcom exporter.
   Exports buffers destined to other nodes.  */

#include <stdlib.h>
#include "pwr.h"
#include "rt_errh.h"
#include "rt_pool.h"
#include "rt_qdb.h"
#include "rt_qnet.h"
#include "rt_qcom.h"

int
main (int argc, char *argv[])
{
  pwr_tStatus	sts;
  qdb_sBuffer	*bp;
  qdb_sBuffer	*nbp;
  qdb_sBuffer	*sbp;
  qdb_sPort	port;
  qdb_sPort	*pp;
  qdb_sQue	*qp;
  void		*p;
  pool_sQlink	*nl;
  qdb_sNode	*np;
  qdb_sRequest	*rp;

  errh_Init("pwr_qexp");

  if (!qcom_Init(&sts, NULL)) exit(sts);

  qdb_ScopeLock {

    qp = qdb_AttachQue(&sts, qdb->export, qdb->ap);

  } qdb_ScopeUnlock;

  if (qp == NULL) pwr_Bugcheck(sts, "attaching queue qdb_cQexport");

  port.sa.sin_family = AF_INET;  
  port.sa.sin_addr.s_addr = htonl(INADDR_ANY);
  port.sa.sin_port = 0;

  pp = qnet_Open(&sts, &port);

  while (qdb->g->up) {
    p = qcom_Get(&sts, &qdb->g->qid_export, NULL, qcom_cTmoEternal);
    if (p == NULL) continue;
    bp = (qdb_sBuffer *)p - 1; 
    pwr_Assert(bp->c.type == qdb_eBuffer_base);

    if (bp->c.flags.b.broadcast) {
      if (!bp->c.flags.b.imported) {
	qdb_ScopeLock {

	  for (
	    nl = pool_Qsucc(NULL, &qdb->pool, &qdb->g->node_lh);
	    nl != &qdb->g->node_lh;
	    nl = pool_Qsucc(NULL, &qdb->pool, nl)
	  ) {
	    np = pool_Qitem(nl, qdb_sNode, node_ll);
	    if (np == qdb->my_node || np == qdb->no_node || np->state != qdb_eState_up)
	      continue;

	    nbp = qdb_CopyBuffer(&sts, bp);
	    pool_Qremove(&sts, &qdb->pool, &nbp->c.ll);
	    pool_QinsertPred(&sts, &qdb->pool, &nbp->c.ll, &np->bcb_lh);
	    nbp->b.noderef = pool_Reference(&sts, &qdb->pool, np); 
	    nbp->b.info.receiver.nid = np->nid;      
	    nbp->b.info.rid = ++np->bc_snd_id;      
	    
	    if (pool_QhasOne(NULL, &qdb->pool, &np->bcb_lh)) {
	      sbp = qdb_CopyBuffer(&sts, nbp);

	      qdb_Unlock;
#if 0
		errh_Info("Sending bcast to que %s on %s",
		  qcom_QidToString(NULL, &sbp->b.info.receiver, 1), cdh_NodeIdToString(NULL, np->nid, 0, 0));
#endif
		qnet_Put(&sts, pp, sbp);
	      qdb_Lock;

	      qdb_Free(NULL, sbp);
	    }
	  }

	} qdb_ScopeUnlock;
      }
    } else {
      if (bp->c.flags.b.reply) {
	nbp = NULL;
	qdb_ScopeLock {
	  rp = hash_Search(&sts, &qdb->req_ht, &bp->b.info.receiver);
	  if (rp == NULL)
	    break;
	  if (rp->rid != bp->b.info.rid)
	    break; /* here we could choose not to send the buffer */
	  if (pool_QisEmpty(&sts, &qdb->pool, &rp->b_lh)) {
	    nbp = qdb_CopyBuffer(&sts, bp);
	    if (nbp == NULL)
	      break;
	    pool_Qremove(&sts, &qdb->pool, &nbp->c.ll);
	    pool_QinsertPred(&sts, &qdb->pool, &nbp->c.ll, &rp->b_lh);
	  }
	} qdb_ScopeUnlock;
#if 0
	if (0)
	  errh_Info("reply: %d, nbp: %d, status\n%m", bp->b.info.rid, nbp, bp->b.info.status);
#endif
	if (rp == NULL || rp->rid == bp->b.info.rid)	
	  qnet_Put(&sts, pp, bp);

      } else {
#if 0
	if (0 && bp->c.flags.b.request) {
	  errh_Info("request: %d, tmo: %d", bp->b.info.rid, bp->b.info.tmo);
	}
#endif
	qnet_Put(&sts, pp, bp);
      }
    }
    qcom_Free(&sts, p);
  }

  qnet_Close(&sts, pp);
  qcom_Exit(&sts);

  exit(QCOM__SUCCESS);
}
