/* 
 * Proview   $Id: rt_alimserver.c,v 1.3 2005-09-01 14:57:48 claes Exp $
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

/* rt_alimserver.c -- <short description>

   This is a program that given an object identity finds all
   supervisor objects connected to it. It then returns the 
   objid of the supervisor object and the limit.  

   */

#ifdef OS_ELN
# include stdio
# include stdlib
# include string
#else
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
#endif

#include "pwr_lst.h"
#include "co_cdh.h"
#include "co_tree.h"
#include "rt_gdh.h"
#include "rt_errh.h"
#include "rt_gdh_msg.h"
#include "rt_ini_event.h"
#include "rt_qcom_msg.h"
#include "rt_qcom.h"
#include "rt_alimsrv.h"
#include "rt_pwr_msg.h"


typedef struct sAttribute sAttribute;
typedef struct sObject sObject;
typedef struct sSupObject sSupObject;

LstType(sAttribute);
LstType(sSupObject);

struct sObject {
  tree_sNode		tn;
  pwr_tObjid		oid;
  int			sup_c;
  int			attr_c;
  LstHead(sAttribute)	attr_l;
};

struct sAttribute {
  LstLink(sAttribute)	attr_l;
  pwr_sAttrRef		aref;
  pwr_tClassId		cid;
  int			sup_c;
  LstHead(sSupObject)	sup_l;
};

struct sSupObject {
  LstLink(sSupObject)	sup_l;
  pwr_tObjid		oid;
  void			*p;
};

static tree_sTable *ltp;

static alimsrv_sSupDataBuf *buildBuffer(alimsrv_sRequest*, unsigned int*);
static void init();
static void event (qcom_sGet	*get);
static sAttribute *findAttribute(sObject*, pwr_sAttrRef*, pwr_tClassId);


static alimsrv_sSupDataBuf *
buildBuffer (
  alimsrv_sRequest  *rqp,
  unsigned int      *size
)
{
  alimsrv_sSupDataBuf	*bp = NULL;
  alimsrv_sAttrSupList	*bap;
  alimsrv_sSupObjEntry	*bsp;
  sObject		*op;
  sAttribute		*ap;
  sSupObject		*sp;
  LstLink(sAttribute)	*al;
  LstLink(sSupObject)	*sl;
  pwr_tStatus           sts;

  *size = 0;

  op = tree_Find(&sts, ltp, &rqp->Attribute.Objid);
  if (op == NULL)
    return NULL;
  if (op->attr_c == 0)
    return NULL;

  *size = sizeof(alimsrv_sSupDataBuf) - sizeof(alimsrv_sAttrSupList) 
	+ op->attr_c * sizeof(alimsrv_sAttrSupList)
	+ (op->sup_c - op->attr_c) * sizeof(alimsrv_sSupObjEntry);

  bp = (alimsrv_sSupDataBuf *)calloc(1, *size);
  if (bp == NULL)
    return NULL;
 
  bp->NoOfSupAttr = op->attr_c;
  bap = bp->AttrSupList;

  for (al = LstFir(&op->attr_l); al != LstEnd(&op->attr_l); al = LstNex(al)) {
    ap = LstObj(al);
    bap->SupAttr = ap->aref;
    bap->NoOfSupObjs = ap->sup_c;
    bap->SupClass = ap->cid;
    bsp = bap->SupList;

    for (sl = LstFir(&ap->sup_l); sl != LstEnd(&ap->sup_l); sl = LstNex(sl)) {
      sp = LstObj(sl);      
      bsp->SupObjid = sp->oid;
      if (ap->cid == pwr_cClass_ASup) {
	pwr_sClass_ASup *asup = (pwr_sClass_ASup *) sp->p;

	bsp->SupData.A.EvType  = asup->EventType;
	bsp->SupData.A.EvPrio  = asup->EventPriority;
	bsp->SupData.A.High    = asup->High;
	bsp->SupData.A.Limit   = asup->CtrlLimit;
	bsp->SupData.A.Hyst    = asup->Hysteres;
	bsp->SupData.A.MsgOn   = asup->DetectOn;
      } else if (ap->cid == pwr_cClass_DSup) {
	pwr_sClass_DSup *dsup = (pwr_sClass_DSup *) sp->p;

	bsp->SupData.D.EvType  = dsup->EventType;
	bsp->SupData.D.EvPrio  = dsup->EventPriority;
	bsp->SupData.D.CtrlPos = dsup->CtrlPosition;
      }
      bsp++;
    } 
    bap = (void *)bsp;
  }

  return bp;
}

static sAttribute *
findAttribute (
  sObject	*op,
  pwr_sAttrRef	*aref,
  pwr_tClassId	cid
)
{
  LstLink(sAttribute) *al;
  sAttribute *ap = NULL;

  for (al = LstFir(&op->attr_l); al != LstEnd(&op->attr_l); al = LstNex(al)) {
    if (aref->Offset == LstObj(al)->aref.Offset) {
      ap = LstObj(al);
      break;
    }
  }

  if (ap == NULL) {
    ap = (sAttribute *)calloc(1, sizeof(*ap));
    if (ap == NULL) exit(2);

    LstIni(&ap->attr_l);
    LstIni(&ap->sup_l);
    (void)LstIns(&op->attr_l, ap, attr_l);
    op->attr_c++;
    ap->cid = cid;
    ap->aref = *aref;
  }
  return ap;
}

