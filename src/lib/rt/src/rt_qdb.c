/* 
 * Proview   $Id: rt_qdb.c,v 1.13 2008-11-24 15:20:06 claes Exp $
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

/* rt_qdb.c -- Queue Communication Database */

#if defined(OS_ELN)
# include $vaxelnc
# include stdarg
# include stsdef
# include descrip
#else 
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#endif

#if defined OS_LYNX || defined OS_LINUX
# include <signal.h>
#endif

#if defined (OS_LYNX)
# include <sys/mman.h>
#elif	defined(OS_LINUX)
# include <sys/file.h>
# include <sys/stat.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include "rt_semaphore.h"
#endif


#include <errno.h>
#include "pwr.h"
#include "co_time.h"
#include "rt_qdb_msg.h"
#include "rt_qcom_msg.h"
#include "rt_errh.h"
#include "rt_qdb.h"
#include "rt_qos.h"
#include "rt_pool.h"
#include "rt_hash.h"

#if defined (OS_ELN)
#if 0
  noshare qdb_sLocal *qdb = NULL;
#endif
#else
  qdb_sLocal *qdb = NULL;
#endif

static int
copyBufferData (
  void		*itp,
  qdb_sBuffer	*bp,
  int		offs,
  int		size
);


/* Copy the contents of a buffer.
   Return number of bytes not copied. */

static int
copyBufferData (
  void		*itp,
  qdb_sBuffer	*bp,
  int		offs,
  int		size
)
{
  char		*fp;
  int		len;
  qdb_sBuffer	*sp;
  pool_sQlink	*sl;
  char		*tp = (char *)itp;

  pwr_Assert(bp != NULL);
  pwr_Assert(bp->c.type == qdb_eBuffer_base);

  if (offs < bp->b.seg_size) {
    len = MIN(bp->b.seg_size - offs, size);
    fp = (char *)(bp + 1);
    fp +=offs;
    memcpy(tp, fp, len);
    tp += len;
    size -= len;
    offs = 0;
  } else {
    offs -= bp->b.size;
  }

  for ( /* copy all segments, if segmented */
    sl = pool_Qsucc(NULL, &qdb->pool, &bp->b.seg_lh);
    sl != &bp->b.seg_lh && size > 0;
    sl = pool_Qsucc(NULL, &qdb->pool, sl)
  ) {
    sp = pool_Qitem(sl, qdb_sBuffer, s.seg_ll);
    if (offs < sp->s.size) {
      len = MIN(sp->s.size - offs, size);
      fp = (char *)(sp + 1);
      fp +=offs;
      memcpy(tp, fp, len);
      tp += len;
      size -= len;
      offs = 0;
    } else {
      offs -= sp->s.size;
    } 
  }

  return size;
}

static qdb_sInit *
evaluateInit (
  qdb_sInit		*ip
)
{

  if (ip == NULL) ip = calloc(1, sizeof(*ip));

  ip->nodes	    = MAX(ip->nodes,	    qdb_cMin_nodes);
  ip->queues	    = MAX(ip->queues,	    qdb_cMin_queues);
  ip->appls	    = MAX(ip->appls,	    qdb_cMin_appls);
  ip->sbufs	    = MAX(ip->sbufs,	    qdb_cMin_sbufs);
  ip->size_sbuf	    = MAX(ip->size_sbuf,    qdb_cSize_sbuf);
  ip->mbufs	    = MAX(ip->mbufs,	    qdb_cMin_mbufs);
  ip->size_mbuf	    = MAX(ip->size_mbuf,    qdb_cSize_mbuf);
  ip->lbufs	    = MAX(ip->lbufs,	    qdb_cMin_lbufs);
  ip->size_lbuf	    = MAX(ip->size_lbuf,    qdb_cSize_lbuf);

  /* The pool contains nodes, queues, bonds, applications, buffers and
     other qdb internal structures. Make it at least 600k to begin with,
     or 1.5 times the size of the object headers we are initially populating
     the database with.
     The pool will extend in segments of 1/2 of the initial segment,
     i.e. no segment will be smaller than 300k bytes. There is a limit
     on the number of pool segments of 256, so the worst case pool
     (a minimal system that need to extend the pool heavily) is
     600k + 255*300k = 146.5Mb.  */

#if 0
  errh_Info("Nodes      : %d : %d bytes", ip->nodes, ip->nodes * sizeof(qdb_sNode));
  errh_Info("Queues     : %d : %d bytes", ip->queues, ip->queues * sizeof(qdb_sQue));
  errh_Info("Appls      : %d : %d bytes", ip->appls, ip->appls * sizeof(qdb_sAppl));
  errh_Info("Small buff : %d : %d bytes", ip->sbufs, ip->sbufs * ip->size_sbuf);
  errh_Info("Medium buff: %d : %d bytes", ip->mbufs, ip->mbufs * ip->size_mbuf);
  errh_Info("Large buff : %d : %d bytes", ip->lbufs, ip->lbufs * ip->size_lbuf);
#endif

  ip->pool_isize =
    ip->nodes	* sizeof(qdb_sNode) +
    ip->queues	* sizeof(qdb_sQue) +
    ip->appls	* sizeof(qdb_sAppl);
#if 0
    ip->sbufs	* ip->size_sbuf +
    ip->mbufs	* ip->size_mbuf +
    ip->lbufs	* ip->size_lbuf;
#endif

  ip->pool_isize = MAX(ip->pool_isize, qdb_cMin_pool_isize);
  ip->pool_esize = ip->pool_isize / 2;

  errh_Info("Qcom pool isize : %d, esize: %d", ip->pool_isize, ip->pool_esize);

  return ip;
}

static qdb_sLocal *
mapLocalDb (
  pwr_tStatus		*sts
)
{
  hash_sTable		*ht;

  qdb_AssumeLocked;

  if (qdb->g->version != qdb_cVersion)
    pwr_Return(NULL, sts, QCOM__REVLEVEL);

  /* Map hash tables.  */

  qdb->my_aix = ++qdb->g->aid.aix;
  qdb->my_aid.aix = qdb->my_aix;
  qdb->my_aid.nid = qdb->g->nid;
  

  ht = hash_Create(sts, &qdb->pool, &qdb->nid_ht, &qdb->g->nid_ht, NULL, NULL);
  if (ht == NULL) errh_Bugcheck(*sts, "initiating nid hash table");

  ht = hash_Create(sts, &qdb->pool, &qdb->qix_ht, &qdb->g->qix_ht, NULL, NULL);
  if (ht == NULL) errh_Bugcheck(*sts, "initiating qix hash table");

  ht = hash_Create(sts, &qdb->pool, &qdb->aix_ht, &qdb->g->aix_ht, NULL, NULL);
  if (ht == NULL) errh_Bugcheck(*sts, "initiating aix hash table");

  ht = hash_Create(sts, &qdb->pool, &qdb->pid_ht, &qdb->g->pid_ht, NULL, NULL);
  if (ht == NULL) errh_Bugcheck(*sts, "initiating pid hash table");

  return qdb;  
}

