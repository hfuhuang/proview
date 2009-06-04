/* 
 * Proview   $Id: rt_mh_outunit.c,v 1.5 2005-09-01 14:57:56 claes Exp $
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

/* rt_mh_outunit.c -- Out unit */

#ifdef OS_ELN
# include stdio
# include stdlib
# include string
#else
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#endif

#ifdef OS_VMS
#include <starlet.h>
#include <signal.h>
#endif

#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "co_time.h"
#include "co_cdh.h"
#include "rt_errh.h"
#include "rt_mh_outunit.h"
#include "rt_mh_def.h"
#include "rt_mh_net.h"
#include "rt_mh_util.h"
#include "rt_qcom.h"
#include "rt_qcom_msg.h"
#include "rt_mh_log.h"

 
typedef struct s_Ack	  sAck;
typedef struct s_Block	  sBlock;
typedef struct s_Handler  sHandler;
LstType(sAck);
LstType(sBlock);
LstType(sHandler);

struct s_Ack {
  LstLink(sAck)	      ack_l;	
  mh_sOutunitAck      ack;
};

struct s_Block {
  LstLink(sBlock)     block_l;	
  mh_sOutunitBlock    block;
};

struct s_Handler {
  LstLink(sHandler)   handler_l;
  LstLink(sHandler)   sync_l;
  pwr_tTime	      birthTime;	/* Time when handler started */
  qcom_sQid	      qid;		/* Qcom queue of handler */
  qcom_sAid	      aid;		/* Qcom application id */
  co_sPlatform	      platform;		/* Platform identity */
  pwr_tNodeIndex      nix;		/* Node index of handler */
  LstHead(sAck)	      ack_l;		/* Head to ack list */
  pwr_tUInt32	      ackGen;		/* Ack generation */
  LstHead(sBlock)     block_l;		/* Head to block list */
  pwr_tUInt32	      blockGen;		/* Block generation */
  pwr_tBoolean	      isDummy;
  pwr_tBoolean	      linkUp;
  pwr_tUInt32	      selGen;
  pwr_tUInt32	      eventGen;
  pwr_tUInt32	      eventIdx;		/* Index of last received event from handler */
};

typedef struct {
  mh_sHead		head;
  mh_eOutunitType	type;     /* Type of outunit */
  pwr_tBoolean		isConnected;
  mh_mOutunitFlags	flags;
  int			timeout;
  int			tempTimeout;
  pwr_tStatus		(*cbAck)(mh_sAck *);
  pwr_tStatus		(*cbAlarm)(mh_sMessage *);
  pwr_tStatus		(*cbBlock)(mh_sBlock *);
  pwr_tStatus		(*cbCancel)(mh_sReturn *);
  pwr_tStatus		(*cbClearAlarmList)(pwr_tNodeIndex);
  pwr_tStatus		(*cbClearBlockList)(pwr_tNodeIndex);
  pwr_tStatus		(*cbInfo)(mh_sMessage *);
  pwr_tStatus		(*cbReturn)(mh_sReturn *);
  LstHead(sHandler)	handler_l;    
  LstHead(sHandler)	sync_l;    
  pwr_tUInt32		selGen;
  pwr_tUInt32		selSize;
  pwr_tString80		(*pSelL)[];
  mh_sSelL		selL[mh_cSelLSize];
  pwr_tBoolean		*SelectListIsUpdated;
} sContext;


/* Local variables */

static sContext l;

/* Local function prototypes */

