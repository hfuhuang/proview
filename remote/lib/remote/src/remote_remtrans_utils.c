/* 
 * Proview   $Id: remote_remtrans_utils.c,v 1.1 2006-01-12 06:39:33 claes Exp $
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

/*********************************************************************
**********************************************************************
*
* Filename :	remtrans_utils.c
*
* Modified :	Date	Pgm		Remark
*		950208	Hans Werner	First version
*		950208	CS		Converted from h to c file
*		960222	Hans Werner	Added RemTrans_Logg
*		010330	CJu		RemTrans_Logg to qcom
*		040506	CJu		Changes to V4.0.0
*
* Description :	Remote trans utilities used by all remote transports
*
**********************************************************************
*********************************************************************/

#include "pwr.h"
#include "pwr_class.h"
#include "co_time.h"
#include "co_cdh.h"
#include "rt_errh.h"
#include "rt_gdh.h"
#include "rt_qcom.h"
#include "rt_qcom_msg.h"
#include "rt_qdb_msg.h"
#include "pwr_remoteclasses.h"
#include "remote.h"
#include "remote_remtrans_utils.h"
#include "rs_remote_msg.h"

/* Declare common variables */

qcom_sQid logg_qid;


/*********************************************************************
**********************************************************************
*
* Name :        RemTrans_Logg
*
* Type :        void
*
* Input :       Pointer to struct remtrans_item with objid and pointer
*
* Output :      QCOM logg-message to RS_REMOTE_LOGG.
* Description : Check if logging is to be done. Put log message together
*		and send to queue RS_PWR_LOGG
*
**********************************************************************
*********************************************************************/

void RemTrans_Logg(remtrans_item *remtrans)
{
  pwr_sClass_RemTrans	*RemTransP;
  pwr_tStatus   	sts;
  pwr_tString80		rad1;		/* Object name */
  pwr_tString80		rad2;		/* Time, status, length */
  char 			*buffp;		/* Pointer inside send buffer */
  char 			*datap;		/* Pointer to transdata buffer */
  unsigned int	 	i;		/* Loop index */
  unsigned int		sizrad1;	/* Buffer size row 1 */
  unsigned int		sizrad2;	/* Buffer size row 2 */
  unsigned int		AntByt;		/* Number of databytes to logg */
  unsigned int		AntRad;		/* Number of lines data in logg */
  unsigned int		sign12;
  unsigned int		sign1;
  unsigned int		sign2;
  unsigned int		maxrad;		/* Number of databytes in line */
  unsigned int		size;		/* Total size of send buffer */
  static unsigned short qcom_initialized = 0;
  char*			dynp;		/* Allocated dynamic data area */
  qcom_sPut		put;

/* Shall we make a logg-entry ? */
  RemTransP = remtrans->objp;
  if ((RemTransP->LoggLevel > 1) || ((RemTransP->LoggLevel == 1) &&
	EVEN(RemTransP->LastSts)))
  {

/* FIRST ROW: Get Object name */
    sts = gdh_ObjidToName(remtrans->objid, (char *) &rad1, sizeof(rad1), cdh_mNName);
    if (EVEN(sts)) return;
    sizrad1 = strlen((char *) &rad1);

/* SECOND ROW: Get Date and Time */
    time_AtoAscii( NULL, time_eFormat_ComprDateAndTime, rad2, sizeof(rad2));
    sizrad2 = strlen(rad2);
    datap = (char *) &rad2 + sizrad2;
/* Status and Buffer Length */
    sizrad2 += sprintf(datap,"  Status %d  Length %d",
	RemTransP->LastSts, RemTransP->DataLength);

/* Get size of data message to be logged */
    if (RemTransP->LoggLevel == 4) AntByt = RemTransP->DataLength;
    else if (RemTransP->LoggLevel == 3) AntByt =
	(RemTransP->DataLength > 48) ? 48 : RemTransP->DataLength;
    else AntByt = 0;
    AntByt = min(AntByt, 10000);
    AntRad = (AntByt + 23) / 24;
    size = sizrad1 + sizrad2 + 9 + 3 * AntByt + AntRad;

    if (qcom_initialized == 0)
    {
/* Try to initiate qcom (it's probably already done) */
      logg_qid = qcom_cNQid;
      logg_qid.qix = rs_pwr_logg_qix;
      qcom_Init(&sts, 0, "Logg");
      if (EVEN(sts) && sts != QDB__ALRMAP) {
        errh_Info("Qcom init failed, status: %d\n", sts);
        return;
      }
      qcom_initialized = 1;
    }

/* Malloc dynamic data area */

    dynp = 0;
    dynp = malloc(size+1);
    if (!dynp) return;
    buffp = dynp;

/* Get rad1 and rad2 into sendbuffer */
    *buffp++ = RS_PWR_LOGG_RemTransId;
    *buffp++ = 0;
    *buffp++ = 0;
    *buffp++ = 0;
    memcpy(buffp, &rad1, sizrad1);
    buffp += sizrad1;
    *buffp = 13; /* CR */
    buffp++;
    *buffp = 10; /* LF */
    buffp++;

    memcpy(buffp, &rad2, sizrad2);
    buffp += sizrad2;
    *buffp = 13; /* CR */
    buffp++;
    *buffp = 10; /* LF */
    buffp++;

/* Logg of Trans Data */
    datap = remtrans->datap;
    while (AntByt > 0)
    {
      maxrad = AntByt;
      if (AntByt > 24) maxrad = 24;
      for (i = 0; i < maxrad; i++)
      {
        sign12 = *datap;
        sign1 = (sign12 & 240) / 16;		/* Most sign 4 bits */
        sign2 = sign12 & 15;			/* Least sign 4 bits */
        datap++;
        if (sign1 < 10) *buffp = sign1 + 48;	/* 0 - 9 */
        else *buffp = sign1 + 55;		/* A - F */
        buffp++;
        if (sign2 < 10) *buffp = sign2 + 48;	/* 0 - 9 */
        else *buffp = sign2 + 55;		/* A - F */
        buffp++;
        *buffp = 32;				/* Space */
        buffp++;
      } /* EndLoop bytes in row */
      buffp--;
      *buffp = 13;				/* CR */
      buffp++;
      *buffp = 10;				/* LF */
      buffp++;
      AntByt -= maxrad;
    } /* Endloop lines */
    *buffp = 0;					/* NULL as end sign */

/* Send to Logg-job */
    put.data = dynp;
    put.size = size;
    put.type.b = qcom_eBtype__;
    put.type.s = qcom_eStype__;
    put.reply.qix = 0;
    put.reply.nid = 0;

    qcom_Put(&sts, &logg_qid, &put);

    free(dynp);

  } /* END Logging */
return;
}