qdb_sAppl *
qdb_AddAppl (
  pwr_tStatus	*status,
  pwr_tBoolean	system
)
{
  qdb_sAppl	*ap;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  if (!system) {
    ap = hash_Search(sts, &qdb->aix_ht, &qdb->my_aix);
    if (ap != NULL) pwr_Return(NULL, sts, QDB__DUPLADD);
  }

  ap = pool_Alloc(sts, &qdb->pool, sizeof(*ap));
  if (ap == NULL) return NULL;

  pool_Qinit(NULL, &qdb->pool, &ap->aix_htl);
  pool_Qinit(NULL, &qdb->pool, &ap->appl_ll);
  pool_Qinit(NULL, &qdb->pool, &ap->out_lh);
  pool_Qinit(NULL, &qdb->pool, &ap->que_lh);

  if (!system) {
    ap->aid = qdb->my_aid;
    ap->pid = qdb->my_pid;

    ap = hash_Insert(sts, &qdb->aix_ht, ap);
    if (ap == NULL) errh_Bugcheck(QCOM__WEIRD, "adding new application identifier");

    ap = hash_Insert(sts, &qdb->pid_ht, ap);
    if (ap == NULL) errh_Bugcheck(QCOM__WEIRD, "adding new application identifier");
  } else {
    strcpy(ap->name, "pwr_system");
  }

  pool_QinsertPred(NULL, &qdb->pool, &ap->appl_ll, &qdb->g->appl_lh);

  return ap;
}

qdb_sNode *
qdb_AddNode (
  pwr_tStatus		*status,
  pwr_tUInt32		nid,  
  pwr_tBitMask		flags
)
{
  qdb_sNode		*np;
  pwr_dStatus		(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  np = hash_Search(sts, &qdb->nid_ht, &nid);
  if (np != NULL) {
    if (flags & qdb_mAdd_failIfAdded)
      pwr_Return(NULL, sts, QDB__DUPLADD);
    else
      pwr_Return(np, sts, QDB__ALRADD);
  }

  np = pool_Alloc(sts, &qdb->pool, sizeof(*np));
  if (np == NULL) return NULL;

  pool_Qinit(NULL, &qdb->pool, &np->nid_htl);
  pool_Qinit(NULL, &qdb->pool, &np->node_ll);
  pool_Qinit(NULL, &qdb->pool, &np->own_lh);
  pool_Qinit(NULL, &qdb->pool, &np->bcb_lh);

  np->nid = nid;
  np->get.timer_max = 30;
  np->put.timer_max = 10;

  np = hash_Insert(sts, &qdb->nid_ht, np);
  if (np == NULL) errh_Bugcheck(QCOM__WEIRD, "adding new node identifier");

  pool_QinsertPred(NULL, &qdb->pool, &np->node_ll, &qdb->g->node_lh);

  return np;
}

/* Bind a forwarding (source) queue to a
   receiving (target) queue */

qdb_sQbond *
qdb_AddBond (
  pwr_tStatus	*status,
  qdb_sQue	*sq,
  qdb_sQue	*tq
)
{
  qdb_sQbond	*bp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);
  
  qdb_AssumeLocked;

  bp = pool_Alloc(sts, &qdb->pool, sizeof(qdb_sQbond));
  if (bp == NULL) return NULL;

  bp->srcQix = sq->qix;
  bp->srcQ   = pool_Reference(sts, &qdb->pool, sq);
  pool_QinsertPred(NULL, &qdb->pool, &bp->tgt_ll, &sq->tgt_lh);

  bp->tgtQix = tq->qix;
  bp->tgtQ   = pool_Reference(sts, &qdb->pool, tq);
  pool_QinsertPred(NULL, &qdb->pool, &bp->src_ll, &tq->src_lh);

  return bp;
}

