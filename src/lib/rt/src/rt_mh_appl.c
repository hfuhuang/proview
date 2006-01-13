/* 
 * Proview   $Id: rt_mh_appl.c,v 1.5 2006-01-13 16:32:39 claes Exp $
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

/* rt_mh_appl.c -- Runtime environment - Message Handler */

#ifdef OS_ELN
# include stdio
# include ssdef
#else
# include <stdio.h>
# include <string.h>
#endif

#ifdef OS_VMS
# include <starlet.h>
# include <ssdef.h>
#endif

#include "pwr_class.h"
#include "co_time.h"
#include "rt_mh_def.h"
#include "rt_mh_net.h"
#include "rt_mh_appl.h"
#include "rt_qcom_msg.h"
#include "rt_mh_util.h"
#include "rt_errh.h"

//! Application context.
typedef struct {
  mh_sHead	head;
  qcom_sQid	handler;
  mh_eApplState	state;
} sApplContext;

/* Local variables */

static sApplContext lAppl;

/* Local function prototypes */

static pwr_tStatus sendAndReceive(mh_eMsg, pwr_tUInt32, void*, mh_uApplReply*);

/** 
 *@brief Cancel a message previously created by this application.  
 *
 * This call can be used to cancel a message before the cause of it 
 * has disappeared. It will be handled as if it were a return message with 
 * the exception that the event list will show it to be a cancel message instead. 
 * @return pwr_tStatus
 */
pwr_tStatus
mh_ApplCancel (
  pwr_tUInt32 id,             /**< The identity of the message to cancel.  */
  pwr_tString80 *cancelText   /**< A string that will be associated with the cancel event.*/ 
)
{
  pwr_tStatus sts;
  mh_sApplReturn cancel;

  /* Check parameters */

  if (cancelText == NULL)
    memset(&cancel.ReturnText, 0, sizeof(cancel.ReturnText));
  else
    strncpy(cancel.ReturnText, (char *)cancelText, sizeof(*cancelText));

  cancel.TargetIdx = id;
  cancel.ReturnType = mh_eEvent_Cancel;

  sts = sendAndReceive(mh_eMsg_ApplReturn, sizeof(cancel), &cancel, NULL);

  return sts;
}



 /*
 *@example mh_examples.c
 */

/**
 *@brief Connects this application to the local Handler.  
 *
 *To be able to generate messages the application must first call mh_ApplConnect.  
 *See mh_examples.c
 *@return pwr_tStatus
 */