/*********************************************************************
**********************************************************************
*
* Name :	RemTrans_Init
*
* Type :	pwr_tInt32
*
* Input :	Pointer to struct remnode_item.
*
* Output :	Linked list of remtrans_items
* Description :	Initialize RemTrans-object
*
**********************************************************************
*********************************************************************/

pwr_tInt32 RemTrans_Init(remnode_item *remnode)
{
  remtrans_item	*remtrans;
  pwr_tStatus	sts;
  pwr_tObjid	child_objid,data_objid;
  pwr_tClassId	class;
  int		mc_found;

  /****** Initialize data in RemNode ******/
  remnode->remtrans = NULL;
  remnode->transbuff = NULL;
  remnode->multicast = NULL;
  mc_found = 0;

  /****** Get first child under RemNode ******/
  sts = gdh_GetChild(remnode->objid, &child_objid);
  while ( ODD(sts)) {

    /****** Test if type remtrans ******/
    sts = gdh_GetObjectClass(child_objid, &class);
    if (class == pwr_cClass_RemTrans) {

      /****** Create remtrans_item and link it ******/
      remtrans = malloc(sizeof(remtrans_item));
      if (remtrans == 0) exit(REM__NOMEMORY);			/* Error exit */
      remtrans->next = (struct remtrans_item *) remnode->remtrans;
      remnode->remtrans = remtrans;

      remtrans->objid = child_objid;
      sts = gdh_ObjidToPointer(child_objid, (pwr_tAddress *) &remtrans->objp);
      remtrans->buffp = NULL;

      /****** Clear data i RemTrans ******/
      remtrans->objp->Buffers = 0;
      remtrans->objp->LastSts = 1;
      remtrans->objp->TransCount = 0;
      remtrans->objp->BuffCount = 0;
      remtrans->objp->LostCount = 0;
      remtrans->objp->ErrCount = 0;
      remtrans->time_since_send = 0;

      /****** Check data object under RemTrans ******/
      sts = gdh_GetChild(child_objid, &data_objid);
      if ( EVEN(sts)) {
        remtrans->objp->MaxLength = 0;
        remtrans->datap = 0;
      }		/* END No Data */
      else {
        sts = gdh_ObjidToPointer(data_objid, (pwr_tAddress *) &remtrans->datap);
        sts = gdh_GetObjectSize(data_objid, 
		(unsigned int *)&remtrans->objp->MaxLength);
      }		/* END Data found */
    }		/* END Class RemTrans */
    
    /* If child is (first) MultiCast object, store pointer to this object in remnode_item */
    if (class == pwr_cClass_MultiCast && mc_found == 0) {
      mc_found = 1;
      sts = gdh_ObjidToPointer(child_objid, (pwr_tAddress *) &remnode->multicast);
    }    

    /****** Get next child under RemNode ******/
    sts = gdh_GetNextSibling(child_objid, &child_objid);
  }		/* END while */
return STATUS_OK;
}		/* END RemTrans_Init */