void
qdb_ApplEvent (
  pwr_tStatus		*status,
  qdb_sAppl		*ap,
  qcom_eStype		event
)
{
  qcom_sAppl		*appl;
  qdb_sQue		*qp;
  qdb_sBuffer		*bp;
  pwr_dStatus(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  qp = qdb_Que(sts, &qcom_cQnetEvent, NULL);
  bp = qdb_Alloc(sts, qdb_eBuffer_base, sizeof(qcom_sAppl));
  appl = (qcom_sAppl *)(bp + 1);

  bp->b.info.sender	  = qdb->my_aid;
  bp->b.info.receiver.qix = qp->qix;
  bp->b.info.receiver.nid = qdb->g->nid;
  bp->b.info.type.b	  = qcom_eBtype_qcom;
  bp->b.info.type.s	  = event;
  bp->b.info.size	  = sizeof(qcom_sAppl);

  appl->aid = ap->aid;
  appl->pid = ap->pid; 

  qdb_Put(sts, bp, qp);

}

/* Dump information of pool. */

void
qdb_DumpPool ()
{
  pwr_tStatus	sts;

  qdb_AssumeLocked;

  pool_Dump(&sts, &qdb->pool);
}

/* Check that a queue is bound to a forwarding queue.  */

qdb_sQbond *
qdb_GetBond (
  pwr_tStatus	*sts,
  qdb_sQue	*sq,
  qdb_sQue	*tq
)
{
  pool_sQlink	*bol;
  qdb_sQbond	*bop;

  qdb_AssumeLocked;

  for (
    bol = pool_Qsucc(NULL, &qdb->pool, &sq->tgt_lh);
    bol != &sq->tgt_lh;
    bol = pool_Qsucc(NULL, &qdb->pool, bol)
  ) {
    bop = pool_Qitem(bol, qdb_sQbond, tgt_ll);
    if (bop->tgtQix == tq->qix)
      return bop;
  }

  return NULL;
}

/* Allocate a buffer from the pool.  */

qdb_sBuffer *
qdb_Alloc (
  pwr_tStatus	*status,
  qdb_eBuffer	btype,
  unsigned int	size
)
{
  qdb_sBuffer	*bp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;
  
  bp = pool_Alloc(sts, &qdb->pool, sizeof(*bp) + size);
  if (bp == NULL) return NULL;

  bp->c.type = btype;

  switch (btype) {
  case qdb_eBuffer_segment:
    bp->s.size = size;
    pool_QinsertPred(sts, &qdb->pool, &bp->c.ll, &qdb->ap->out_lh);
    pool_Qinit(NULL, &qdb->pool, &bp->s.seg_ll);
    break;
  case qdb_eBuffer_base:
    bp->b.size	    = size;  
    bp->b.seg_size  = size;  
    bp->b.cookie    = qdb_cCookie;
    pool_QinsertPred(sts, &qdb->pool, &bp->c.ll, &qdb->ap->out_lh);
    pool_Qinit(NULL, &qdb->pool, &bp->b.seg_lh);
    pool_Qinit(NULL, &qdb->pool, &bp->b.ref_lh);
    break;
  case qdb_eBuffer_reference:
    pool_QinsertPred(sts, &qdb->pool, &bp->c.ll, &qdb->ap->out_lh);
    pool_Qinit(NULL, &qdb->pool, &bp->r.ref_ll);
    break;
  default:
    bp = NULL;
    break;
  }

  return bp;
}

/* Copy the contents of a buffer.
   Return number of bytes not copied. */

void *
qdb_CopyBufferData (
  pwr_tStatus	*status,
  qdb_sBuffer	*bp,
  qcom_sGet	*gp
)
{
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  if (bp->c.type != qdb_eBuffer_base)
    errh_Bugcheck(QCOM__WEIRD, "wrong buffer type");

  if (gp->size < bp->b.info.size) *sts = QCOM__BUFOVRUN;
  
  gp->size = MIN(gp->maxSize, bp->b.info.size);
  copyBufferData(gp->data, bp, 0, gp->size);

  return gp->data;
}

qdb_sBuffer *
qdb_Deque (
  pwr_tStatus	*status,
  qdb_sQue	*qp,
  int		tmo
)
{
  pool_sQlink	*bl;
  pool_sQlink	*q_head = &qp->in_lh;
  qdb_sBuffer	*bp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  if (qp->flags.b.reply) {
    q_head = &qp->rep_lh;
    /* remove all buffers not matching current rid */
    for (
      bl = pool_Qsucc(NULL, &qdb->pool, q_head);
      bl != q_head;
    ) {
      bp = pool_Qitem(bl, qdb_sBuffer, c.ll);
      bl = pool_Qsucc(NULL, &qdb->pool, bl);
      if (bp->b.info.rid != qp->rid) {
	qdb_Free(sts, bp);
        qp->in_lc--;
      }
    }
  }

  if (pool_QisEmpty(sts, &qdb->pool, q_head)) {
    if (tmo == qcom_cTmoNone) {
      *sts = QCOM__QEMPTY;
      return NULL;
    } else if (!qos_WaitQue(sts, qp, tmo)) {
      return NULL;
    }
  }

  bl = pool_QremoveSucc(sts, &qdb->pool, q_head);
  if (bl == NULL) pwr_Return(NULL, sts, QCOM__QEMPTY);
  qp->in_lc--;
  pool_QinsertPred(sts, &qdb->pool, bl, &qp->read_lh);

  return pool_Qitem(bl, qdb_sBuffer, c.ll);
}

pwr_tBoolean
qdb_Enque (
  pwr_tStatus	*status,
  qdb_sBuffer	*bp,
  qdb_sQue	*qp
)
{
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  if ( qp->in_quota && qp->in_lc >= qp->in_quota) {
    *status = QDB__QUOTAEXCEEDED;
    return 0;
  }
    
  pool_Qremove(sts, &qdb->pool, &bp->c.ll);
  if (!bp->c.flags.b.remote && bp->c.flags.b.reply) {
    if (qp->flags.b.reply && bp->b.info.rid == qp->rid) {
      pool_QinsertPred(sts, &qdb->pool, &bp->c.ll, &qp->rep_lh);
      qp->in_lc++;
      qos_SignalQue(sts, qp);
    } else {
      qdb_Free(NULL, bp);
    }
  } else {
    pool_QinsertPred(sts, &qdb->pool, &bp->c.ll, &qp->in_lh);
    qp->in_lc++;
    qos_SignalQue(sts, qp);
  }

  return ODD(*sts);
}

/* Free a buffer allocated from the pool.

   If the buffer is a forwarding buffer then the
   original buffer reference count is decremented.
   If the reference count reaches zero then the original
   buffer is also freed.  */

void
qdb_Free (
  pwr_tStatus	*status,
  qdb_sBuffer	*bp
)
{
  qdb_sBuffer	*bbp;
  qdb_sBuffer	*sbp;
  pool_sQlink	*sl;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  switch (bp->c.type) {
  case qdb_eBuffer_base:
    if (pool_QisLinked(sts, &qdb->pool, &bp->c.ll))
      pool_Qremove(sts, &qdb->pool, &bp->c.ll);
    if (pool_QisLinked(sts, &qdb->pool, &bp->b.ref_lh))
      break; /* buffer is still referenced by other buffers */
    for ( /* remove all segments, if segmented */
      sl = pool_Qsucc(NULL, &qdb->pool, &bp->b.seg_lh);
      sl != &bp->b.seg_lh;
    ) {
      sbp = pool_Qitem(sl, qdb_sBuffer, s.seg_ll);
      sl = pool_Qsucc(NULL, &qdb->pool, sl);
      pool_Qremove(sts, &qdb->pool, &sbp->s.seg_ll);
      qdb_Free(sts, sbp);
    }
    pool_Free(sts, &qdb->pool, bp);
    break;
  case qdb_eBuffer_reference:
    if (pool_QisLinked(sts, &qdb->pool, &bp->c.ll))
      pool_Qremove(sts, &qdb->pool, &bp->c.ll);
    if (pool_QisLinked(sts, &qdb->pool, &bp->r.ref_ll))
      pool_Qremove(sts, &qdb->pool, &bp->r.ref_ll);
    bbp = pool_Address(sts, &qdb->pool, bp->r.src);
    if (!pool_QisLinked(sts, &qdb->pool, &bbp->b.ref_lh) &&
	!pool_QisLinked(sts, &qdb->pool, &bbp->c.ll)
    ) {
      qdb_Free(sts, bbp);
    }
    pool_Free(sts, &qdb->pool, bp);
    break;
  case qdb_eBuffer_segment:
    if (pool_QisLinked(sts, &qdb->pool, &bp->c.ll))
      pool_Qremove(sts, &qdb->pool, &bp->c.ll);
    if (pool_QisLinked(sts, &qdb->pool, &bp->s.seg_ll))
      pool_Qremove(sts, &qdb->pool, &bp->s.seg_ll);
    pool_Free(sts, &qdb->pool, bp);
    break;
  default:
    break;
  }
  
}

/* .  */

void
qdb_GetInfo (
  qcom_sGet	*gp,
  qdb_sBuffer	*bp
)
{
  qdb_sInfo	*ip = &bp->b.info;

  if (bp->c.type != qdb_eBuffer_base)
    errh_Bugcheck(QCOM__WEIRD, "qdb_GetInfo: wrong buffer type");

  gp->sender	= ip->sender;
  gp->pid	= ip->pid;
  gp->receiver	= ip->receiver;
  gp->reply	= ip->reply;
  gp->type	= ip->type;
  gp->rid	= ip->rid;
  gp->size	= ip->size;
}

/* This routine maps the QCOM database.
   It should only be called by the init program.  */

qdb_sLocal *
qdb_CreateDb (
  pwr_tStatus		*status,
  qdb_sInit		*ip
)
{
  pwr_tBoolean		created;
  qdb_sNode		*np;
  qdb_sLocal		*lp;
  cdh_uRefId		qid;
  sect_sHead		*sp;
  pool_sHead		*pp;
  pwr_tTime		time;
  pwr_dStatus		(sts, status, QCOM__SUCCESS);

  if (qdb != NULL)
    pwr_Return(qdb, sts, QDB__ALRCRE);
 
  /* Create job-local structure.  */
  qdb = calloc(1, sizeof(*qdb));
  if (qdb == NULL) errh_Bugcheck(QDB__INSVIRMEM, "initiating local database");

  qdb->my_pid = getpid();

  /* Create lock section.  */

  sp = sect_Alloc(sts, &created, &qdb->lock, sizeof(sect_sMutex), qdb_cNameDbLock);
  if (sp == NULL) errh_Bugcheck(*sts, "creating db lock");
  if (!created) errh_Bugcheck(QCOM__WEIRD, "db lock was allready created");

  /* Create pools.  */

  ip = evaluateInit(ip);

  pp = pool_Create(sts, &qdb->pool, qdb_cNamePool, ip->pool_isize, ip->pool_esize);
  if (sp == NULL) errh_Bugcheck(*sts, "initating pool");

  qdb->g = pool_AllocNamedSegment(sts, &qdb->pool, sizeof(*qdb->g), qdb_cNameDatabase);
  if (qdb->g == NULL) errh_Bugcheck(*sts, "database directory");

  qdb->g->version = qdb_cVersion;

  qdb->g->eval_init = *ip;
  qdb->g->nid = ip->nid;

  qdb->g->qid_import.nid = qdb->g->nid;
  qdb->g->qid_import.qix = qdb_cIimport;
  qdb->g->qid_export.nid = qdb->g->nid;
  qdb->g->qid_export.qix = qdb_cIexport;

  sect_InitLock(sts, &qdb->lock, &qdb->g->lock);
  *sts = sync_MutexInit(&qdb->thread_lock.mutex);
  if (EVEN(*sts)) return NULL;
  *sts = sync_CondInit(&qdb->thread_lock.cond);
  if (EVEN(*sts)) return NULL;

#if 0
  pool_AllocLookasideSegment(sts, &qdb->pool, ip->objects, sizeof(qdb_sObject));
#endif

  qdb_ScopeLock {

    /* Initiate all list headers.  */

    pool_Qinit(NULL, &qdb->pool, &qdb->g->node_lh);
    pool_Qinit(NULL, &qdb->pool, &qdb->g->appl_lh);

    /* Set up our node identity.  */

    qdb->g->nid = ip->nid;
    qdb->my_nid = qdb->g->nid;

    /* Initiate next free queue identity.  */

    qid.pwr.nid = qdb->g->nid;
    qid.r.vid_3 = cdh_eVid3_qid;

    qdb->g->qid.nid = qid.pwr.nid;
    qdb->g->qid.qix = 1;

    /* Hash tables.  */

    hash_Init(&qdb->g->qix_ht, ip->queues, sizeof(qcom_tQix), sizeof(qdb_sQue),
      offsetof(qdb_sQue, qix), offsetof(qdb_sQue, qix_htl), hash_eKey_qix);

    hash_Init(&qdb->g->nid_ht, ip->nodes, sizeof(pwr_tNodeId), sizeof(qdb_sNode),
      offsetof(qdb_sNode, nid), offsetof(qdb_sNode, nid_htl), hash_eKey_nid);

    hash_Init(&qdb->g->pid_ht, ip->appls, sizeof(pid_t), sizeof(qdb_sAppl),
      offsetof(qdb_sAppl, pid), offsetof(qdb_sAppl, pid_htl), hash_eKey_pid);

    hash_Init(&qdb->g->aix_ht, ip->appls, sizeof(qcom_tAix), sizeof(qdb_sAppl),
      offsetof(qdb_sAppl, aid.aix), offsetof(qdb_sAppl, aix_htl), hash_eKey_pid);

    lp = mapLocalDb(sts);
    if (lp == NULL) break;

    if (ip->bus == 0)
      qdb->g->bus = 1;
    else
      qdb->g->bus = ip->bus;

    np = qdb->my_node = qdb_AddNode(sts, qdb->my_nid, qdb_mAdd_failIfAdded);
    if (qdb->my_node == NULL) errh_Bugcheck(*sts, "creating local node");
    qdb_Platform(np);

#if 0
    gethostname(np->name, sizeof(np->name));
#endif

    np->sa.sin_family = AF_INET;  
    np->sa.sin_addr.s_addr = htonl(INADDR_ANY);
    np->sa.sin_port = htons(55000 + qdb->g->bus);  
    clock_gettime(CLOCK_REALTIME, &time);
    np->birth = time.tv_sec;

    qdb->no_node = qdb_AddNode(sts, 0, qdb_mAdd_failIfAdded);
    if (qdb->no_node == NULL) errh_Bugcheck(*sts, "creating the unknown node");
    strcpy(qdb->no_node->name, "*** I am the unknown node ***");

    qdb->g->up = TRUE;
    qdb->g->tmo_export = 10000;
    qdb->g->qmon_sleep = 1;
    
  } qdb_ScopeUnlock;

  if (EVEN(*sts) || lp == NULL)
    qdb = NULL;

  return qdb;
}

#if defined OS_LYNX || defined OS_LINUX
/*
 * A fix which unlinks all segments for the given name.
 */
static void
unlinkPool(
  const char *name
){
  int 	 i;
  int    fd;                                              
  int    flags = O_RDWR;              
  mode_t mode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP; 
  key_t  key;
  int    shm_id;
  struct shmid_ds   ds;

  char   *str = getenv(pwr_dEnvBusId);
  char   segname[128];
  char	 busid[8];

  strncpy(busid, (str ? str : "XXX"), 3);
  busid[3] = '\0';

  sprintf(segname, "%s_%.3s", name, busid);

#if defined OS_LYNX
  fd = shm_open(segname, flags, mode); 
#else
  fd = open(segname, flags, mode); 
#endif
  if (fd != -1) { 
    close(fd);

#if defined OS_LYNX
      shm_unlink(segname);
#else
      key    = ftok(segname, 'P');
      shm_id = shmget(key, 0, 0660);
      shmctl(shm_id, IPC_RMID, &ds);
      unlink(segname);
#endif

    for (i = 1; TRUE; i++) {
      sprintf(segname, "%.11s%04d_%.3s", name, i, busid);
#if defined OS_LYNX
      fd = shm_open(segname, flags, mode); 
#else
      fd = open(segname, flags, mode); 
#endif

      if (fd == -1)
        break;

      close(fd);

#if defined OS_LYNX
      shm_unlink(segname);
#else
      key    = ftok(segname, 'P');
      shm_id = shmget(key, 0, 0660);
      shmctl(shm_id, IPC_RMID, &ds);
      unlink(segname);
#endif

    }
  }
}

#endif

/* This routine unlinks QCOM database and  
   removes database lock.
   It should only be called by the init program.  */

void
qdb_UnlinkDb ()
{
  char   segname[128];
  char	 busid[8];
  char   *str = getenv(pwr_dEnvBusId);
  key_t  key;
  int    shm_id;
  struct shmid_ds   ds;

#if defined OS_LYNX || defined OS_LINUX

  /* Unlink pool. */

  unlinkPool(qdb_cNamePool);

  /* Remove database lock. */

  strncpy(busid, (str ? str : "XXX"), 3);
  busid[3] = '\0';

  sprintf(segname, "%s_%.3s", qdb_cNameDbLock, busid);

#if defined OS_LYNX
      shm_unlink(segname);
#else
      key    = ftok(segname, 'P');
      shm_id = shmget(key, 0, 0660);
      shmctl(shm_id, IPC_RMID, &ds);
      posix_sem_unlink(segname);
      unlink(segname);
#endif

#endif
}

qdb_sLocal *
qdb_MapDb (
  pwr_tStatus		*status
)
{
  pwr_tBoolean		created;
  qdb_sLocal		*lp;
  pwr_tNodeId		no_nid = pwr_cNNodeId;
  sect_sHead		*sp;
  pool_sHead		*pp;
  pwr_dStatus		(sts, status, QCOM__SUCCESS);

  if (qdb != NULL)
    pwr_Return(qdb, sts, QDB__ALRMAP);
 
  qdb = calloc(1, sizeof(*qdb));
  if (qdb == NULL) errh_Bugcheck(QDB__INSVIRMEM, "initiating local database");

  qdb->my_pid = getpid();

  /* Map lock sections.  */

  sp = sect_Alloc(sts, &created, &qdb->lock, sizeof(sect_sMutex), qdb_cNameDbLock);
  if (sp == NULL) errh_Bugcheck(*sts, "mapping db lock");
  if (created) errh_Bugcheck(QCOM__WEIRD, "db lock was not created");
  *sts = sync_MutexInit(&qdb->thread_lock.mutex);
  if (EVEN(*sts)) return NULL;
  *sts = sync_CondInit(&qdb->thread_lock.cond);
  if (EVEN(*sts)) return NULL;

  /* Map 'pool' & 'rtdb' pools.  */

  pp = pool_Create(sts, &qdb->pool, qdb_cNamePool, 0, 0);
  if (pp == NULL) errh_Bugcheck(*sts, "initating pool");

  qdb->g = pool_AllocNamedSegment(sts, &qdb->pool, sizeof(*qdb->g), qdb_cNameDatabase);
  if (qdb->g == NULL) errh_Bugcheck(*sts, "database directory");

  qdb_ScopeLock {

    lp = mapLocalDb(sts);
    if (lp == NULL) return NULL;

    qdb->my_node = hash_Search(sts, &qdb->nid_ht, &qdb->g->nid);
    qdb->no_node = hash_Search(sts, &qdb->nid_ht, &no_nid);
    qdb->my_nid  = qdb->g->nid;
    qdb->exportque  = hash_Search(sts, &qdb->qix_ht, &qdb->g->qid_export.qix);
  } qdb_ScopeUnlock;

  return qdb;
}

qdb_sBuffer *
qdb_Get (
  pwr_tStatus	*status,
  qdb_sQue	*qp,
  int		tmo,
  qcom_sGet	*gp,
  pwr_tBitMask  flags
)
{
  pwr_tStatus	csts;
  qdb_sBuffer	*bp;
  qdb_sBuffer	*sbp;		/* pointer to source buffer */
  qdb_sBuffer	*nbp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);
  qdb_mGet      lflags;

  lflags.m = flags;

  qdb_AssumeLocked;

  if (!lflags.b.multipleGet && !pool_QisEmpty(sts, &qdb->pool, &qp->read_lh))
    pwr_Return(NULL, sts, QCOM__ALLOCQUOTA);

  bp = qdb_Deque(sts, qp, tmo);
  if (bp == NULL) return NULL;

  if (bp->c.type == qdb_eBuffer_reference
    && pool_QhasOne(sts, &qdb->pool, &bp->r.ref_ll)
  ) {
    /* we are the last reader, move base buffer to this que */

    sbp = pool_Address(sts, &qdb->pool, bp->r.src);
    pool_Qremove(NULL, &qdb->pool, &sbp->c.ll);
    pool_QinsertPred(NULL, &qdb->pool, &sbp->c.ll, &qp->read_lh);

    /* remove reference buffer */
    qdb_Free(sts, bp);

    bp = sbp;
  }

  if (gp == NULL || gp->data == NULL) {
    if (bp->c.type == qdb_eBuffer_reference) {

      sbp = pool_Address(sts, &qdb->pool, bp->r.src);
      nbp = qdb_CopyBuffer(&csts, sbp);

      pool_Qremove(NULL, &qdb->pool, &nbp->c.ll);
      pool_QinsertPred(NULL, &qdb->pool, &nbp->c.ll, &qp->read_lh);

      /* remove reference buffer */
      qdb_Free(sts, bp);

      bp = nbp;

    } else if (bp->c.flags.b.segmented) {

      nbp = qdb_CopyBuffer(&csts, bp);

      pool_Qremove(NULL, &qdb->pool, &nbp->c.ll);
      pool_QinsertPred(NULL, &qdb->pool, &nbp->c.ll, &qp->read_lh);

      /* remove segmented buffer */
      qdb_Free(sts, bp);

      bp = nbp;

    }
  }


  if (gp != NULL) {
    switch (bp->c.type) {
    case qdb_eBuffer_base:
      qdb_GetInfo(gp, bp);
      break;
    case qdb_eBuffer_reference:
      sbp = pool_Address(sts, &qdb->pool, bp->r.src);
      qdb_GetInfo(gp, sbp);
      break;
    default:
      break;
    }
  }

  return bp;

}