static void ackListDelete(sHandler*, sAck*);
static void ackListDestroy(sHandler*);
static void ackListInsert(sHandler*, pwr_tUInt32);
static void blockListDelete(sHandler*, sBlock*);
static void blockListDestroy(sHandler*);
static void blockListInsert(sHandler*, pwr_tObjid*, pwr_tUInt32);
static void checkSync();
static void checkSyncListDelete(sHandler*);
static void checkSyncReply(sHandler*);
static pwr_tStatus enableQcomAllOutunits();
static pwr_tStatus enableQcomEvents();
static void fromHandler(mh_sHead*, int, qcom_sAid*);
static void fromQcom(qcom_sGet*);
static void getSelectList();
static void handlerEvent(sHandler*, mh_sHead*);
static void handlerLog(sHandler*, char*);
static void handlerSync(sHandler*, mh_sHead*);
static pwr_tBoolean isValidHandler(mh_sHead*, qcom_sAid*, sHandler**);
static void linkDown(qcom_sNode*);
static void linkUp(qcom_sNode*);
static void procDown(qcom_sAid*);
static void sendInfo(sHandler*);
static pwr_tStatus sendToHandler(sHandler*, mh_eMsg, unsigned int, void*);

/** 
 *@brief Informs all Handlers known to this Outunit that the
 * message with identity TargetId is acknowleged.  
 */

pwr_tStatus
mh_OutunitAck (
  mh_sEventId  *targetId    /**< The identity of an alarm.*/ 
)
{
  pwr_tStatus sts = MH__SUCCESS;
  sHandler *hp;
  LstLink(sHandler) *hl;

  /* Find originating handler */

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstNex(hl))
    if (LstObj(hl)->nix == targetId->Nix)
      break;

  if (hl == LstEnd(&l.handler_l)) {
    return MH__HANDLERDOWN;
  } else {
    hp = LstObj(hl);
  }

  ackListInsert(hp, targetId->Idx);

  return sts;
}

/**
 * @brief Send a block request. Prio == 0 -> unblock. 
 */

pwr_tStatus
mh_OutunitBlock (
  pwr_tObjid object,       /**< The identity of an object to be blocked. */
  mh_eEventPrio prio       /**< The priority to block. */
)
{
  pwr_tStatus		sts;
  sHandler		*hp;
  LstLink(sHandler)	*hl;
  pwr_tNodeIndex	nix;
  pwr_tBoolean		is_mount_clean;	

  /* Find corresponding handler */

  sts = gdh_GetObjectNodeIndex(object, &nix);
  if (EVEN(sts)) return sts;

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstNex(hl)) {
    if ( !hl)
      return MH__HANDLERDOWN;

    if (LstObj(hl)->nix == nix)
      break;
  }

  if (hl == LstEnd(&l.handler_l)) {
    return MH__HANDLERDOWN;
  } else {
    hp = LstObj(hl);
  }

  blockListInsert(hp, &object, prio);

  sts = gdh_IsMountClean(object, &is_mount_clean);
  if (ODD(sts) && !is_mount_clean)
    return MH__NOTMNTCLEAN;

  return MH__SUCCESS;
}

/** 
 *@brief Connects this Outunit to the local Handler.  
 *@return pwr_tStatus
 */