pwr_tStatus
mh_ApplConnect (
  pwr_tObjid		applObject, /**< Supplies the object id of the $appl-object for 
                                         the application, or if no object exists, a value in 
                                        the range [1000..2000]. This argument is used to identify 
                                        the application and must be different for all application 
                                        programs in the system, otherwise complete chaos can be the 
                                        result. This identity may also be used to identify the origin 
                                        of the messages. In this way a restarted application can find 
                                        out if it has any active messages. */ 
  mh_mApplFlags		flags,          /**< see mh_mApplFlags N.B.! Ignored for this release  */
  pwr_tString80		abortEventName, /**< If the application terminates abnormally, an alarm message 
                                             with this text as the event name will be generated.<br>  
                                             N.B.! Ignored for this release  */
  mh_eEvent		abortEventType, /**< ZZZ */
  mh_eEventPrio		abortEventPrio, /**< If the application terminates abnormally, an alarm message
                                             with this priority will be generated.<br>
                                             N.B.! Ignored for this release  */
  mh_mEventFlags	abortEventFlags,/**< If the application terminates abnormally, an alarm message 
                                             with this priority will be generated.<br>
                                             N.B.! Ignored for this release  */ 
  pwr_tString80		abortEventText, /**< If the application terminates abnormally, an alarm message 
                                             with this arbitrary text as the event text will be 
                                             generated.<br> 
                                             N.B.! Ignored for this release  */
  pwr_tUInt32		*activeMessages /**< When restarting an application there might exist active 
                                             messages originating from it, this parameter returns the 
                                             number of such messages. Use mh_ApplGetMsgInfo to acquire 
                                             more information about these. */ 
)
{
  pwr_tStatus		sts;
  pwr_tClassId		cid;
  mh_sApplConnect	connect;
  mh_uApplReply		reply;
  qcom_sQid		qid = qcom_cNQid;
  char			name[32];
  qcom_sNode		myNode;
  qcom_sQattr		qAttr;

  mh_UtilWaitForMh();
#if 0
  if (lAppl.State == mh_eApplState_Connected)
    return MH__ALLRCON;
#endif          

  sprintf(name, "MhAppl_%X", applObject.oix);

  if (!qcom_Init(&sts, NULL, name)) {
    errh_Error("Failed connecting application to QCOM\n%m", sts);
    return MH__QCOMCONAPPL;
  } 

  qAttr.type = qcom_eQtype_private;
  qAttr.quota = 100;
  if (!qcom_CreateQ(&sts, &qid, &qAttr, name)) {
    errh_Error("Failed to create QCOM que\n%m", sts);
    return MH__QCOMCREQ;
  } 

  qcom_MyNode(&sts, &myNode);
  lAppl.head.platform.os = myNode.os;
  lAppl.head.platform.hw = myNode.hw;

  /* Check Application object */

  if (applObject.vid != pwr_cNVolumeId) { /* Application object should exist in Rtdb */
    sts = gdh_GetObjectClass(applObject, &cid);
    if (EVEN(sts)) return sts;
    switch (cid) {
    case pwr_eClass_Appl:
    case pwr_cClass_OpAppl:
    case pwr_cClass_Application:
      break;
    default:
      return MH__NOAPPL;
    }
  }

  lAppl.head.xdr = FALSE; /* To local handler only */
  lAppl.head.ver = mh_cVersion;
  lAppl.head.source = mh_eSource_Application;
  clock_gettime(CLOCK_REALTIME, &lAppl.head.birthTime);
  lAppl.head.qid = qid;
  lAppl.head.aid = applObject;
  lAppl.handler.nid = qid.nid;
  lAppl.handler.qix = mh_cProcHandler;

  connect.ApplObject = applObject;
  connect.Qid = qid;
  connect.Flags = flags;
  strncpy(connect.AbortEventName, abortEventName, sizeof(connect.AbortEventName));    
  connect.AbortEventType = abortEventType;
  connect.AbortEventPrio = abortEventPrio;
  connect.AbortEventFlags = abortEventFlags;
  strncpy(connect.AbortEventText, abortEventText, sizeof(connect.AbortEventText));    

  sts = sendAndReceive(mh_eMsg_ApplConnect, sizeof(connect), &connect, &reply);
  if (EVEN(sts)) return sts;

  if (activeMessages != NULL)
    *activeMessages = reply.Connect.NoOfActMessages;

  lAppl.state = mh_eApplState_Connected;
  return sts;
}

/** 
 * @brief  Informs the local Handler to remove this Application
 * from its list of known applications.  
 *
 * This routine must be called before an application attached to mh exits. 
 * All active messages originating from the application will be cancelled. 
 * @return pwr_tStatus
 */
pwr_tStatus
mh_ApplDisconnect ()
{
  pwr_tStatus sts;

  sts = sendAndReceive(mh_eMsg_ApplDisconnect, 0, NULL, NULL);

  lAppl.state = mh_eApplState_Disconnected;

  return MH__SUCCESS;
}

/** 
 * @brief  This routine is called by an application to create 
 * and send a message. 
 *
 * see mh_examples.c 
 * @return pwr_tStatus
 */

pwr_tStatus
mh_ApplMessage (
  pwr_tUInt32      *id,    /**< The identity of the message created. 
                                The application must save this id to be able to send 
                                return and acknowledgement messages. */
  mh_sApplMessage *message /**< A structure containing the properties of the message. 
                                The application must fill this in before calling mh_ApplMessage, 
                                See mhs_ApplMessage for the details. */
)
{
  pwr_tStatus sts;
  mh_uApplReply reply;

  /* Check parameters */

  if (message == NULL)
    return MH__BADPARAM;

  sts = sendAndReceive(mh_eMsg_ApplMessage, sizeof(*message), message, &reply);
  if (EVEN(sts)) return sts;

  message->Id = reply.Message.Idx;
  if (id != NULL)
    *id = reply.Message.Idx;
  return sts;
}

