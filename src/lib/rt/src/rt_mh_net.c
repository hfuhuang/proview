/* 
 * Proview   $Id: rt_mh_net.c,v 1.2 2005-09-01 14:57:56 claes Exp $
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

/* rt_mh_net.c -- MH net routines */

#include "rt_mh_msg.h"
#include "rt_mh_net.h"
#include "rt_qcom.h"
#include "rt_errh.h"

static pwr_tStatus
xdrEvent (
  XDR *xdrs,                  
  mh_eEvent event,               
  mh_sMsgInfo *mp
);              


pwr_tStatus                  
mh_NetSendMessage(           
  qcom_sQid *qid,            
  co_sPlatform *recPlatform,  
  int prio,
  int subtype,               
  mh_sHead *hp,              
  unsigned int size          
) {
  pwr_tStatus sts = MH__SUCCESS;
  pwr_tStatus sts2;
  qcom_sPut   msg;
  XDR xdrs;

  if ((msg.data = (char*)qcom_Alloc(&sts, size)) == NULL)
    return sts;

  msg.type.b = mh_cMsgClass;
  msg.type.s = subtype;
  msg.reply = hp->qid;
  msg.size = size;

  if (recPlatform == NULL || co_IsXdrNeeded(&hp->platform, recPlatform)) {
    hp->xdr = TRUE;
    xdrmem_create(&xdrs, msg.data, size, XDR_ENCODE);
    sts = mh_NetXdrMessage(&xdrs, subtype, hp);
    if (EVEN(sts)) {
      qcom_Free(&sts2, msg.data);
      return sts;
    }
  } else {
    hp->xdr = FALSE;
    memcpy(msg.data, hp, size);
  }


  qcom_Put(&sts, qid, &msg);
  if (EVEN(sts))
    qcom_Free(&sts2, msg.data);

  return sts;
}                           

pwr_tStatus                  
mh_NetXdrMessage(  
  XDR *xdrs,                  
  int subtype,               
  mh_sHead *hp              
) {
  pwr_tStatus sts;


  if (!xdr_mh_sHead(xdrs, hp))  {
    return MH__XDRFAILED;
  }

  switch (hp->type) {    
  case mh_eMsg_Event:
    sts = xdrEvent(xdrs, (mh_eEvent)subtype, (mh_sMsgInfo *)(hp + 1));
    if (EVEN(sts))
      return sts;
    break;
  case mh_eMsg_HandlerDisconnect:
  case mh_eMsg_HandlerSync:
  case mh_eMsg_HandlerHello:
    break;
  case mh_eMsg_OutunitAck:
    if (!xdr_mh_sOutunitAck(xdrs, (mh_sOutunitAck *)(hp + 1))) 
      return MH__XDRFAILED;
    break;
  case mh_eMsg_OutunitBlock:
    if (!xdr_mh_sOutunitBlock(xdrs, (mh_sOutunitBlock *)(hp + 1))) 
      return MH__XDRFAILED;
    break;
  case mh_eMsg_OutunitDisconnect:
  case mh_eMsg_OutunitHello:
    break;
  case mh_eMsg_OutunitInfo:
    if (!xdr_mh_sOutunitInfo(xdrs, hp + 1)) 
      return MH__XDRFAILED;
    break;
  case mh_eMsg_OutunitSync:
    break;
  case mh_eMsg_ProcDown:
    if (!xdr_qcom_sAid(xdrs, (qcom_sAid *)(hp + 1))) 
      return MH__XDRFAILED;
    break;
  case mh_eMsg_Sync:
    break;
  default:
    errh_Info("Unexpected message type: %d, (%s)",
      hp->type, qcom_QidToString(NULL, &hp->qid, 1));
    return MH__XDRUNEXPECT;
    break;
  }

  return MH__SUCCESS;   
}                           

static pwr_tStatus
xdrEvent (
  XDR *xdrs,
  mh_eEvent event,
  mh_sMsgInfo *mp
) {

  switch (event) {
  case mh_eEvent_Ack:
    if (!xdr_mh_sAck(xdrs, (mh_sAck *)mp))
      return MH__XDRFAILED;
    break;

  case mh_eEvent_Block:
  case mh_eEvent_CancelBlock:
  case mh_eEvent_Reblock:
  case mh_eEvent_Unblock:
    if (!xdr_mh_sBlock(xdrs, (mh_sBlock *)mp))
      return MH__XDRFAILED;
    break;

  case mh_eEvent_Return:
  case mh_eEvent_Cancel:
    if (!xdr_mh_sReturn(xdrs, (mh_sReturn *)mp))
      return MH__XDRFAILED;
    break;

  case mh_eEvent_Alarm:
  case mh_eEvent_Info:
    if (!xdr_mh_sMessage(xdrs, (mh_sMessage *)mp))
      return MH__XDRFAILED;
    break;

  /* mh_eEvent_Missing are not sent, only stored by
     rt_eventlogger.  */

  case mh_eEvent__:
  case mh_eEvent_Missing:
  case mh_eEvent_:
  default:
    errh_Info("Unexpected event type: %d", event);
    return MH__XDRUNEXPECT;
    break;
  }
  return MH__SUCCESS;
}