pwr_tStatus
mh_OutunitConnect (
  pwr_tObjid			outunit,          /**< Object identity of an outunit object.*/
  mh_eOutunitType		type,             /**< Type of outunit. */
  mh_mOutunitFlags		flags,            /**< ZZZ */
  mh_cbOutunitAck		cbAck,            /**< Address of a function to receive aknowledge messages.*/ 
  mh_cbOutunitAlarm		cbAlarm,          /**< Address of a function to receive alarm messages messages.*/ 
  mh_cbOutunitBlock		cbBlock,          /**< Address of a function to receive blocking messages.*/ 
  mh_cbOutunitCancel		cbCancel,         /**< Address of a function to receive cancel messages.*/   
  mh_cbOutunitClearAlarmList	cbClearAlarmList, /**< Address of a function to clear alarm list from alarms 
                                                       generated by the node corresponding to PAMS_ADDRESS. */ 
  mh_cbOutunitClearBlockList	cbClearBlockList, /**< Address of a function to clear block list from blocks 
						       generated by the node corresponding to PAMS_ADDRESS.*/ 
  mh_cbOutunitInfo		cbInfo,           /**< ZZZ */
  mh_cbOutunitReturn		cbReturn          /**< ZZZ */
)
{
  pwr_tStatus	sts;
  pwr_tObjid	*p;
  pwr_tClassId	cid;
  qcom_sNode	myNode;
  qcom_sQattr	qAttr;
  int i;
  sHandler	*hp;
  char		name[80];

  sprintf(name, "Outunit_%X", outunit.oix);

  mh_UtilWaitForMh();

  if (l.isConnected)
    return MH__ALLRCON;

  /* Check Outunit object */

  memset(&l, 0, sizeof(l));

  if (type == mh_eOutunitType_Logger) {
    l.SelectListIsUpdated = NULL;
  } else {
    sts = gdh_GetObjectClass(outunit, &cid);
    if (EVEN(sts)) return sts;
    sts = gdh_ObjidToPointer(outunit, (pwr_tAddress *)&p);
    if (EVEN(sts)) return sts;

    switch (cid) {
    case pwr_cClass_User:
      type = mh_eOutunitType_Operator;
      l.pSelL = (void *)&((pwr_sClass_User*) p)->SelectList[0];
      l.SelectListIsUpdated = (pwr_tBoolean *)&((pwr_sClass_User*) p)->SelectListIsUpdated;
      break;
    case pwr_cClass_EventPrinter:
      type = mh_eOutunitType_Printer;
      l.pSelL = (void *)&((pwr_sClass_EventPrinter*) p)->SelectList[0];
      l.SelectListIsUpdated = NULL;
      break;
    default:
      return MH__NOOUTUNIT;
      break;
    }
  }

  l.head.ver = mh_cVersion;

  l.head.qid = qcom_cNQid;

  qAttr.type = qcom_eQtype_private;
  qAttr.quota = 100;
  if (!qcom_CreateQ(&sts, &l.head.qid, &qAttr, name)) {
    errh_Error("mh_OutunitConnect: Failed to create QCOM que\n%m", sts);
    return MH__QCOMCREQ;
  } 

  qcom_MyNode(&sts, &myNode);
  l.head.platform.os = myNode.os;
  l.head.platform.hw = myNode.hw;

  l.head.source = mh_eSource_Outunit;
  time_GetTime(&l.head.birthTime);
  l.head.outunit = outunit;
  sts = gdh_GetNodeIndex(&l.head.nix);
  if (EVEN(sts)) return sts;
  l.type = type;
  l.flags = flags;
  l.timeout = flags == mh_mOutunitFlags_ReadWait ? -1 : 0;
  l.tempTimeout = l.timeout;
  l.cbAck = cbAck;
  l.cbAlarm = cbAlarm;
  l.cbBlock = cbBlock;
  l.cbCancel = cbCancel;
  l.cbClearAlarmList = cbClearAlarmList;
  l.cbClearBlockList = cbClearBlockList;
  l.cbInfo = cbInfo;
  l.cbReturn = cbReturn;

  LstIni(&l.handler_l);
  LstIni(&l.sync_l);
    
  for (i = 0; i < 5; i++) {
    hp = (sHandler*) calloc(1, sizeof(*hp));
    hp->isDummy = TRUE;
    LstIns(&l.sync_l, hp, sync_l);
  }

  getSelectList();

  sts = enableQcomEvents();
  if (EVEN(sts)) return sts;
  sts = enableQcomAllOutunits();
  if (EVEN(sts)) return sts;

  sts = sendToHandler(NULL, mh_eMsg_OutunitHello, 0, NULL);
  if (EVEN(sts)) return sts;
  l.isConnected = TRUE;

  return MH__SUCCESS;
}

/** 
 *@brief Inform all Handlers known to this Outunit, to remove
 * this Outunit from their list of known out units.
 *
 * The Outunit context block local to the mh_Outunit
 * interface will be cleared.  
 *@return pwr_tStatus
 */

pwr_tStatus
mh_OutunitDisconnect ()
{
  sHandler *hp;
  LstLink(sHandler) *hl;

  /* Disconnect all handlers and free memory */

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstFir(&l.handler_l)) {
    hp = LstObj(hl);
    sendToHandler(hp, mh_eMsg_OutunitDisconnect, 0, NULL);
    ackListDestroy(hp);
    blockListDestroy(hp);
    LstRem(hl);
    LstNul(hl);
    free(hp);
  }   

  l.isConnected = FALSE;
  return MH__SUCCESS;
}