/*********************************************************************
**********************************************************************
*
* Name :	RemTrans_Cyclic
*
* Type :	pwr_tInt32
*
* Input :	Pointer to struct remnode_item
*
* Description :	Check if there are any free buffers to treat or
*		any transes to send.
*
**********************************************************************
*********************************************************************/

pwr_tInt32 RemTrans_Cyclic(	remnode_item	*remnode, 
				unsigned int (* remnode_send)
					(remnode_item *remnode,
			  	 	pwr_sClass_RemTrans *remtrans,
			  	 	char *buf,
			  	 	int buf_size)
)

{
  pwr_tStatus		sts;
  remtrans_item		*remtrans;
  pwr_sClass_RemTrans	*RemTransP;
  rem_t_transbuff	*buffp,*nextp;
  remtrans_item 	*transp;
  pwr_tInt32		buffsize;
  pwr_tStatus		SendSts;

  /* Test if there is a buffered trans */
  if (remnode->transbuff) {
    buffp = (rem_t_transbuff *) remnode->transbuff;
    transp = (remtrans_item *) buffp->remtrans;
    /* Trans is not to be sent before time-out if buffered in wait for ack */
    if ((transp->time_since_send >= remnode->retransmit_time) ||
	(buffp->ackbuf == 0)) {
      sts = (remnode_send) (remnode, transp->objp, &buffp->data, buffp->size);
      transp->time_since_send = 0;
      if (ODD(sts)) {
        remnode->transbuff = (rem_t_transbuff *) buffp->next;
        transp->objp->Buffers--;
        free(buffp);
      }
    }
  }

  /* Loop all remtrans under remnode */
  remtrans = remnode->remtrans;
  while (remtrans) {
    RemTransP = remtrans->objp;

    /* Check if buffered receive */
    if (RemTransP->Direction == REMTRANS_IN && RemTransP->Buffers > 0 &&
	!RemTransP->DataValid) {
      buffp = (rem_t_transbuff *) remtrans->buffp;
      buffsize = buffp->size;		/* buffersize */
      remtrans->buffp = buffp->next;	/* next buffer */

      memcpy(remtrans->datap,&buffp->data,buffsize);	/* copy transdata */
      RemTransP->Buffers--;
      RemTransP->DataLength = buffp->size;
      RemTransP->LastSts = STATUS_BUFF;
      RemTrans_Logg(remtrans);		/* Logg */
      RemTransP->DataValid = TRUE;

      free(buffp);			/* Dispose of buffer */
    }	/* END buffered receive */
    /* Treat send trans */
    else if (RemTransP->Direction == REMTRANS_OUT) {

      /* Is there new data to send ? */
      if (RemTransP->DataValid) {
        if (RemTransP->DataLength > RemTransP->MaxLength ){
          RemTransP->ErrCount++;
          RemTransP->LastSts = STATUS_LENGTH;
        }	/* END too long data */
        else {
          SendSts = 0;	/* Assume that we could not send */
          if ((RemTransP->MaxBuffers == 0) ||
		!remnode->transbuff) {	/* Try to send if no buffers */
            SendSts = (remnode_send) (remnode,RemTransP,
		remtrans->datap,RemTransP->DataLength);
	    remtrans->time_since_send  = 0;
            if (ODD(SendSts)) {
              RemTransP->TransCount++;
	      clock_gettime( CLOCK_REALTIME, &RemTransP->TransTime);
              RemTransP->LastSts = STATUS_OK;
            }	/* END Send OK */
          }	/* END Try to send */
          if (EVEN(SendSts)) {			/* Trans should be buffered */
            if ((RemTransP->MaxBuffers > 0) &&
		(RemTransP->Buffers < RemTransP->MaxBuffers)) {
              buffp = malloc(RemTransP->DataLength + 
				sizeof(rem_t_transbuff) - 1);
              if (buffp) {
                RemTransP->Buffers++;		/* Buffer counter */
                RemTransP->BuffCount++;		/* Buffered trans counter */
                buffp->next = 0;
                buffp->remtrans = remtrans;
                buffp->size = RemTransP->DataLength;
                buffp->ackbuf = (SendSts == STATUS_BUFACK) ? 1 : 0;
	        memcpy(&buffp->data, remtrans->datap, RemTransP->DataLength);
                if (remnode->transbuff == 0) remnode->transbuff = buffp;
                else {		/* Put buffer last in queue */
                  nextp = (rem_t_transbuff *) remnode->transbuff;
                  while (nextp->next) nextp = (rem_t_transbuff *) nextp->next;
                  nextp->next = (struct rem_t_transbuff *) buffp;
                }	/* END New buffer last in queue */
                RemTransP->TransCount++;
	        clock_gettime( CLOCK_REALTIME, &RemTransP->TransTime);
                RemTransP->LastSts = STATUS_BUFF;
              }		/* END  Create new bufer */
              else {
                RemTransP->ErrCount++;
                if (EVEN(sts)) RemTransP->LastSts = sts;
                else RemTransP->LastSts = STATUS_FELSEND;
              }		/* END No memory for buffer */
            }		/* END Buffering wanted */
            else {
              RemTransP->ErrCount++;
              RemTransP->LastSts = STATUS_FELSEND;
            }		/* END No more buffers */
          }		/* END trans is not sent */
        }		/* END Send request for valid trans */
        RemTrans_Logg(remtrans);		/* Logg */
        RemTransP->DataValid = FALSE;		/* Trans is treated */
      }			/* END Data valid was set */
    }			/* END Send direction */
    remtrans = (remtrans_item *) remtrans->next;
  }	/* END while */

return STATUS_OK;
}