/* Copy the contents of a buffer to another buffer.  */

qdb_sBuffer *
qdb_CopyBuffer (
  pwr_tStatus	*status,
  qdb_sBuffer	*bp
)
{
  qdb_sBuffer	*nbp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  if (bp->c.type != qdb_eBuffer_base)
    errh_Bugcheck(QCOM__WEIRD, "wrong buffer type");

  nbp = qdb_Alloc(sts, bp->c.type, bp->b.info.size);
  if (nbp == NULL) return NULL;

  if (bp->c.flags.b.segmented)
    copyBufferData(nbp + 1, bp, 0, bp->b.info.size);
  else
    memcpy(nbp + 1, bp + 1, bp->b.info.size);

  nbp->c.flags.m  = bp->c.flags.m & ~qdb_mBuffer_segmented;
  nbp->b.noderef  = bp->b.noderef;  
  nbp->b.info	  = bp->b.info;

  return nbp;
}

/* Detach buffer from its application in queue.  */

qdb_sBuffer *
qdb_DetachBuffer (
  pwr_tStatus	*status,
  qdb_sBuffer	*bp
)
{
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  pool_Qremove(NULL, &qdb->pool, &bp->c.ll);

  return bp;
}

void
qdb_Put (
  pwr_tStatus	*status,
  qdb_sBuffer	*bp,
  qdb_sQue	*qp
)
{
  qdb_sQue	*fqp = NULL;
  qdb_sBuffer	*rbp = NULL;
  qdb_sQbond	*bop = NULL;
  pool_sQlink	*bol = NULL;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  pwr_Assert(bp != NULL);
  pwr_Assert(qp != NULL);

  qdb_AssumeLocked;

  switch (qp->type) {
  case qdb_eQue_private:
    qdb_Enque(sts, bp, qp);
    break;
  case qdb_eQue_forward:
    if (qp->flags.b.broadcast)
      bp->c.flags.b.broadcast = 1;

    for ( /* all bound queues */
      bol = pool_Qsucc(NULL, &qdb->pool, &qp->tgt_lh);
      bol != &qp->tgt_lh;
      bol = pool_Qsucc(NULL, &qdb->pool, bol)
    ) {
      bop = pool_Qitem(bol, qdb_sQbond, tgt_ll);
      fqp = pool_Address(NULL, &qdb->pool, bop->tgtQ);

      if (fqp->qix == qdb_cIexport && bp->c.flags.b.imported) continue;

      rbp = qdb_Alloc(sts, qdb_eBuffer_reference, 0);
      pool_QinsertPred(sts, &qdb->pool, &rbp->r.ref_ll, &bp->b.ref_lh);
      rbp->r.src = pool_ItemReference(sts, &qdb->pool, bp);
      qdb_Enque(sts, rbp, fqp);
    }
    if (pool_QisLinked(sts, &qdb->pool, &bp->b.ref_lh)) {
      /* buffer is forwarded, put base buffer in forward que */
      qdb_Enque(sts, bp, qp);
    } else {
      /* buffer is not forwarded, throw it */
      qdb_Free(sts, bp);
    }
    break;
  default:
    errh_Bugcheck(QCOM__WEIRD, "unknown queue type");
  }
}