/**
 *@brief Receive next messages in Queue. 
 *@return pwr_tStatus
 */

pwr_tStatus
mh_OutunitReceive ()
{
  pwr_tStatus	sts;
  static char	mp[2000];
  mh_sHead	*hp;
  qcom_sGet	msg;
  XDR		xdrs;

  /* If SelectListIsUpdated is set, the outunit will be updated */
  if (l.type == mh_eOutunitType_Operator && *l.SelectListIsUpdated)
  {
    *l.SelectListIsUpdated = 0;
    mh_OutunitUpdate();
  }

  msg.data = mp;
  msg.maxSize = sizeof(mp);
  qcom_Get(&sts, &l.head.qid, &msg, l.tempTimeout);
  l.tempTimeout = l.timeout;
  if (sts == QCOM__QEMPTY) {
    checkSync();
    return MH__NOMESSAVAIL;
  } else if (sts == QCOM__TMO) {
    checkSync();
    return MH__TMO;
  } else if (EVEN(sts)) {
    return MH__QCOMGETMSG;
  }

  switch (msg.type.b) {

  case qcom_eBtype_qcom:
    fromQcom(&msg);
    break;

  case mh_cMsgClass:
    hp = (mh_sHead*) mp;
    if (hp->xdr) {
      xdrmem_create(&xdrs, (char *)hp, msg.size, XDR_DECODE);
      sts = mh_NetXdrMessage(&xdrs, msg.type.s, hp);
      if (EVEN(sts))
        return sts;
    }
    fromHandler(hp, msg.size, &msg.sender);
    break;
  default:
    break;
  }

  return MH__SUCCESS;
}

/** 
 *@brief Sets the timeout time in ms for mh_OutunitReceive.
 *
 * This routine must be called before mh_OutunitReceive.
 * An negative number means wait forever
 *@return pwr_tStatus
 */
pwr_tStatus
mh_OutunitSetTimeout (
  int timeout             /**< The timeout in ms. */
)
{
  l.timeout = timeout;
  return MH__SUCCESS;
}

/** 
 *@brief Inform all event monitors known to this outunit,
 * that the select list is changed. 
 *@return pwr_tStatus
 */

pwr_tStatus
mh_OutunitUpdate ()
{
  LstLink(sHandler) *hl;

  getSelectList();

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstNex(hl))
    sendInfo(LstObj(hl));

  return MH__SUCCESS;
}


static void
ackListDelete (
  sHandler	*hp,
  sAck		*ap
)
{
  LstLink(sAck) *al;

  al = LstRem(&ap->ack_l);
  LstNul(&ap->ack_l);
  free(ap);

  checkSyncListDelete(hp);
}

static void
ackListDestroy (
  sHandler *hp
)
{
  sAck *ap;
  LstLink(sAck) *al;

  for (al = LstFir(&hp->ack_l); al != LstEnd(&hp->ack_l); al = LstFir(&hp->ack_l)) {
    ap = LstObj(al);
    LstRem(al);
    LstNul(al);
    free(ap);
  }

  hp->ackGen = 0;
  checkSyncListDelete(hp);
}

static void
ackListInsert (
  sHandler *hp,
  pwr_tUInt32  targetIdx
)
{
  sAck *ap;
  LstLink(sAck) *al;

  /* Don't insert if already present */
  for (al = LstFir(&hp->ack_l); al != LstEnd(&hp->ack_l); al = LstNex(al))
    if (LstObj(al)->ack.targetIdx == targetIdx)
      return;

  ap = (sAck*) calloc(1, sizeof(*ap));
  ap->ack.ackGen = ++hp->ackGen;
  ap->ack.targetIdx = targetIdx;
  al = LstEnd(&hp->ack_l);
  LstIns(al, ap, ack_l);

  if (!LstInl(&hp->sync_l)) {
    LstIns(LstEnd(&l.sync_l), hp , sync_l);
  }

  if (ap == LstObj(LstFir(&hp->ack_l))) {
    sendToHandler(hp, mh_eMsg_OutunitAck, sizeof(ap->ack), (void *) &ap->ack);
  }    
}

