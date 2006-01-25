/* 
 * Proview   $Id: remote_remio_utils.c,v 1.2 2006-01-13 06:38:27 claes Exp $
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

/*************************************************************************
*
* Filename:             remio_utils.c
*
*                       Date    Pgm.    Read.   Remark
* Modified              
*
* Description:		Remote I/O utilities
*
**************************************************************************/

/*_Include files_________________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "pwr_class.h"
#include "pwr_systemclasses.h"
#include "rt_gdh.h"
#include "co_cdh.h"
#include "pwr_baseclasses.h"
#include "pwr_remoteclasses.h"
#include "rs_remote_msg.h"
#include "remote.h"
#include "remote_remio_utils.h"


/*************************************************************************
**************************************************************************
*
* Name : Functions that converts raw values for AI and AO to actual values
*	 and vice versa.
*
***************************************************************************
**************************************************************************/

pwr_tFloat32 ConvAItoAct(pwr_tInt16 raw, pwr_sClass_RemChan_Ai *body)
{
  pwr_tFloat32	fraw;
  pwr_tFloat32	actval;
  double	dval;

  fraw = raw;

  if (body->SensorPolyType == 1)
    actval = body->SensorPolyCoef0 + body->SensorPolyCoef1*fraw;
  else if (body->SensorPolyType == 2)
    actval = body->SensorPolyCoef0 +
	fraw * (body->SensorPolyCoef1 + body->SensorPolyCoef2 * fraw);
  else if (body->SensorPolyType == 3) {
    dval = body->SensorPolyCoef0 + body->SensorPolyCoef1*fraw;
    if (dval >= 0) actval = body->SensorPolyCoef2 * sqrt(dval);
    else actval = 0;
  }
  else if (body->SensorPolyType == 4) {
    dval = body->SensorPolyCoef0 + body->SensorPolyCoef1*fraw;
    if (dval >= 0) actval = body->SensorPolyCoef2 * sqrt(dval);
    else actval = -body->SensorPolyCoef2 * sqrt(-dval);
  }
  else actval = fraw;

  return actval;
}

/************************************************************/

pwr_tFloat32 ConvAOtoAct(pwr_tInt16 raw, pwr_sClass_RemChan_Ao *body)
{
  pwr_tFloat32 fraw;
  pwr_tFloat32 actval;

  fraw = raw;
  if (body->OutPolyCoef1 != 0.0)
	actval = (fraw - body->OutPolyCoef0)/body->OutPolyCoef1;
  else actval = body->OutMinLimit;
  return actval;
}

/************************************************************/