/** 
 * @brief Set return status on a message previously
 * created by this application. 
 *
 * @return pwr_tStatus
 */

pwr_tStatus
mh_ApplReturn (
  pwr_tUInt32 id,            /**< The identity of the message, to which the return message applies. */
  pwr_tString80 *returnText  /**< The text string appearing on the second row in the alarm window. */
)
{
  pwr_tStatus sts;
  mh_sApplReturn  applReturn;

  /* Check parameters */

  if (returnText == NULL)
    memset(&applReturn.ReturnText, 0, sizeof(applReturn.ReturnText));
  else
    strncpy(applReturn.ReturnText, (char *)returnText, sizeof(*returnText));

  applReturn.TargetIdx  = id;
  applReturn.ReturnType = mh_eEvent_Return;

  sts = sendAndReceive(mh_eMsg_ApplReturn, sizeof(applReturn), &applReturn, NULL);

  return sts;
  }

/**
 * @brief Fetches the properties of an earlier sent message. 
 *
 * see mh_examples.c
 * @return pwr_tStatus
 */
pwr_tStatus
mh_ApplGetMsgInfo (
  pwr_tUInt32     id,          /**< The identity of the message, about which you want information. 
                                    If you supply an identity not known, data for the message with 
                                    the closest (larger) identity will be returned. This means that 
                                    if you supply 0, the data for the oldest still active message will 
                                    be returned. The identity of the message will be found in Message.id.*/
  mh_sApplMessage *message     /**< The requested information will be returned here. See mhs_ApplMessage, 
                                    If no message is found, the contents of the structure will be undefined. */
)
{
  pwr_tStatus	  sts;
  mh_uApplReply   reply;

  /* Check parameters */

  if (message == NULL) return MH__BADPARAM;

  sts = sendAndReceive(mh_eMsg_ApplGetMsgInfo, sizeof(id), &id, &reply);
  if (EVEN(sts)) return sts;

  memcpy(message, &reply.Info.Message, sizeof(*message));

  return MH__SUCCESS;
}


static pwr_tStatus
sendAndReceive (
  mh_eMsg	type,
  pwr_tUInt32	inSize,
  void		*ip,
  mh_uApplReply *reply
)
{
  pwr_tStatus   sts;
  qcom_sPut     put;
  qcom_sGet     get;
  mh_sHead      *hp;
  mh_uApplReply *rp;
  void*         rvoid;
  pwr_tUInt32	size = sizeof(lAppl.head) + inSize;

  if ((put.data = qcom_Alloc(&sts, size)) == NULL) {
    errh_Error("mh_Appl, sendAndReceive, qcom_AllocMsg failed\n%m", sts);
    return MH__QCOMALLOCMSG;
  }

  memcpy(put.data, &lAppl.head, sizeof(lAppl.head));
  ((mh_sHead*) put.data)->type = type;
  if (inSize > 0) memcpy((char*)put.data + sizeof(lAppl.head), ip, inSize);

  put.type.b	= mh_cMsgClass;
  put.type.s	= 0;
  put.reply.qix = 0;
  put.reply.nid = 0;
  put.size	= size;

  get.data = NULL;

  rvoid = qcom_Request(&sts, &lAppl.handler, &put, &lAppl.head.qid, &get, 1000*3600);
  if (EVEN(sts)) {
    errh_Error("mhAppl, sendAndReceive, qcom_Get failed\n%m", sts);
    qcom_Free(NULL, put.data);
    return MH__QCOMRCVMSG;
  }

  hp = (mh_sHead *)get.data; 
  rp = (mh_uApplReply*) (hp + 1);
  sts = rp->Sts;

  if (ODD(sts) && reply != NULL)
    memcpy(reply, rp, sizeof(mh_uApplReply));

  qcom_Free(NULL, get.data);

  return sts;
}