static void
blockListDelete (
  sHandler *hp,
  sBlock *bp
)
{
  LstLink(sBlock) *bl;

  bl = LstRem(&bp->block_l);
  LstNul(&bp->block_l);
  free(bp);

  checkSyncListDelete(hp);
}

static void
blockListDestroy (
  sHandler *hp
)
{
  sBlock *bp;
  LstLink(sBlock) *bl;

  for (bl = LstFir(&hp->block_l); bl != LstEnd(&hp->block_l); bl = LstFir(&hp->block_l)) {
    bp = LstObj(bl);
    LstRem(bl);
    LstNul(bl);
    free(bp);
  }

  hp->blockGen = 0;
  checkSyncListDelete(hp);
}

static void
blockListInsert (
  sHandler *hp,
  pwr_tObjid  *op,
  pwr_tUInt32 prio
)
{
  sBlock *bp;
  LstLink(sBlock) *bl;

  bp = (sBlock*) calloc(1, sizeof(*bp));
  bp->block.blockGen = ++hp->blockGen;
  bp->block.object   = *op;
  bp->block.outunit  = l.head.outunit;
  bp->block.prio     = prio;
#if 0
  bp->block.time
#endif
  bl = LstEnd(&hp->block_l);
  LstIns(bl, bp, block_l);

  if (!LstInl(&hp->sync_l)) {
    LstIns(LstEnd(&l.sync_l), hp , sync_l);
  }

  if (bp == LstObj(LstFir(&hp->block_l))) {
    sendToHandler(hp, mh_eMsg_OutunitBlock, sizeof(bp->block), (void *) &bp->block);
  }    
}

static void
checkSync ()
{
  LstLink(sHandler) *sl;
  sHandler *hp;

  for (sl = LstFir(&l.sync_l); sl != LstEnd(&l.sync_l); sl = LstFir(&l.sync_l)) {
    hp = LstObj(sl);
    LstRem(sl);
    LstIns(LstEnd(&l.sync_l), hp, sync_l);
    if (hp->isDummy)
      return;
    if (!hp->linkUp)
      return;
    // handlerLog(hp, "Sending sync");
    sendToHandler(hp, mh_eMsg_OutunitSync, 0, NULL);
  }
}

static void
checkSyncListDelete (
  sHandler *hp
)
{
  
  if (!LstInl(&hp->sync_l)) return;
  if (!LstEmp(&hp->ack_l)) return;
  if (!LstEmp(&hp->block_l)) return;
  if (hp->selGen != l.selGen) return;

  LstRem(&hp->sync_l);
  LstNul(&hp->sync_l);
}

static void
checkSyncReply (
  sHandler		*hp
)
{
  
  if (hp->selGen != l.selGen) {
    sendInfo(hp);
  } else if (!LstEmp(&hp->block_l)) {
    sBlock *bp;

    bp = LstObj(LstNex(&hp->block_l));
    sendToHandler(hp, mh_eMsg_OutunitBlock, sizeof(bp->block), (void *) &bp->block);
  } else if (!LstEmp(&hp->ack_l)) {
    sAck *ap;

    ap = LstObj(LstNex(&hp->ack_l));
    sendToHandler(hp, mh_eMsg_OutunitAck, sizeof(ap->ack), (void *) &ap->ack);
  }

  if (LstInl(&hp->sync_l)) {
    LstRem(&hp->sync_l);
    LstIns(LstEnd(&l.sync_l), hp, sync_l);
  }
}

static pwr_tStatus
enableQcomAllOutunits ()
{
  pwr_tStatus sts;
  qcom_sQid allOuQid = mh_cProcAllOutunits;

  if (!qcom_Bind(&sts, &l.head.qid, &allOuQid)) {
    errh_Error("EnableQcomAllOutunits: qcom_Bind failed, reason:\n%m", sts);  
    return MH__QCOMBINDQ;
  }

  return MH__SUCCESS;
}