pwr_tInt16 ConvAOtoRaw(pwr_tFloat32 act, pwr_sClass_RemChan_Ao *body)
{
  pwr_tInt16 raw;
  pwr_tFloat32 actval;

  if (act > body->OutMaxLimit) actval = body->OutMaxLimit;
  else if (act < body->OutMinLimit) actval = body->OutMinLimit;
  else actval = act;

  raw = body->OutPolyCoef0 + body->OutPolyCoef1 * actval;

  return raw;
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Init_ALCM
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to struct remnode_item with objid and objp
*
* Description : Search for all RemChan_xx under rem_node
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Init_ALCM(remnode_item *remnode)
{
  pwr_tObjid		childobjid, chanobjid, sigobjid;
  pwr_tClassId		class;
  pwr_sClass_Di		*dip;
  pwr_sClass_Do		*dop;
  pwr_sClass_Dv		*dvp;
  pwr_sClass_Ai		*aip;
  pwr_sClass_Ao		*aop;
  pwr_sClass_Co		*cop;
  pwr_sClass_RemChan_Di	*chandip;
  pwr_sClass_RemChan_Dv	*chandvp;
  pwr_sClass_RemChan_Do	*chandop;
  pwr_sClass_RemChan_Ai	*chanaip;
  pwr_sClass_RemChan_Ao	*chanaop;
  pwr_sClass_RemChan_Co	*chancop;
  remdi_item		*remdi;
  remdo_item		*remdo;
  remdv_item		*remdv;
  remai_item		*remai;
  remao_item		*remao;
  remco_item		*remco;
  pwr_tStatus		sts;
  pwr_sClass_RemnodeALCM *objp;

  /* Get pointer to remnodeALCM-object */
  sts = gdh_ObjidToPointer(remnode->objid, (pwr_tAddress *) &objp);

  /* Initialize remnode_item and RemNode-object*/
  remnode->remdi = remdi = NULL;
  remnode->remdo = remdo = NULL;
  remnode->remdv = remdv = NULL;
  remnode->remai = remai = NULL;
  remnode->remao = remao = NULL;
  remnode->remco = remco = NULL;
  remnode->remio_data = NULL;

  remnode->maxremdi = objp->NumberOfDI = 0;
  remnode->maxremdo = objp->NumberOfDO = 0;
  remnode->maxremdv = objp->NumberOfDV = 0;
  remnode->maxremai = objp->NumberOfAI = 0;
  remnode->maxremao = objp->NumberOfAO = 0;
  remnode->maxremco = objp->NumberOfCo = 0;
  objp->IOPollDiff = 0;
  objp->IOStallFlag = FALSE;
  remnode->remio_err = FALSE;

  /* Get I/O dataarea for RemIO */
  chanobjid = objp->IODataArea;
  sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &remnode->remio_data);
  if (ODD(sts)) sts = gdh_GetObjectSize(chanobjid, 
		(unsigned int *) &remnode->remio_size);
  else remnode->remio_size = 0;

  /* Get first child to RemNode */
  sts = gdh_GetChild(remnode->objid, &childobjid);
  while (ODD(sts)) {

    /* Test type of object */
    sts = gdh_GetObjectClass(childobjid, &class);
    switch (class) {

      /* Search for RemChanDi with Di-connection */
      case pwr_cClass_RemDi: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Di) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chandip);
            if (ODD(sts)) {
              sigobjid = chandip->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Di) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &dip);
                /* Create new item and link it */
                remdi = malloc(sizeof(remdi_item));
                if (remdi == 0) exit(REM__NOMEMORY);
                remdi->next = (struct remdi_item *) remnode->remdi;
                remnode->remdi = remdi;

                /* Initialize remdi_item */
                objp->NumberOfDI++;
                remdi->objp = chandip;
                remdi->actval = (pwr_tBoolean *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) dip->ActualValue);
              if (class == pwr_cClass_Di)  
                chandip->ActualValue = dip->ActualValue;
                remdi->objp->BuffOff = remdi->objp->ConvOff / 8;
                remdi->objp->ConvMask = 1 << (remdi->objp->ConvOff % 8);
              }	/* END ClassDi */
            }	/* END Object Di */
          }	/* END Class RemChanDi */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanDi under RemDi */
        break;
      }		/* END case RemDi */

      /* Search for RemChanD0 with Do-connection */
      case pwr_cClass_RemDo: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Do) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chandop);
            if (ODD(sts)) {
              sigobjid = chandop->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Do) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &dop);
                /* Create new item and link it */
                remdo = malloc(sizeof(remdo_item));
                if (remdo == 0) exit(REM__NOMEMORY);
                remdo->next = (struct remdo_item *) remnode->remdo;
                remnode->remdo = remdo;

                /* Initialize remdo_item */
                objp->NumberOfDO++;
                remdo->objp = chandop;
                remdo->actval = gdh_TranslateRtdbPointer(
			(pwr_tUInt32) dop->ActualValue);
              if (class == pwr_cClass_Do)   
                chandop->ActualValue = dop->ActualValue;
                remdo->objp->BuffOff = remdo->objp->ConvOff / 8;
                remdo->objp->ConvMask = 1 << (remdo->objp->ConvOff % 8);
                remdo->objp->OldValue = *remdo->actval;
              }	/* END Class Do */
            }	/* END Object Do */
          }	/* END Class RemChanDo */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanDo under RemDo */
        break;
      }		/* END case RemDo */

      /* Search for RemChanDv with Dv-connection */
      case pwr_cClass_RemDv: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Dv) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chandvp);
            if (ODD(sts)) {
              sigobjid = chandvp->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Dv) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &dvp);
                /* Create new item and link it */
                remdv = malloc(sizeof(remdv_item));
                if (remdv == 0) exit(REM__NOMEMORY);
                remdv->next = (struct remdv_item *) remnode->remdv;
                remnode->remdv = remdv;

                /* Initialize Dv, RemChanDv and remdv_item */
                objp->NumberOfDV++;
                remdv->objp = chandvp;
                remdv->actval = gdh_TranslateRtdbPointer(
			(pwr_tUInt32) dvp->ActualValue);
                chandvp->ActualValue = dvp->ActualValue;
                remdv->objp->OldValue = *remdv->actval;
                remdv->objp->BuffOff = remdv->objp->ConvOff / 8;
                remdv->objp->ConvMask = 1 << (remdv->objp->ConvOff % 8);
              }	/* END Class Dv */
            }	/* END Object Dv */
          }	/* END Class RemChanDv */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanDv under RemDv */
        break;
      }		/* END case RemDv */

      /* Search for RemChanAi with Ai-connection */
      case pwr_cClass_RemAi: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Ai) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chanaip);
            if (ODD(sts)) {
              sigobjid = chanaip->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Ai) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &aip);
                /* Create new item and link it */
                remai = malloc(sizeof(remai_item));
                if (remai == 0) exit(REM__NOMEMORY);
                remai->next = (struct remai_item *) remnode->remai;
                remnode->remai = remai;

                /* Initialize Ai, RemChanAi and remai_item */
                objp->NumberOfAI++;
                remai->objp = chanaip;
                remai->actval = (pwr_tFloat32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) aip->ActualValue);
                chanaip->ActualValue = aip->ActualValue;
                remai->rawval = (short int *) &aip->RawValue;
              }	/* END Class Ai */
            }	/* END Object Ai */
          }	/* END Class RemChanAi */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanAi under RemAi */
        break;
      }		/* END case RemAi */

      /* Search for RemChanAo with Ao-connection */
      case pwr_cClass_RemAo: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Ao) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chanaop);
            if (ODD(sts)) {
              sigobjid = chanaop->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Ao) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &aop);
                /* Create new item and link it */
                remao = malloc(sizeof(remao_item));
                if (remao == 0) exit(REM__NOMEMORY);
                remao->next = (struct remao_item *) remnode->remao;
                remnode->remao = remao;

                /* Initialize Ao, RemChanAo and remao_item */
                objp->NumberOfAO++;
                remao->objp = chanaop;
                remao->actval = (pwr_tFloat32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) aop->ActualValue);
                chanaop->ActualValue = aop->ActualValue;
                remao->rawval = (short int *)&aop->RawValue;
                remao->objp->OldValue = aop->RawValue;
              }	/* END Class Ao */
            }	/* END Object Ao */
          }	/* END Class RemChanAo */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanAo under RemAo */
        break;
      }		/* END case RemAo */

      /* Search for RemChanCo with Co-connection */
      case pwr_cClass_RemCo: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Co) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chancop);
            if (ODD(sts)) {
              sigobjid = chancop->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Co) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &cop);
                /* Create new item and link it */
                remco = malloc(sizeof(remco_item));
                if (remco == 0) exit(REM__NOMEMORY);
                remco->next = (struct remco_item *) remnode->remco;
                remnode->remco = remco;

                /* Initialize Co, RemChanCo and remco_item */
                objp->NumberOfCo++;
                remco->objp = chancop;
                remco->extval = (pwr_tInt32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) cop->AbsValue);
                chancop->ExtendedValue = cop->AbsValue;
                remco->actval = (pwr_tInt32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) cop->RawValue);
                chancop->ActualValue = cop->RawValue;
              }	/* END Class Co */
            }	/* END Object Co */
          }	/* END Class RemChanCo */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanCo under RemCo */
        break;
      }		/* END case RemCo */

    }		/* END switch */
    /* Get next child under RemNode */
    sts = gdh_GetNextSibling(childobjid, &childobjid);
  }	/* END while */

  return(1);
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Stall_ALCM
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to struct remnode_item with objid and objp
*
* Description : Action made when remote IO stalls
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Stall_ALCM(remnode_item *remnode)
{
  remdi_item	*remdi;
  remdo_item	*remdo;
  remdv_item	*remdv;
  remai_item	*remai;
  remao_item	*remao;

  pwr_sClass_RemnodeALCM *objp;
  pwr_tStatus sts;

  /* Get pointer to remnodeALCM-object */
  sts = gdh_ObjidToPointer(remnode->objid, (pwr_tAddress *) &objp);
  
  /* Set StallFlag */
  if ( !objp->IOStallFlag) {
    objp->IOStallFlag = TRUE;

    /* Clear Remote signals according to IOStallAction */
    if (objp->IOStallAction >= 2) {

      remdi = remnode->remdi;
      while (remdi != 0) {
        *remdi->actval = FALSE;
        remdi = (remdi_item *) remdi->next;
      }		/* END while */

      remdo = remnode->remdo;
      while (remdo != 0) {
        if (!remdo->objp->PwrIsMaster) *remdo->actval = FALSE;
      remdo = (remdo_item *) remdo->next;
      }		/* END while */

      remdv = remnode->remdv;
      while (remdv != 0) {
        if (!remdv->objp->PwrIsMaster) *remdv->actval = FALSE;
        remdv = (remdv_item *) remdv->next;
      }		/* END while */

      remai = remnode->remai;
      while(remai) {
        *remai->rawval = 0;
        *remai->actval = ConvAItoAct(*remai->rawval, remai->objp);
        remai = (remai_item *) remai->next;
      }		/* END while */

      remao = remnode->remao;
      while(remao) {
        if (!remao->objp->PwrIsMaster) {
          *remao->rawval = 0;
          *remao->actval = ConvAOtoAct(*remao->rawval, remao->objp);
        }
        remao = (remao_item *) remao->next;
      }		/* END while */
    }		/* END if (IOStallAction... */
  }		/* END if new IOStallFlag */

  return(1);
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Receive_ALCM
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to struct remnode_item
*		Pointer to data buffer
*		Size of databuffer
*
* Description : Invoked when I/O-data is received from subsystem.
*		Packs up di-, do-, etc channels and store actual values
*		in rtdb.
*		Transport ALCM
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Receive_ALCM(	remnode_item	*remnode,
				bsp_buffer		*buffer,
				int		size)

#define	MAX16	32767
#define	MIN16	-32767
#define	MAXCO16	65536
#define	MAX24	8388607
#define	MIN24	-8388607
#define	MAXCO24	16777216

{

  char		*bytep;
  short int	*wordp;
  int		*longp;
  remdi_item	*remdi;
  remdo_item	*remdo;
  remdv_item	*remdv;
  remai_item	*remai;
  remao_item	*remao;
  remco_item	*remco;
  int		nochan;
  pwr_tInt32	longval;
  pwr_tInt32	diff;
  pwr_tStatus	err;
  
  pwr_sClass_RemnodeALCM *objp;
  pwr_tStatus sts;

  /* Get pointer to remnodeALCM-object */
  sts = gdh_ObjidToPointer(remnode->objid, (pwr_tAddress *) &objp);

  /* Update IOPollDiff */
  objp->IOPollDiff--;
  objp->IOStallFlag = FALSE;
  err = false;

  /* Store data in data-object */
  if (remnode->remio_size >= size) memcpy(remnode->remio_data, buffer, size);

  /* Pack up di-bits, set actval */
  bytep = (char *) &buffer->di_offset + buffer->di_offset;
  wordp = (short int *) bytep;
  nochan = *wordp;
  remnode->maxremdi = nochan * 8;
  bytep += 2;
  remdi = remnode->remdi;
  while (remdi != 0) {
    if (remdi->objp->BuffOff < nochan) *remdi->actval =
	(remdi->objp->ConvMask & *(bytep+remdi->objp->BuffOff)) ? TRUE : FALSE;
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Di offset %d\n",remdi->objp->ConvOff);
      }
    }
    remdi = (remdi_item *) remdi->next;
  }		/* END while */

  /* Pack up do-bits, set actval if not PwrIsMaster */
  bytep = (char *) &buffer->do_offset + buffer->do_offset;
  wordp = (short int *) bytep;
  nochan = *wordp;
  remnode->maxremdo = nochan * 8;
  bytep += 2;
  remdo = remnode->remdo;
  while (remdo != 0) {
    if (remdo->objp->BuffOff < nochan) {
      remdo->objp->OldValue =
	(remdo->objp->ConvMask & *(bytep+remdo->objp->BuffOff)) ? TRUE : FALSE;
      if (!remdo->objp->PwrIsMaster) *remdo->actval = remdo->objp->OldValue;
    }
    else {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Do offset %d\n",remdo->objp->ConvOff);
      }
    }
    remdo = (remdo_item *) remdo->next;
  }		/* END while */

  /* Pack up dv-bits, set actval if not PwrIsMaster */
  bytep = (char *) &buffer->dv_offset + buffer->dv_offset;
  wordp = (short int *) bytep;
  nochan = *wordp;
  remnode->maxremdv = nochan * 8;
  bytep += 2;
  remdv = remnode->remdv;
  while (remdv != 0) {
    if (remdv->objp->BuffOff < nochan) {
      remdv->objp->OldValue =
	(remdv->objp->ConvMask & *(bytep+remdv->objp->BuffOff)) ? TRUE : FALSE;
      if (!remdv->objp->PwrIsMaster) *remdv->actval = remdv->objp->OldValue;
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Dv offset %d\n",remdv->objp->ConvOff);
      }
    }
    remdv = (remdv_item *) remdv->next;
  }		/* END while */

  /*Convert raw AI values to actual values */
  bytep = (char *) &buffer->ai_offset + buffer->ai_offset;
  wordp = (short int *) bytep;
  remnode->maxremai = nochan = *wordp;
  /* Stega f�rbi den oanv�nda arean som �r lika stor som antalet AI (se dekl av bsp_buffer) */
  wordp += nochan+1;
  remai = remnode->remai;
  while(remai) {
    if (remai->objp->ConvOff < nochan) {
      *remai->rawval = *(wordp+remai->objp->ConvOff);
      *remai->actval = ConvAItoAct(*remai->rawval, remai->objp);
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Ai offset %d\n",remai->objp->ConvOff);
      }
    }
    remai = (remai_item *) remai->next;
  }		/* END while */

  /* Convert raw AO values to actual values, set oldvalue,
	set actval if not PwrIsMaster */
  bytep = (char *) &buffer->ao_offset + buffer->ao_offset;
  wordp = (short int *) bytep;
  remnode->maxremao = nochan = *wordp;
  wordp++;
  remao = remnode->remao;
  while(remao) {
    if (remao->objp->ConvOff < nochan) {
      remao->objp->OldValue = *(wordp+remao->objp->ConvOff);
      if (!remao->objp->PwrIsMaster) {
        *remao->rawval = remao->objp->OldValue; 
        *remao->actval = ConvAOtoAct(*remao->rawval, remao->objp);
      }
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Ao offset %d\n",remao->objp->ConvOff);
      }
    }
    remao = (remao_item *) remao->next;
  }		/* END while */

  /* Store Co values in COVALUEBASE and CAVALUEBASE */
  bytep = (char *) &buffer->co_offset + buffer->co_offset;
  wordp = (short int *) bytep;
  remnode->maxremco = nochan = *wordp;
  wordp++;
  longp = (int *) wordp;
  longp += nochan;
  remco = remnode->remco;
  while(remco) {
    if (remco->objp->ConvOff < nochan) {
      longval = *(longp+remco->objp->ConvOff); 
      diff  = longval - *remco->actval;
      /* 16-bit counter  ??? */
      if (remco->objp->NoOfBits == 16) {
        if (diff < MIN16)      diff += MAXCO16;
        else if (diff > MAX16) diff -= MAXCO16;
      }
      /* 24-bit counter */
      else {
        if (diff < MIN24)      diff += MAXCO24;
        else if (diff > MAX24) diff -= MAXCO24;
      }
      /* Add differens and save value */
      *remco->extval += diff;
      *remco->actval  = longval;
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        err = remnode->remio_err = TRUE;
        printf(" REMIO fel Co offset %d\n",remco->objp->ConvOff);
      }
    }
    remco = (remco_item *) remco->next;
  }		/* END while */

  /* Was all remote I/O inside buffer ? */
  if (!err && remnode->remio_err) {
    printf("Remote I/O node is OK again !\n");
  }
  remnode->remio_err = err;
  return(1);
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Cyclic_ALCM
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to remnode_item
*
* Description : Invoked when the cycle timer expires for any remNode.
*		Transport ALCM
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Cyclic_ALCM(	remnode_item *remnode,
				void (* send_pollbuff)
					(remnode_item *remnode, 
					pssupd_buffer *buf))
{

  remdo_item		*remdo;
  remdv_item		*remdv;
  remao_item		*remao;
  pssupd_buffer		askbuff;
  pssupd_order_header	*buffhead;
  unsigned char		buffdata[MAX_ORDERS_DATASIZE];
  unsigned char		*datap;
  unsigned int		buffsize, datasize;
  
  pwr_sClass_RemnodeALCM *objp;
  pwr_tStatus sts;

  /* Get pointer to remnodeALCM-object */
  sts = gdh_ObjidToPointer(remnode->objid, (pwr_tAddress *) &objp);

  /* Step through do, dv and ao to see if they should be
	updated remote */
  buffhead = (pssupd_order_header *) &askbuff.data;
  askbuff.no_of_updates = 0;
  datap = buffdata;
  datasize = buffsize = 0;

  remdo = remnode->remdo;
  while(remdo) {
    if (remdo->objp->PwrIsMaster && (remdo->objp->ConvOff < remnode->maxremdo)
		&& (remdo->objp->OldValue != *remdo->actval)){
      if ((buffsize + sizeof(pssupd_order_header) + 1) >
		MAX_ORDER_BUFFERSIZE ) {
        /* Send buffer and re-initiate it! */
        askbuff.length = buffsize + sizeof(pssupd_buffer) -
		MAX_ORDER_BUFFERSIZE;
        memcpy(buffhead, &buffdata, datasize);
        (send_pollbuff) (remnode, &askbuff);
        objp->IOPollDiff++;
        buffhead = (pssupd_order_header *) &askbuff.data;
        askbuff.no_of_updates = 0;
        datap = buffdata;
        datasize = buffsize = 0;
      }
      askbuff.no_of_updates++;
      buffhead->type = PSS_UPD_DO;
      buffhead->size = 0;
      buffhead->signal = remdo->objp->ConvOff;
      buffhead++;
      *datap = *remdo->actval;
      datap++;
      datasize++;
      buffsize += sizeof(pssupd_order_header) + 1;
    }
    remdo = (remdo_item *) remdo->next;
  }

  remdv = remnode->remdv;
  while(remdv) {
    if (remdv->objp->PwrIsMaster && (remdv->objp->ConvOff < remnode->maxremdv)
		&& remdv->objp->OldValue != *remdv->actval){
      if ((buffsize + sizeof(pssupd_order_header) + 1) >
		MAX_ORDER_BUFFERSIZE ) {
        /* Send buffer and re-initiate it! */
        askbuff.length = buffsize + sizeof(pssupd_buffer) -
		MAX_ORDER_BUFFERSIZE;
        memcpy(buffhead, &buffdata, datasize);
        (send_pollbuff) (remnode, &askbuff);
        objp->IOPollDiff++;
        buffhead = (pssupd_order_header *) &askbuff.data;
        askbuff.no_of_updates = 0;
        datap = buffdata;
        datasize = buffsize = 0;
      }
      askbuff.no_of_updates++;
      buffhead->type = PSS_UPD_DV;
      buffhead->size = 0;
      buffhead->signal = remdv->objp->ConvOff;
      buffhead++;
      *datap = *remdv->actval;
      datap++;
      datasize++;
      buffsize += sizeof(pssupd_order_header) + 1;
    }
    remdv = (remdv_item *) remdv->next;
  }

  remao = remnode->remao;
  while(remao) {
    if (remao->objp->PwrIsMaster && remao->objp->ConvOff < remnode->maxremao) {
      *remao->rawval = ConvAOtoRaw( *remao->actval, remao->objp);
      if (remao->objp->OldValue != *remao->rawval) {
        if ((buffsize + sizeof(pssupd_order_header) + 2) >
		MAX_ORDER_BUFFERSIZE ) {
        /* Send buffer and re-initiate it! */
          askbuff.length = buffsize + sizeof(pssupd_buffer) -
		MAX_ORDER_BUFFERSIZE;
          memcpy(buffhead, &buffdata, datasize);
          (send_pollbuff) (remnode, &askbuff);
          objp->IOPollDiff++;
          buffhead = (pssupd_order_header *) &askbuff.data;
          askbuff.no_of_updates = 0;
          datap = buffdata;
          datasize = buffsize = 0;
        }
        askbuff.no_of_updates++;
        buffhead->type = PSS_UPD_AO;
        buffhead->size = 2;
        buffhead->signal = remao->objp->ConvOff;
        buffhead++;
        *(short int *)datap = *remao->rawval;
        datap += 2;
        datasize += 2;
        buffsize += sizeof(pssupd_order_header) + 2;
      }
    }
    remao = (remao_item *) remao->next;
  }

  /* Sendbuffer if updates or poll */
  if ((askbuff.no_of_updates > 0) || objp->IOPoll) {
    askbuff.length = buffsize + sizeof(pssupd_buffer) - MAX_ORDER_BUFFERSIZE;
    memcpy(buffhead, &buffdata, datasize);	/* Copy data after headers */
    (send_pollbuff) (remnode, &askbuff);			/* Send orders */
    objp->IOPollDiff++;
  }

  return STATUS_OK;
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Init_3964R
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to struct remnode_item with objid and objp
*
* Description : Search for all RemChan_xx under rem_node
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Init_3964R(remnode_item *remnode)
{
  pwr_tObjid		childobjid, chanobjid, sigobjid;
  pwr_tClassId		class;
  pwr_sClass_Di		*dip;
  pwr_sClass_Do		*dop;
  pwr_sClass_Dv		*dvp;
  pwr_sClass_Ai		*aip;
  pwr_sClass_Ao		*aop;
  pwr_sClass_Co		*cop;
  pwr_sClass_RemChan_Di	*chandip;
  pwr_sClass_RemChan_Dv	*chandvp;
  pwr_sClass_RemChan_Do	*chandop;
  pwr_sClass_RemChan_Ai	*chanaip;
  pwr_sClass_RemChan_Ao	*chanaop;
  pwr_sClass_RemChan_Co	*chancop;
  remdi_item		*remdi;
  remdo_item		*remdo;
  remdv_item		*remdv;
  remai_item		*remai;
  remao_item		*remao;
  remco_item		*remco;
  pwr_tStatus		sts;
  pwr_sClass_Remnode3964R *objp;
  char 			name[80];

  /* Get pointer to remnode3964R-object */
  sts = gdh_ObjidToPointer(remnode->objid, (pwr_tAddress *) &objp);

  /* Initialize remnode_item and RemNode-object*/
  remnode->remdi = remdi = NULL;
  remnode->remdo = remdo = NULL;
  remnode->remdv = remdv = NULL;
  remnode->remai = remai = NULL;
  remnode->remao = remao = NULL;
  remnode->remco = remco = NULL;
  remnode->remio_data = NULL;

  remnode->maxremdi = 0;
  remnode->maxremdo = 0;
  remnode->maxremdv = 0;
  remnode->maxremai = 0;
  remnode->maxremao = 0;
  remnode->maxremco = 0;
  remnode->remio_err = FALSE;
  remnode->remio_size = 0;

  objp->LinkUp = 1;

  /* Get I/O dataarea */
  sts = gdh_GetChild(remnode->objid, &childobjid);
  while (ODD(sts)) {
    sts = gdh_ObjidToName(childobjid, name, sizeof(name), cdh_mName_object);
    if (strstr(name, "IO_AREA")) {
      sts = gdh_ObjidToPointer(childobjid, (pwr_tAddress *) &remnode->remio_data);
      if (ODD(sts)) sts = gdh_GetObjectSize(childobjid,(unsigned int *) &remnode->remio_size);
      break;
    }
    sts = gdh_GetNextSibling(childobjid, &childobjid);
  }

  /* Get first child to RemNode */
  sts = gdh_GetChild(remnode->objid, &childobjid);
  while (ODD(sts)) {

    /* Test type of object */
    sts = gdh_GetObjectClass(childobjid, &class);
    switch (class) {

      /* Search for RemChanDi with Di-connection */
      case pwr_cClass_RemDi: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Di) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chandip);
            if (ODD(sts)) {
              sigobjid = chandip->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Di) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &dip);
                /* Create new item and link it */
                remdi = malloc(sizeof(remdi_item));
                if (remdi == 0) exit(REM__NOMEMORY);
                remdi->next = (struct remdi_item *) remnode->remdi;
                remnode->remdi = remdi;

                /* Initialize remdi_item */
//                objp->NumberOfDI++;
                remdi->objp = chandip;
                remdi->actval = (pwr_tBoolean *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) dip->ActualValue);
              if (class == pwr_cClass_Di)  
                chandip->ActualValue = dip->ActualValue;
                remdi->objp->BuffOff = remdi->objp->ConvOff / 8;
                remdi->objp->ConvMask = 1 << (remdi->objp->ConvOff % 8);
              }	/* END ClassDi */
            }	/* END Object Di */
          }	/* END Class RemChanDi */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanDi under RemDi */
        break;
      }		/* END case RemDi */

      /* Search for RemChanD0 with Do-connection */
      case pwr_cClass_RemDo: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Do) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chandop);
            if (ODD(sts)) {
              sigobjid = chandop->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Do) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &dop);
                /* Create new item and link it */
                remdo = malloc(sizeof(remdo_item));
                if (remdo == 0) exit(REM__NOMEMORY);
                remdo->next = (struct remdo_item *) remnode->remdo;
                remnode->remdo = remdo;

                /* Initialize remdo_item */