static void
init (
)
{
  pwr_sClass_ASup *asp;
  pwr_sClass_DSup *dsp; 
  pwr_tStatus sts;
  sAttribute *ap;
  sObject *op;
  sSupObject *sp;
  pwr_tObjid oid;

  ltp = tree_CreateTable(&sts, sizeof(pwr_tObjid), offsetof(sObject, oid),
    sizeof(sObject), 200, tree_Comp_oid);

  sts = gdh_GetClassList(pwr_cClass_ASup, &oid);
  while (ODD(sts)) {
    sts = gdh_ObjidToPointer(oid, (void **)&asp);
    op = tree_Find(&sts, ltp, &asp->Attribute.Objid);
    if (op == NULL) {
      op = tree_Insert(&sts, ltp, &asp->Attribute.Objid);
      LstIni(&op->attr_l);
    }

    ap = findAttribute(op, &asp->Attribute, pwr_cClass_ASup);
    
    sp = calloc(1, sizeof(*sp));
    (void)LstIns(LstEnd(&ap->sup_l), sp, sup_l); 
    sp->oid = oid;
    sp->p = asp;
    op->sup_c++;
    ap->sup_c++;

    sts = gdh_GetNextObject(oid, &oid);
  }

  sts = gdh_GetClassList(pwr_cClass_DSup, &oid);
  while (ODD(sts)) {
    sts = gdh_ObjidToPointer(oid, (void **)&dsp);
    op = tree_Find(&sts, ltp, &dsp->Attribute.Objid);
    if (op == NULL) {
      op = tree_Insert(&sts, ltp, &dsp->Attribute.Objid);
      LstIni(&op->attr_l);
    }

    ap = findAttribute(op, &dsp->Attribute, pwr_cClass_DSup);
    
    sp = calloc(1, sizeof(*sp));
    (void)LstIns(LstEnd(&ap->sup_l), sp, sup_l); 
    sp->oid = oid;
    sp->p = dsp;
    op->sup_c++;
    ap->sup_c++;

    sts = gdh_GetNextObject(oid, &oid);
  }

}

int main (int argc, char **argv)
{
  pwr_tStatus      sts;
  unsigned int     size;
  qcom_sQid        myQId;
  alimsrv_sSupDataBuf *bp;
  qcom_sGet        get;
  qcom_sPut        put;
  alimsrv_sRequest request;
  XDR              xdrs;
  qcom_sQattr      qAttr;

  errh_Init("pwr_alim", errh_eAnix_alim);
  errh_SetStatus( PWR__SRVSTARTUP);

  if (!qcom_Init(&sts, 0, "pwr_alim")) {
    errh_Fatal("qcom_Init, %m", sts);
    exit(-1);
  } 

  myQId.qix = alimsrv_cQix;
  myQId.nid = 0;
  qAttr.type  = qcom_eQtype_private;
  qAttr.quota = 100;
  if (!qcom_CreateQ(&sts, &myQId,  &qAttr, "pwr_alim")) {
    errh_Fatal("qcom_CreateQ, %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(-1);
  } 
  if (!qcom_Bind(&sts, &myQId, &qcom_cQini)) {
    errh_Fatal("qcom_Bind, %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(-1);
  }

  sts = gdh_Init("pwr_alim");
  if (EVEN(sts)) {
    errh_Fatal("gdh_Init, %m", sts);
    errh_SetStatus( PWR__SRVTERM);
    exit(-1);
  } 

  init();

  errh_SetStatus( PWR__SRUN);

  /* Loop forever and receive objid's */

  for (;;) {
    do {
      get.maxSize = sizeof(request);
      get.data = (char *)&request;
      qcom_Get(&sts, &myQId, &get, qcom_cTmoEternal);
      if (sts != QCOM__TMO && sts != QCOM__QEMPTY) {
        if (get.type.b == qcom_eBtype_event) {
          event(&get);
        }
      }
    } while (ODD(sts) && get.type.s != alimsrv_eSubType_Request && get.type.b != alimsrv_cMsgType);

    if (EVEN(sts)) {
      errh_Error("qcom_Get, %m",sts);
      continue;
    }

    if (request.Xdr) {
      xdrmem_create(&xdrs, (char *)&request, sizeof(request), XDR_DECODE);
      if (!xdr_alimsrv_sRequest(&xdrs, &request)) {
	errh_Error("XDR Decode failed");
	continue;
      }
    }

    bp = buildBuffer(&request, &size);

    if (ODD(sts)) {
      bp->Xdr = TRUE;
      xdrmem_create(&xdrs, (char *)bp, size, XDR_ENCODE);
      if (!xdr_alimsrv_sSupDataBuf(&xdrs, bp)) {
	errh_Error("XDR Encode failed");
      } else {
	put.type.b = alimsrv_cMsgType;
	put.type.s = alimsrv_eSubType_Answer;
	put.reply = myQId;
	put.data = (char *) bp;
	put.size = size;

	if (!qcom_Reply(&sts, &get, &put))
	  errh_Error("qcom_Respond, %m", sts);
      }            

      free(bp);
    }
  }    
}

static void
event (
  qcom_sGet	*get
)
{
  qcom_sEvent  *ep = (qcom_sEvent*) get->data;
  ini_mEvent            new_event;

  if (get->type.s != qcom_cIini)
    return;

  new_event.m = ep->mask;
  if (new_event.b.terminate) {
    exit(0);
  }
}