static pwr_tStatus
enableQcomEvents ()
{
  pwr_tStatus sts;
  qcom_sQid otherQue;

  otherQue = qcom_cQnetEvent;
  if (!qcom_Bind(&sts, &l.head.qid, &otherQue)) {
    errh_Error("EnableQcomEvents: Binding link que failed, reason:\n%m", sts);  
    return MH__QCOMBINDQ;
  }

  otherQue = qcom_cQapplEvent;
  if (!qcom_Bind(&sts, &l.head.qid, &otherQue)) {
    errh_Error("EnableQcomEvents: Binding process notify que failed, reason:\n%m", sts);  
    return MH__QCOMBINDQ;
  }

  return MH__SUCCESS;
}

static void
fromHandler (
  mh_sHead *p,
  int size,
  qcom_sAid *aid
)
{
  sHandler *hp;

  if (!isValidHandler(p, aid, &hp))
    return;

  switch (p->type) {
  case mh_eMsg_Event:
    handlerEvent(hp, p);
    handlerSync(hp, p);
    break;
  case mh_eMsg_HandlerSync:
    handlerSync(hp, p);
    sendToHandler(hp, mh_eMsg_Sync, 0, NULL); 
    break;
  case mh_eMsg_Sync:
    handlerSync(hp, p);
    checkSyncReply(hp);
    break;
  case mh_eMsg_HandlerHello:
    if (hp->selGen != l.selGen)
      sendInfo(hp);
    break;
  default:
    errh_Warning("Received unhandled messagetype: %d", p->type);
    break;
  }
}

static void
fromQcom (
  qcom_sGet *mp
)
{

  switch (mp->type.s) {
  case qcom_eStype_applDisconnect:
    procDown(&((qcom_sAppl*) mp->data)->aid);
    break;
  case qcom_eStype_linkDisconnect:
    linkDown((qcom_sNode*)mp->data);
    break;
  case qcom_eStype_linkConnect:
    linkUp((qcom_sNode*)mp->data);
    break;
  default:
    break;
  }
}

static void
getSelectList ()
{
  int i;
  int j;
  int len;
  char *s;

  if (l.type == mh_eOutunitType_Logger) {
    l.selSize = 0;
    l.selGen = 1;
    return;
  }

  memset(&l.selL, 0, sizeof(l.selL));

  for (i = 0, j = 0; i < mh_cSelLSize; i++) {
    for ( s = (*l.pSelL)[i], len = 0; !(*s == 0 || *s == 32 || * s == 9); s++)
      len++;
    len = MIN(len, sizeof(pwr_tString80));
    if (len == 0)
      continue;
    l.selL[j].len = len;
    strncpy(l.selL[j].objName, (*l.pSelL)[i], len);
    l.selL[j].objName[sizeof(pwr_tString80) - 1] = '\0';
    cdh_ToUpper(l.selL[j].objName, l.selL[j].objName);
    j++;
  }
  l.selSize = j;
  l.selGen++;
}