//                objp->NumberOfDO++;
                remdo->objp = chandop;
                remdo->actval = gdh_TranslateRtdbPointer(
			(pwr_tUInt32) dop->ActualValue);
              if (class == pwr_cClass_Do)   
                chandop->ActualValue = dop->ActualValue;
                remdo->objp->BuffOff = remdo->objp->ConvOff / 8;
                remdo->objp->ConvMask = 1 << (remdo->objp->ConvOff % 8);
                remdo->objp->OldValue = *remdo->actval;
              }	/* END Class Do */
            }	/* END Object Do */
          }	/* END Class RemChanDo */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanDo under RemDo */
        break;
      }		/* END case RemDo */

      /* Search for RemChanDv with Dv-connection */
      case pwr_cClass_RemDv: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Dv) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chandvp);
            if (ODD(sts)) {
              sigobjid = chandvp->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Dv) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &dvp);
                /* Create new item and link it */
                remdv = malloc(sizeof(remdv_item));
                if (remdv == 0) exit(REM__NOMEMORY);
                remdv->next = (struct remdv_item *) remnode->remdv;
                remnode->remdv = remdv;

                /* Initialize Dv, RemChanDv and remdv_item */
//                objp->NumberOfDV++;
                remdv->objp = chandvp;
                remdv->actval = gdh_TranslateRtdbPointer(
			(pwr_tUInt32) dvp->ActualValue);
                chandvp->ActualValue = dvp->ActualValue;
                remdv->objp->OldValue = *remdv->actval;
                remdv->objp->BuffOff = remdv->objp->ConvOff / 8;
                remdv->objp->ConvMask = 1 << (remdv->objp->ConvOff % 8);
              }	/* END Class Dv */
            }	/* END Object Dv */
          }	/* END Class RemChanDv */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanDv under RemDv */
        break;
      }		/* END case RemDv */

      /* Search for RemChanAi with Ai-connection */
      case pwr_cClass_RemAi: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Ai) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chanaip);
            if (ODD(sts)) {
              sigobjid = chanaip->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Ai) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &aip);
                /* Create new item and link it */
                remai = malloc(sizeof(remai_item));
                if (remai == 0) exit(REM__NOMEMORY);
                remai->next = (struct remai_item *) remnode->remai;
                remnode->remai = remai;

                /* Initialize Ai, RemChanAi and remai_item */