void
qdb_RemoveAppl (
  pwr_tStatus	*status,
  qdb_sAppl	*ap
)
{
  pool_sQlink	*ql;
  pool_sQlink	*bl;
  qdb_sBuffer	*bp;
  qdb_sQue	*qp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  qdb_ApplEvent(NULL, ap, qcom_eStype_applDisconnect);

  for ( /* for all qwned queues */
    ql = pool_Qsucc(NULL, &qdb->pool, &ap->que_lh);
    ql != &ap->que_lh;
  ) {
    qp = pool_Qitem(ql, qdb_sQue, que_ll);
    ql = pool_Qsucc(NULL, &qdb->pool, ql);
    qdb_RemoveQue(sts, qp); 
  }

  hash_Remove(sts, &qdb->pid_ht, ap);
  hash_Remove(sts, &qdb->aix_ht, ap);
  pool_Qremove(sts, &qdb->pool, &ap->appl_ll);

  for (	/* free all buffers allocated to be sent */
    bl = pool_Qsucc(NULL, &qdb->pool, &ap->out_lh);
    bl != &ap->out_lh;
  ) {
    bp = pool_Qitem(bl, qdb_sBuffer, c.ll);
    bl = pool_Qsucc(NULL, &qdb->pool, bl);
    qdb_Free(sts, bp);
  }


  pool_Free(sts, &qdb->pool, ap);
}