static void
handlerEvent (
  sHandler *hp,
  mh_sHead *p
)
{
  pwr_tStatus sts;
  mh_sMsgInfo *mp = (mh_sMsgInfo*) (p + 1);
  LstLink(sAck) *al;
  mh_sAck *ap;
  mh_sReturn *cp;

  /* Skip events sent from an old select list. */
  if (p->selGen != l.selGen)
    return;

  if (hp->eventGen != p->eventGen) {
    /* This is the first event with a new select list. */
    if (l.cbClearAlarmList != NULL)
      l.cbClearAlarmList(hp->nix);
    if (l.cbClearBlockList != NULL)
      l.cbClearBlockList(hp->nix);
    hp->eventGen = p->eventGen;
  }

  hp->eventIdx = mp->Id.Idx;
  sendToHandler(hp, mh_eMsg_Sync, 0, NULL);

  if (l.tempTimeout == 0)
    l.tempTimeout = 100;

  switch (mp->EventType) {
  case mh_eEvent_Ack:
    /* Delete alarm from local ack list, if present */

    ap = (mh_sAck *)mp;

    for (al = LstFir(&hp->ack_l); al != LstEnd(&hp->ack_l); al = LstNex(al))
      if (LstObj(al)->ack.targetIdx == ap->TargetId.Idx) {
        ackListDelete(hp, LstObj(al));
	break;
      }

    if (l.cbAck != NULL)
      l.cbAck(ap);
    break;
  case mh_eEvent_Alarm:
    if (l.cbAlarm != NULL)
      sts = l.cbAlarm((mh_sMessage*) mp);
    break;
  case mh_eEvent_Block:
  case mh_eEvent_Unblock:
  case mh_eEvent_Reblock:
  case mh_eEvent_CancelBlock:
    if (l.cbBlock != NULL)
      sts = l.cbBlock((mh_sBlock*) mp);
    break;
  case mh_eEvent_Cancel:
    /* Delete alarm from local ack list, if present */

    cp = (mh_sReturn*) mp;

    for (al = LstFir(&hp->ack_l); al != LstEnd(&hp->ack_l); al = LstNex(al))
      if (LstObj(al)->ack.targetIdx == cp->TargetId.Idx) {
        ackListDelete(hp, LstObj(al));
	break;
      }

    if (l.cbCancel != NULL)
      sts = l.cbCancel(cp);
    break;
  case mh_eEvent_Info:
    if (l.cbInfo != NULL)
      sts = l.cbInfo((mh_sMessage*) mp);
    break;
  case mh_eEvent_Return:
    if (l.cbReturn != NULL)
      sts = l.cbReturn((mh_sReturn*) mp);
    break;
  default:
    errh_Warning("Received unknown event type", mp->EventType);
    break;
  }

}

static void
handlerLog (
  sHandler *hp,
  char *s
)
{

  errh_Info("%s, (%s)", s, qcom_QidToString(NULL, &hp->qid, 1));
}

static void
handlerSync (
  sHandler *hp,
  mh_sHead *p
)
{

  if (!LstEmp(&hp->ack_l)) {
    sAck *ap;

    ap = LstObj(LstNex(&hp->ack_l));
    if (ap->ack.ackGen == p->ackGen)
      ackListDelete(hp, ap);    
  }

  if (!LstEmp(&hp->block_l)) {
    sBlock *bp;

    bp = LstObj(LstNex(&hp->block_l));
    if (bp->block.blockGen == p->blockGen)
      blockListDelete(hp, bp);    
  }

  if (hp->selGen != l.selGen && p->selGen == l.selGen) {
    hp->selGen = l.selGen;
    checkSyncListDelete(hp);
  }
}

static pwr_tBoolean
isValidHandler (
  mh_sHead *p,
  qcom_sAid *aid,
  sHandler **h
)
{
  sHandler *hp;
  LstLink(sHandler) *hl;

  if (p->ver != mh_cVersion) {
    /* Different versions, not yet implemented */
    errh_Warning("Received a Message with different version: %d != %d", p->ver, mh_cVersion);
    *h = NULL;
    return FALSE;
  }

  /* Find handler in handler list */

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstNex(hl))
    if (LstObj(hl)->qid.qix == p->qid.qix 
        && LstObj(hl)->qid.nid == p->qid.nid
    )
      break;

  if (hl == LstEnd(&l.handler_l))  {
    /* Handler not known, make it known */
    hp = (sHandler*) calloc(1, sizeof(*hp));
    hp->birthTime = p->birthTime;
    hp->qid = p->qid;
    hp->aid = *aid;
    hp->platform = p->platform;
    hp->nix = p->nix;
    hp->linkUp = TRUE;
    LstIns(&l.handler_l, hp, handler_l);
    LstIni(&hp->ack_l);
    LstIni(&hp->block_l);
    handlerLog(hp, "New handler");
  } else {
    hp = LstObj(hl);

    if (memcmp(&hp->birthTime, &p->birthTime, sizeof(hp->birthTime)) != 0) {
      /* Different times, i.e. the handler is restarted */
      hp->birthTime = p->birthTime;
      hp->qid = p->qid;
      hp->aid = *aid;
      ackListDestroy(hp);
      blockListDestroy(hp);
      hp->linkUp = TRUE;
      hp->eventGen = 0;
      hp->eventIdx = 0;
      hp->selGen = 0;
      handlerLog(hp, "Restarted handler");
    }
  }

  *h = hp;
  return TRUE;
}