//                objp->NumberOfAI++;
                remai->objp = chanaip;
                remai->actval = (pwr_tFloat32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) aip->ActualValue);
                chanaip->ActualValue = aip->ActualValue;
                remai->rawval = (short int *) &aip->RawValue;
              }	/* END Class Ai */
            }	/* END Object Ai */
          }	/* END Class RemChanAi */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanAi under RemAi */
        break;
      }		/* END case RemAi */

      /* Search for RemChanAo with Ao-connection */
      case pwr_cClass_RemAo: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Ao) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chanaop);
            if (ODD(sts)) {
              sigobjid = chanaop->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Ao) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &aop);
                /* Create new item and link it */
                remao = malloc(sizeof(remao_item));
                if (remao == 0) exit(REM__NOMEMORY);
                remao->next = (struct remao_item *) remnode->remao;
                remnode->remao = remao;

                /* Initialize Ao, RemChanAo and remao_item */
//                objp->NumberOfAO++;
                remao->objp = chanaop;
                remao->actval = (pwr_tFloat32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) aop->ActualValue);
                chanaop->ActualValue = aop->ActualValue;
                remao->rawval = (short int *)&aop->RawValue;
                remao->objp->OldValue = aop->RawValue;
              }	/* END Class Ao */
            }	/* END Object Ao */
          }	/* END Class RemChanAo */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanAo under RemAo */
        break;
      }		/* END case RemAo */

      /* Search for RemChanCo with Co-connection */
      case pwr_cClass_RemCo: {
        sts = gdh_GetChild(childobjid, &chanobjid);
        while (ODD(sts)) {
          sts = gdh_GetObjectClass(chanobjid, &class);
          if (class == pwr_cClass_RemChan_Co) {
            sts = gdh_ObjidToPointer(chanobjid, (pwr_tAddress *) &chancop);
            if (ODD(sts)) {
              sigobjid = chancop->SigChanCon;
              sts = gdh_GetObjectClass(sigobjid, &class);
              if (class == pwr_cClass_Co) {
                sts = gdh_ObjidToPointer(sigobjid, (pwr_tAddress *) &cop);
                /* Create new item and link it */
                remco = malloc(sizeof(remco_item));
                if (remco == 0) exit(REM__NOMEMORY);
                remco->next = (struct remco_item *) remnode->remco;
                remnode->remco = remco;

                /* Initialize Co, RemChanCo and remco_item */
//                objp->NumberOfCo++;
                remco->objp = chancop;
                remco->extval = (pwr_tInt32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) cop->AbsValue);
                chancop->ExtendedValue = cop->AbsValue;
                remco->actval = (pwr_tInt32 *) gdh_TranslateRtdbPointer(
			(pwr_tUInt32) cop->RawValue);
                chancop->ActualValue = cop->RawValue;
              }	/* END Class Co */
            }	/* END Object Co */
          }	/* END Class RemChanCo */
          sts = gdh_GetNextSibling(chanobjid, &chanobjid);
        }	/* END while RemChanCo under RemCo */
        break;
      }		/* END case RemCo */

    }		/* END switch */
    /* Get next child under RemNode */
    sts = gdh_GetNextSibling(childobjid, &childobjid);
  }	/* END while */

  return(1);
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Stall_3964R
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to struct remnode_item with objid and objp
*
* Description : Action made when remote IO stalls
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Stall_3964R(remnode_item *remnode, int stall_action)
{
  remdi_item	*remdi;
  remdo_item	*remdo;
  remdv_item	*remdv;
  remai_item	*remai;
  remao_item	*remao;

  pwr_sClass_Remnode3964R *objp;
  pwr_tStatus sts;

  /* Get pointer to remnode-object */
  sts = gdh_ObjidToPointer(remnode->objid, (pwr_tAddress *) &objp);
  
  /* Set StallFlag */
  if (objp->LinkUp != 0) {
    objp->LinkUp = 0;

    /* Clear Remote signals according to IOStallAction */
    if (stall_action >= 2) {

      remdi = remnode->remdi;
      while (remdi != 0) {
        *remdi->actval = FALSE;
        remdi = (remdi_item *) remdi->next;
      }		/* END while */

      remdo = remnode->remdo;
      while (remdo != 0) {
        if (!remdo->objp->PwrIsMaster) *remdo->actval = FALSE;
      remdo = (remdo_item *) remdo->next;
      }		/* END while */

      remdv = remnode->remdv;
      while (remdv != 0) {
        if (!remdv->objp->PwrIsMaster) *remdv->actval = FALSE;
        remdv = (remdv_item *) remdv->next;
      }		/* END while */

      remai = remnode->remai;
      while(remai) {
        *remai->rawval = 0;
        *remai->actval = ConvAItoAct(*remai->rawval, remai->objp);
        remai = (remai_item *) remai->next;
      }		/* END while */

      remao = remnode->remao;
      while(remao) {
        if (!remao->objp->PwrIsMaster) {
          *remao->rawval = 0;
          *remao->actval = ConvAOtoAct(*remao->rawval, remao->objp);
        }
        remao = (remao_item *) remao->next;
      }		/* END while */
    }		/* END if (IOStallAction... */
  }		/* END if new IOStallFlag */

  return(1);
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Receive_3964R
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to struct remnode_item
*		Pointer to data buffer
*		Size of databuffer
*
* Description : Invoked when I/O-data is received from subsystem.
*		Packs up di-, do-, etc channels and store actual values
*		in rtdb.
*		Transport 3964R VNET
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Receive_3964R(remnode_item	 *remnode,
			        unsigned char  *buffer,
			        int            size)

#define	MAX16	32767
#define	MIN16	-32767
#define	MAXCO16	65536
#define	MAX24	8388607
#define	MIN24	-8388607
#define	MAXCO24	16777216

{

  unsigned char *bytep;
  short int	*wordp;
  int		*longp;
  remdi_item	*remdi;
  remdo_item	*remdo;
  remdv_item	*remdv;
  remai_item	*remai;
  remao_item	*remao;
  remco_item	*remco;
  int		nochan;
  pwr_tInt32	longval;
  pwr_tInt32	diff;
  pwr_tStatus	err;

  /* Update PollDiff etc */
//  remnode->objp->PollDiff--;
//  remnode->objp->LinkUp = 1;
//  remnode->Time_since_IO = 0;
  err = false;

  /* Store data in data-object */
  if (remnode->remio_size >= size) memcpy(remnode->remio_data, buffer, size);

  /* Pack up di-bits, set actval */
  bytep = buffer;
  wordp = (short int *) bytep;
  nochan = *wordp * 2;                 /* Convert to bytes */
  remnode->maxremdi = nochan * 8;
  bytep += 2;
  remdi = remnode->remdi;
  while (remdi != 0) {
    if (remdi->objp->BuffOff < nochan) *remdi->actval =
	(remdi->objp->ConvMask & *(bytep+remdi->objp->BuffOff)) ? TRUE : FALSE;
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Di offset %d\n",remdi->objp->ConvOff);
      }
    }
    remdi = (remdi_item *) remdi->next;
  }		/* END while */

  /* Pack up do-bits, set actval if not PwrIsMaster */
  bytep += nochan;
  wordp = (short int *) bytep;
  nochan = *wordp * 2;                 /* Convert to bytes */
  remnode->maxremdo = nochan * 8;
  bytep += 2;
  remdo = remnode->remdo;
  while (remdo != 0) {
    if (remdo->objp->BuffOff < nochan) {
      remdo->objp->OldValue =
	(remdo->objp->ConvMask & *(bytep+remdo->objp->BuffOff)) ? TRUE : FALSE;
      if (!remdo->objp->PwrIsMaster) *remdo->actval = remdo->objp->OldValue;
    }
    else {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Do offset %d\n",remdo->objp->ConvOff);
      }
    }
    remdo = (remdo_item *) remdo->next;
  }		/* END while */

  /* Pack up dv-bits, set actval if not PwrIsMaster */
  bytep += nochan;
  wordp = (short int *) bytep;
  nochan = *wordp * 2;                 /* Convert to bytes */
  remnode->maxremdv = nochan * 8;
  bytep += 2;
  remdv = remnode->remdv;
  while (remdv != 0) {
    if (remdv->objp->BuffOff < nochan) {
      remdv->objp->OldValue =
	(remdv->objp->ConvMask & *(bytep+remdv->objp->BuffOff)) ? TRUE : FALSE;
      if (!remdv->objp->PwrIsMaster) *remdv->actval = remdv->objp->OldValue;
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Dv offset %d\n",remdv->objp->ConvOff);
      }
    }
    remdv = (remdv_item *) remdv->next;
  }		/* END while */

  /*Convert raw AI values to actual values */
  bytep += nochan;
  wordp = (short int *) bytep;
  remnode->maxremai = nochan = *wordp;
  bytep += 2;
  wordp += 1;
  remai = remnode->remai;
  while(remai) {
    if (remai->objp->ConvOff < nochan) {
      *remai->rawval = *(wordp+remai->objp->ConvOff);
      *remai->actval = ConvAItoAct(*remai->rawval, remai->objp);
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Ai offset %d\n",remai->objp->ConvOff);
      }
    }
    remai = (remai_item *) remai->next;
  }		/* END while */

  /* Convert raw AO values to actual values, set oldvalue,
	set actval if not PwrIsMaster */
  bytep += nochan * 2;                   /* Convert to bytes */
  wordp = (short int *) bytep;
  remnode->maxremao = nochan = *wordp;
  bytep += 2;
  wordp += 1;
  remao = remnode->remao;
  while(remao) {
    if (remao->objp->ConvOff < nochan) {
      remao->objp->OldValue = *(wordp+remao->objp->ConvOff);
      if (!remao->objp->PwrIsMaster) {
        *remao->rawval = remao->objp->OldValue; 
        *remao->actval = ConvAOtoAct(*remao->rawval, remao->objp);
      }
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        remnode->remio_err = TRUE;
        printf(" REMIO fel Ao offset %d\n",remao->objp->ConvOff);
      }
    }
    remao = (remao_item *) remao->next;
  }		/* END while */

  /* Store Co values in COVALUEBASE and CAVALUEBASE */
  bytep += nochan * 2;                   /* Convert to bytes */
  wordp = (short int *) bytep;
  remnode->maxremco = nochan = *wordp;
  wordp += 1;
  longp = (int *) wordp;
  remco = remnode->remco;
  while(remco) {
    if (remco->objp->ConvOff < nochan) {
      longval = *(longp+remco->objp->ConvOff); 
      diff  = longval - *remco->actval;
      /* 16-bit counter  ??? */
      if (remco->objp->NoOfBits == 16) {
        if (diff < MIN16)      diff += MAXCO16;
        else if (diff > MAX16) diff -= MAXCO16;
      }
      /* 24-bit counter */
      else {
        if (diff < MIN24)      diff += MAXCO24;
        else if (diff > MAX24) diff -= MAXCO24;
      }
      /* Add difference and save value */
      *remco->extval += diff;
      *remco->actval  = longval;
    }
    else  {
      err = TRUE;
      if (!remnode->remio_err) {
        err = remnode->remio_err = TRUE;
        printf(" REMIO fel Co offset %d\n",remco->objp->ConvOff);
      }
    }
    remco = (remco_item *) remco->next;
  }		/* END while */

  /* Was all remote I/O inside buffer ? */
  if (!err && remnode->remio_err) {
    printf("Remote I/O node is OK again !\n");
  }
  remnode->remio_err = err;
  return(1);
}