void *
qdb_Request (
  pwr_tStatus	*status,
  qdb_sBuffer	*pbp,
  qdb_sQue	*pqp,
  qdb_sQue	*gqp,
  int		tmo,
  qcom_sGet	*gp,
  pwr_tBitMask  flags
)
{
  qdb_sBuffer	*gbp = NULL;
  qcom_tRid	rid;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  pwr_Assert(pbp != NULL);
  pwr_Assert(pqp != NULL);

  qdb_AssumeLocked;

  if (tmo < 0) {
    pwr_Return(NULL, sts, QCOM__HIGHTMO);
  }

  if (pqp->type != qdb_eQue_private) {
    qdb_Free(NULL, pbp);
    pwr_Return(NULL, sts, QCOM__QTYPE);
  }

  gqp->rid = rid = ++qdb->g->rid;
  pbp->b.info.rid = rid;
#if 0
  pool_Qremove(sts, &qdb->pool, &pbp->c.ll);
#endif

  gqp->flags.b.reply = 1;
  if (!qdb_Enque(sts, pbp, pqp))
    return NULL;
  gbp = qdb_Get(sts, gqp, tmo, gp, flags);
  gqp->flags.b.reply = 0;
  if (!pbp->c.flags.b.remote)
    return gbp;

  if (gbp != NULL && EVEN(gbp->b.info.status)) {
    *sts = gbp->b.info.status;
    qdb_Free(NULL, gbp);
    gbp = NULL;
  }

  return gbp;
}