static void
linkDown (
  qcom_sNode  *nodep
)
{
  LstLink(sHandler) *hl;
  sHandler *hp;

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstNex(hl))
    if (LstObj(hl)->qid.nid == nodep->nid) {
      hp = LstObj(hl);
      hp->linkUp = FALSE;
      handlerLog(hp, "Link down");
      return;
    }
}

static void
linkUp (
  qcom_sNode  *nodep
)
{
  LstLink(sHandler) *hl;
  sHandler *hp;

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstNex(hl))
    if (LstObj(hl)->qid.nid == nodep->nid) {
      hp = LstObj(hl);
      hp->linkUp = TRUE;
      handlerLog(hp, "Link up");
      return;
    }
}

static void
procDown (
  qcom_sAid *aid
)
{
  sHandler *hp;
  LstLink(sHandler) *hl;

  for (hl = LstFir(&l.handler_l); hl != LstEnd(&l.handler_l); hl = LstNex(hl))
    if (LstObj(hl)->aid.aix == aid->aix && LstObj(hl)->aid.nid == aid->nid) {
      /* Delete this handler from handler list */
      hp = LstObj(hl);
      handlerLog(hp, "Handler aborted");
      ackListDestroy(hp);
      blockListDestroy(hp);
      LstRem(hl);
      LstNul(hl);
      free(hp);
      return;
    }
}

static void
sendInfo (
  sHandler	  *hp
)
{
  pwr_tStatus	  sts;
  mh_sOutunitInfo *ip;
  char		  *mp;
  unsigned int	  size;

  size = sizeof(mh_sOutunitInfo) + l.selSize * sizeof(mh_sSelL);

  mp = calloc(1, size);
  if (mp == NULL)
    return;

  ip = (mh_sOutunitInfo *)mp;
  ip->type     = l.type;
  ip->selGen   = l.selGen;
  ip->selSize  = l.selSize;

  if (ip->selSize > 0)
    memcpy(ip + 1, &l.selL[0], ip->selSize * sizeof(mh_sSelL));

  sts = sendToHandler(hp, mh_eMsg_OutunitInfo, size, mp);

  free(mp);

  if (!LstInl(&hp->sync_l)) {
    LstIns(LstEnd(&l.sync_l), hp, sync_l);
  }
}

static pwr_tStatus
sendToHandler (
  sHandler	*hp,
  mh_eMsg	type,
  unsigned int	size,
  void		*bp
)
{
  pwr_tStatus	sts;
  char		*mp;
  unsigned int	msize = sizeof(l.head) + size;
  co_sPlatform	*pfp;
  qcom_sQid	qid;
  mh_sHead	*p;

  mp = calloc(1, msize);

  memcpy(mp, &l.head, sizeof(l.head));
  p = (mh_sHead *) mp;
  p->type = type;

  if (size > 0)
    memcpy(mp + sizeof(l.head), bp, size);

  if (hp == NULL) {
    /* Send to all handlers */

    qid = mh_cProcAllHandlers;
    pfp = NULL;
  } else {
    p->eventGen = hp->eventGen;
    p->eventIdx = hp->eventIdx;
    qid = hp->qid;
    pfp = &hp->platform;
  }

  sts = mh_NetSendMessage(&qid, pfp, 0, 0, (mh_sHead *)mp, msize);

  free(mp);
  if (ODD(sts)) 
    return sts;

  errh_Error("sendToHandler failed\n%m", sts);

  return sts;
}