/*************************************************************************
**************************************************************************
*
* Name : RemIO_Cyclic_3964R
*
* Typ  : pwr_tStatus
*
* Input :	Pointer to remnode_item
*
* Description : Invoked when the cycle timer expires for the remNode.
*		Transport 3964R VNET
*
**************************************************************************
**************************************************************************/

pwr_tStatus RemIO_Cyclic_3964R(remnode_item	*remnode,
				void (* send_pollbuff)
					(remnode_item *remnode, 
					pssupd_buffer_vnet *buf))
{

  remdo_item		*remdo;
  remdv_item		*remdv;
  remao_item		*remao;
  pssupd_buffer_vnet	askbuff;
  pssupd_order_header	*buffhead;
  unsigned char		buffdata[MAX_ORDERS_DATASIZE];
  unsigned char		*datap;
  unsigned int		buffsize, datasize;

  /* Step through do, dv and ao to see if they should be
	updated remote */
  buffhead = (pssupd_order_header *) &askbuff.data;
  askbuff.no_of_updates = 0;
  datap = buffdata;
  datasize = buffsize = 0;

  remdo = remnode->remdo;
  while(remdo) {
    if (remdo->objp->PwrIsMaster && (remdo->objp->ConvOff < remnode->maxremdo)
		&& (remdo->objp->OldValue != *remdo->actval)){
      if ((buffsize + 2 * (sizeof(pssupd_order_header) + 1) ) >
           MAX_ORDER_BUFFERSIZE_VNET ) {
        /* Add a follow on telegram header. */
        askbuff.no_of_updates++;
        buffhead->type = PSS_Follow_On;
        buffhead->size = buffhead->signal = 0;
        *datap = 0;
        buffsize += sizeof(pssupd_order_header) + 1;
        /* Send buffer and re-initiate it! */
        askbuff.length = (buffsize + sizeof(pssupd_buffer_vnet) -
           MAX_ORDER_BUFFERSIZE_VNET + 1) / 2;
        buffhead++;
        memcpy(buffhead, &buffdata, datasize);
        (send_pollbuff) (remnode, &askbuff);
//        remnode->objp->PollDiff++;
        buffhead = (pssupd_order_header *) &askbuff.data;
        askbuff.no_of_updates = 0;
        datap = buffdata;
        datasize = buffsize = 0;
      } 
      askbuff.no_of_updates++;
      buffhead->type = PSS_UPD_DO;
      buffhead->size = 0;
      buffhead->signal = remdo->objp->ConvOff;
      buffhead++;
      *datap = *remdo->actval;
      datap++;
      datasize++;
      buffsize += sizeof(pssupd_order_header) + 1;
    }
    remdo = (remdo_item *) remdo->next;
  }

  remdv = remnode->remdv;
  while(remdv) {
    if (remdv->objp->PwrIsMaster && (remdv->objp->ConvOff < remnode->maxremdv)
		&& remdv->objp->OldValue != *remdv->actval){
      if ((buffsize + 2 * (sizeof(pssupd_order_header) + 1) ) >
		MAX_ORDER_BUFFERSIZE_VNET ) {
        /* Add a follow on telegram header. */
        askbuff.no_of_updates++;
        buffhead->type = PSS_Follow_On;
        buffhead->size = buffhead->signal = 0;
        *datap = 0;
        buffsize += sizeof(pssupd_order_header) + 1;
        /* Send buffer and re-initiate it! */
        askbuff.length = (buffsize + sizeof(pssupd_buffer_vnet) -
		MAX_ORDER_BUFFERSIZE_VNET + 1) / 2;
        buffhead++;
        memcpy(buffhead, &buffdata, datasize);
        (send_pollbuff) (remnode, &askbuff);
//        remnode->objp->PollDiff++;
        buffhead = (pssupd_order_header *) &askbuff.data;
        askbuff.no_of_updates = 0;
        datap = buffdata;
        datasize = buffsize = 0;
      }
      askbuff.no_of_updates++;
      buffhead->type = PSS_UPD_DV;
      buffhead->size = 0;
      buffhead->signal = remdv->objp->ConvOff;
      buffhead++;
      *datap = *remdv->actval;
      datap++;
      datasize++;
      buffsize += sizeof(pssupd_order_header) + 1;
    }
    remdv = (remdv_item *) remdv->next;
  }

  remao = remnode->remao;
  while(remao) {
    if (remao->objp->PwrIsMaster && remao->objp->ConvOff < remnode->maxremao) {
      *remao->rawval = ConvAOtoRaw( *remao->actval, remao->objp);
      if (remao->objp->OldValue != *remao->rawval) {
        if ((buffsize + 2 * (sizeof(pssupd_order_header) + 2) ) >
		MAX_ORDER_BUFFERSIZE_VNET ) {
          /* Add a follow on telegram header. */
          askbuff.no_of_updates++;
          buffhead->type = PSS_Follow_On;
          buffhead->size = buffhead->signal = 0;
          *(short int *)datap = 0;
          buffsize += sizeof(pssupd_order_header) + 2;
          /* Send buffer and re-initiate it! */
          askbuff.length = (buffsize + sizeof(pssupd_buffer_vnet) -
		MAX_ORDER_BUFFERSIZE_VNET + 1) / 2;
          buffhead++;
          memcpy(buffhead, &buffdata, datasize);
          (send_pollbuff) (remnode, &askbuff);
//          remnode->objp->PollDiff++;
          buffhead = (pssupd_order_header *) &askbuff.data;
          askbuff.no_of_updates = 0;
          datap = buffdata;
          datasize = buffsize = 0;
        }
        askbuff.no_of_updates++;
        buffhead->type = PSS_UPD_AO;
        buffhead->size = 2;
        buffhead->signal = remao->objp->ConvOff;
        buffhead++;
        *(short int *)datap = *remao->rawval;
        datap += 2;
        datasize += 2;
        buffsize += sizeof(pssupd_order_header) + 2;
      }
    }
    remao = (remao_item *) remao->next;
  }

  /* Always send */
//  if ((askbuff.no_of_updates > 0) /* || remnode->objp->Poll */) {
    askbuff.length = (buffsize + sizeof(pssupd_buffer_vnet) - 
                     MAX_ORDER_BUFFERSIZE_VNET + 1) / 2;
    memcpy(buffhead, &buffdata, datasize);  /* Add data after headers */
    (send_pollbuff) (remnode, &askbuff);    /* Send orders */
//    remnode->objp->PollDiff++;
//  }

  return STATUS_OK;
}