/* Create a queue and make an implicit connect to it.  */

qdb_sQue *
qdb_AddQue (
  pwr_tStatus	*status,
  qcom_tQix	qix
)
{
  qdb_sQue	*qp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);
  pthread_condattr_t   condattr;
  pthread_mutexattr_t  mutexattr;

  qdb_AssumeLocked;

  if (qix != qcom_cNQix) {
    /* Make sure queue does not exist.  */
    qp = hash_Search(sts, &qdb->qix_ht, &qix);
    if (qp != NULL) pwr_Return(NULL, sts, QCOM__QALLREXIST);
  } else {
    if ( qdb->g->qid.qix == qdb_cQix_ReservedMin)
      qdb->g->qid.qix = qdb_cQix_ReservedMax + 1;
    qix = qdb->g->qid.qix++;
  }


  qp = pool_Alloc(sts, &qdb->pool, sizeof(qdb_sQue));
  if (qp == NULL) return NULL;
  pool_Qinit(NULL, &qdb->pool, &qp->qix_htl);
  pool_Qinit(NULL, &qdb->pool, &qp->que_ll);
  pool_Qinit(NULL, &qdb->pool, &qp->src_lh);
  pool_Qinit(NULL, &qdb->pool, &qp->tgt_lh);
  pool_Qinit(NULL, &qdb->pool, &qp->eve_lh);
  pool_Qinit(NULL, &qdb->pool, &qp->eve_ll);
  pool_Qinit(NULL, &qdb->pool, &qp->in_lh);
  pool_Qinit(NULL, &qdb->pool, &qp->rep_lh);
  pool_Qinit(NULL, &qdb->pool, &qp->read_lh);
  qp->qix = qix;
  
  pthread_condattr_init(&condattr);
  pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);
  pthread_mutexattr_init(&mutexattr);
  pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
  pthread_mutex_init(&qp->lock.mutex, &mutexattr);
  pthread_cond_init(&qp->lock.cond, &condattr);
  
  qp = hash_Insert(sts, &qdb->qix_ht, qp);
    
  return qp;
}

/*   */