/*********************************************************************
**********************************************************************
*
* Name :	RemTrans_Receive
*
* Type :	pwr_tInt32
*
* Input :	Pointer to struct remtrans_item,
*		Pointer to buffer and size of buffer
*
* Description :	Treat received trans that belongs to this remtrans_item.
*
**********************************************************************
*********************************************************************/

pwr_tInt32 RemTrans_Receive(	remtrans_item	*remtrans,
				char		*buffer,
				int		size)

{
  pwr_sClass_RemTrans	*RemTransP;
  rem_t_transbuff	*buffp,*nextp;

  RemTransP = remtrans->objp;		/* Get remtrans object */
  RemTransP->TransCount++;
  clock_gettime( CLOCK_REALTIME, &RemTransP->TransTime);

  if ((unsigned int) size > RemTransP->MaxLength) {	/* Too big trans */
    RemTransP->ErrCount++;
    RemTransP->LastSts = STATUS_LENGTH;
    RemTrans_Logg(remtrans);		/* Logg */
    return STATUS_LENGTH;
  }		/* END Too big trans */
  else {
    /* First dispose of buffer if last trans is treated */
    if (!RemTransP->DataValid && (RemTransP->Buffers > 0)) {
      buffp = (rem_t_transbuff *) remtrans->buffp;
      remtrans->buffp = buffp->next;

      memcpy(remtrans->datap,&buffp->data,buffp->size);	/* copy transdata */
      RemTransP->Buffers--;
      RemTransP->DataLength = buffp->size;
      RemTransP->LastSts = STATUS_BUFF;
      RemTransP->DataValid = TRUE;

      free(buffp);			/* Dispose of buffer */
    }	/* END buffered receive */
    if (!RemTransP->DataValid && RemTransP->Buffers == 0) {	/* Store directly */
      memcpy(remtrans->datap,buffer,size);
      RemTransP->DataLength = size;
      RemTransP->LastSts = STATUS_OK;
      RemTrans_Logg(remtrans);		/* Logg */
      RemTransP->DataValid = TRUE;
      return STATUS_OK;
    }		/* END Store trans directly */
    else if ((RemTransP->MaxBuffers > 0) &&
	(RemTransP->Buffers < RemTransP->MaxBuffers)) {
      buffp = malloc(size + sizeof(rem_t_transbuff) - 1);
      if (buffp) {
        RemTransP->Buffers++;		/* Buffer counter */
        RemTransP->BuffCount++;		/* Buffered trans counter */
        buffp->next = 0;
        buffp->size = size;
        memcpy(&buffp->data,buffer,size);
        if (remtrans->buffp == 0) remtrans->buffp = 
			(struct rem_t_transbuff *) buffp;
        else {
          nextp = (rem_t_transbuff *) remtrans->buffp;
          while (nextp->next) nextp = (rem_t_transbuff *) nextp->next;
          nextp->next = (struct rem_t_transbuff *)buffp;
        }	/* END New buffer last in queue */
        return STATUS_BUFF;
      }		/* END  Create new bufer */
      else {
        RemTransP->LostCount++;
        RemTransP->LastSts = STATUS_LOST;
        RemTrans_Logg(remtrans);		/* Logg */
        return STATUS_LOST;
      }		/* END  No memory for buffer*/
    }		/* END Try to buffer */
    else {
      RemTransP->LostCount++;
      RemTransP->LastSts = STATUS_LOST;
      RemTrans_Logg(remtrans);		/* Logg */
      return STATUS_LOST;
    }		/* END Buffering not allowed */
  }		/* END Treat OK trans  */
}