qdb_sQue *
qdb_AttachQue (
  pwr_tStatus	*status,
  qdb_sQue	*qp,
  qdb_sAppl	*ap
)
{
  pwr_dStatus		(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  pool_QinsertPred(sts, &qdb->pool, &qp->que_ll, &ap->que_lh);
  qp->aid = ap->aid;
  qos_CreateQlock(sts, qp);

  return qp;
}

void
qdb_Eput (
  pwr_tStatus	*status,
  qdb_sQue	*ep
)
{
  qdb_sBuffer	*bp;
  qcom_sEvent	*event;

  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  if (pool_QisEmpty(sts, &qdb->pool, &ep->tgt_lh))
    return;

  bp = qdb_Alloc(sts, qdb_eBuffer_base, sizeof(*event));
  event = (qcom_sEvent*)(bp + 1);
  event->aid = qdb->my_aid;
  event->pid = qdb->my_pid;
  event->mask = ep->mask;
  
  bp->b.info.sender   = qdb->my_aid;
  bp->b.info.pid      = qdb->my_pid;
  bp->b.info.receiver.qix = ep->qix;
  bp->b.info.reply    = qcom_cNQid;
  bp->b.info.type.b   = qcom_eBtype_event;
  bp->b.info.type.s   = ep->qix;
  bp->b.info.rid      = 0;
  bp->b.info.size     = sizeof(*event);

  qdb_Put(sts, bp, ep);
}

/* .  */

void
qdb_PutInfo (
  qdb_sBuffer		*bp,
  qcom_sPut		*pp,
  const qcom_sQid	*receiver,
  qcom_tRid		rid
)
{
  qdb_sInfo		*ip = &bp->b.info;

  ip->sender	= qdb->ap->aid;
  ip->pid	= qdb->ap->pid;
  ip->receiver	= *receiver;
  ip->reply	= pp->reply;
  ip->type	= pp->type;
  ip->rid	= rid;
  ip->size	= pp->size;
}

pwr_tBoolean
qdb_RemoveQue (
  pwr_tStatus	*status,
  qdb_sQue	*qp
)
{
  pool_sQlink	*bl;
  pool_sQlink	*qbl;
  qdb_sBuffer	*bp;
  qdb_sQbond	*qbp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  for (	/* free all buffers to be read */
    bl = pool_Qsucc(NULL, &qdb->pool, &qp->in_lh);
    bl != &qp->in_lh;
  ) {
    bp = pool_Qitem(bl, qdb_sBuffer, c.ll);
    bl = pool_Qsucc(NULL, &qdb->pool, bl);
    qdb_Free(sts, bp);
  }

  for (	/* free all buffers allready read */
    bl = pool_Qsucc(NULL, &qdb->pool, &qp->read_lh);
    bl != &qp->read_lh;
  ) {
    bp = pool_Qitem(bl, qdb_sBuffer, c.ll);
    bl = pool_Qsucc(NULL, &qdb->pool, bl);
    qdb_Free(sts, bp);
  }

  for (	/* unbind from all source queues */
    qbl = pool_Qsucc(NULL, &qdb->pool, &qp->src_lh);
    qbl != &qp->src_lh;
  ) {
    qbp = pool_Qitem(qbl, qdb_sQbond, src_ll);
    qbl = pool_Qsucc(NULL, &qdb->pool, qbl);
    pool_Qremove(sts, &qdb->pool, &qbp->src_ll);
    pool_Qremove(sts, &qdb->pool, &qbp->tgt_ll);
    pool_Free(sts, &qdb->pool, qbp);
  }

  for (	/* unbind from all target queues */
    qbl = pool_Qsucc(NULL, &qdb->pool, &qp->tgt_lh);
    qbl != &qp->tgt_lh;
  ) {
    qbp = pool_Qitem(bl, qdb_sQbond, tgt_ll);
    qbl = pool_Qsucc(NULL, &qdb->pool, qbl);
    pool_Qremove(sts, &qdb->pool, &qbp->tgt_ll);
    pool_Qremove(sts, &qdb->pool, &qbp->src_ll);
    pool_Free(sts, &qdb->pool, qbp);
  }

  /* Left to do !!! Unlink all queues linked eve_lh.  */

  /* unlink que from own_ll */
  pool_Qremove(sts, &qdb->pool, &qp->que_ll);
  pool_Qremove(sts, &qdb->pool, &qp->eve_ll);
  qos_DeleteQlock(sts, qp);

  hash_Remove(sts, &qdb->qix_ht, qp);
  pool_Free(sts, &qdb->pool, qp);

  return TRUE;
}

void
qdb_NetEvent (
  pwr_tStatus	*status,
  qdb_sNode	*np,
  qcom_eStype	event
)
{
  qcom_sNode	*node;
  qdb_sQue	*qp;
  qdb_sBuffer	*bp;
  pwr_dStatus	(sts, status, QCOM__SUCCESS);
  
  qdb_AssumeLocked;

  qp = qdb_Que(sts, &qcom_cQnetEvent, NULL);
  bp = qdb_Alloc(sts, qdb_eBuffer_base, sizeof(qcom_sNode));
  node = (qcom_sNode *)(bp + 1);

  bp->b.info.sender	    = qdb->my_aid;
  bp->b.info.receiver.qix   = qp->qix;
  bp->b.info.receiver.nid   = qdb->g->nid;
  bp->b.info.type.b	    = qcom_eBtype_qcom;
  bp->b.info.type.s	    = event;
  bp->b.info.size	    = sizeof(qcom_sNode);

  qdb_NodeInfo(NULL, node, np);
  
  qdb_Put(sts, bp, qp);

}

void
qdb_NodeInfo (
  pwr_tStatus	*status,
  qcom_sNode	*node,
  qdb_sNode	*np
)
{

  pwr_Status(status, QDB__SUCCESS);

  node->flags.b.initiated = np->flags.b.initiated;
  node->flags.b.connected = np->flags.b.connected;
  node->flags.b.active = np->flags.b.active;
  node->nid = np->nid;
  strncpy(node->name, np->name, sizeof(np->name));
  node->os = np->os;
  node->hw = np->hw;
  node->bo = np->bo;
  node->ft = np->ft;
  node->connection = np->connection;
}

void
qdb_Platform (
  qdb_sNode	*np
)
{
  co_sPlatform platform;

  co_GetOwnPlatform(&platform);
  np->os = platform.os;
  np->hw = platform.hw;
  np->bo = platform.bo;
  np->ft = platform.ft;
}


qdb_sQue *
qdb_Que (
  pwr_tStatus		*status,
  const qcom_sQid	*qid,
  qdb_sNode		**onp
)
{
  qdb_uQid		q;
  qdb_sNode		*np = NULL;
  qdb_sQue		*qp = NULL;
  pwr_dStatus		(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  q.pwr = *qid;
  q.q.nid_3 = 0;
  if (q.pwr.nid == pwr_cNNodeId && q.pwr.qix != qcom_cNQix)
    q.pwr.nid = qdb->my_nid;

  if (q.pwr.nid != pwr_cNNodeId && q.pwr.nid != qdb->g->nid) {
    if (onp == NULL) pwr_Return(NULL, sts, QCOM__QNOTLOCAL);

    np = hash_Search(sts, &qdb->nid_ht, &q.pwr.nid);
    if (np == NULL) pwr_Return(NULL, sts, QCOM__NOLINK);
    if (!np->flags.b.connected) pwr_Return(NULL, sts, QCOM__NOLINK);
    if (!np->flags.b.active && !qdb->flags.b.ignoreStall) pwr_Return(NULL, sts, QCOM__LINKDOWN);

    /* the que may exist on the remote node, lets export it */
    qp = qdb->exportque;
    if (qp == NULL) pwr_Return(NULL, sts, QCOM__NOEXPORT);
  } else {
    np = qdb->my_node;
    qp = hash_Search(sts, &qdb->qix_ht, &q.pwr.qix);
    if (qp == NULL) pwr_Return(NULL, sts, QCOM__NOQ);
  }

  if (onp != NULL)
    *onp = np;

  return qp;
}

pwr_tBoolean
qdb_Signal (
  pwr_tStatus	*status,
  qdb_sQue	*ep
)
{
  pool_sQlink	*ql;
  qdb_sQue	*qp;

  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  for (
    ql = pool_Qsucc(NULL, &qdb->pool, &ep->eve_lh);
    ql != &ep->eve_lh;
  ) {
    qp = pool_Qitem(ql, qdb_sQue, eve_ll);
    ql = pool_Qsucc(NULL, &qdb->pool, ql);

    if (qp->or_event) {
      if (!(qp->mask & ep->mask))
	continue;
    } else {
      if ((qp->mask & ep->mask) != qp->mask)
	continue;
    }

    pool_Qremove(sts, &qdb->pool, &qp->eve_ll);
    qos_SignalQue(sts, qp);
  }

  return ODD(*sts);
}

int
qdb_Wait (
  pwr_tStatus	*status,
  qdb_sQue	*qp,
  qdb_sQue	*ep,
  int		tmo
)
{
  pwr_dStatus	(sts, status, QCOM__SUCCESS);

  qdb_AssumeLocked;

  pool_QinsertPred(NULL, &qdb->pool, &qp->eve_ll, &ep->eve_lh);

  return qos_WaitQue(sts, qp, tmo);
}
