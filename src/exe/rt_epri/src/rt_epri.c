/* 
 * Proview   $Id: rt_epri.c,v 1.2 2005-09-01 14:57:48 claes Exp $
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

/* rt_eventprinter.c -- Runtime Environment - Event printer */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <descrip.h>
#include "pwr.h"
#include "pwr_class.h"
#include "pwr_baseclasses.h"
#include "rt_mh_outunit.h"
#include "rt_mh_msg.h"
#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "co_time.h"


#ifndef PWR_LANG 
#define PWR_LANG_SWE
#endif

#if defined PWR_LANG_SWE
#define _ALA	    "Lar-"
#define _ACK	    "Kvi-"
#define _BLC	    "Blo-"
#define _RET	    "Ret-"
#define _CANC	    "Ann-"
#define _MSG	    "Med "
#define _UBLC	    "Dbl-"
#define _RBLC	    "Rbl-"
#define _CBLC	    "Abl-"
#define _OPPLACE    "OpPlats"
#define _OPERATOR   "Operat�r"
#define _VALUE	    "V�rde: "
#define _LIMIT	    "Gr�ns: "
#define _TIMEFORM   0	/* SWEDISH */


#elif defined PWR_LANG_ENG
#define _ALA	    "Ala-"
#define _ACK	    "Ack-"
#define _BLC	    "Blo-"
#define _RET	    "Rtn-"
#define _CANC	    "Can-"
#define _MSG	    "Msg "
#define _UBLC	    "Ubl-"
#define _RBLC	    "Rbl-"
#define _CBLC	    "Cbl-"
#define _OPPLACE    "OpPlace"
#define _OPERATOR   "Operator"
#define _VALUE	    "Value: "
#define _LIMIT	    "Limit: "
#define _TIMEFORM   1	/* ENGLISH */
#endif


#define _TIMESOL    0	/* SECOND */
#define _INDENT_STR "       "
#define _NO_INDENT  7

#define ACTUALVALUENAME "ActualValue"

static FILE *Printer = NULL;
static pwr_tString80 EventPrinter;
static pwr_sClass_EventPrinter EventPrinterObj;

static void FormatEventMsg(
  mh_sMsgInfo *ip,
  char *EventMsg,
  int RowSize
);
static char *FormatHighLow(
  mh_eSupType SupType,
  mh_uSupInfo *SupInfo
);
static void FormatMsgInfo(
  char *EventStr,
  mh_sMsgInfo *ip,
  char *HighLow
);
static char *FormatOperator(
  pwr_tObjid Outunit
);
static char *FormatPrio(
  mh_sMsgInfo *ip
);
static char *FormatSupInfo(
  mh_eSupType SupType,
  mh_uSupInfo *sip
);
static void FormatValue(
  pwr_tFloat32 Value, 
  char *ValString
);
static void PrintEvent(
  mh_sMsgInfo  *MsgP
);
static char *StripActualValue(
  char *FullName
);
static pwr_tStatus Timer (
  pwr_tInt32 TimerId
);

/*------------------------------------------------------------------------------
*/
static char *StripActualValue(
  char *FullName
)
{
  char  *sp;
  static pwr_tString80 Text;

  strncpy(Text, FullName, sizeof(Text));
  Text[sizeof(Text) - 1] = '\0';

  if ((sp = strchr(Text, '.')) != NULL)
    if (strcmp(sp + 1, ACTUALVALUENAME) == 0)
      *sp = '\0';

  return Text;
}

static void FormatEventMsg(
  mh_sMsgInfo *ip,
  char *EventMsg,
  int RowSize
)
{
  pwr_tInt32 Idx;
  mh_sAck *ap;
  mh_sMessage *mp;
  mh_sReturn *rp;
  mh_sBlock *bp;
  char EventStr[300];

  switch(ip->EventType) {
  case mh_eEvent_Alarm: 
    strcpy(EventStr, _ALA);
    mp = (mh_sMessage *) ip;
    FormatMsgInfo(EventStr, ip, FormatHighLow(mp->SupInfo.SupType, &mp->SupInfo));
    strcat(EventStr, mp->EventText);
    strcat(EventStr, "  ");
    strcat(EventStr, FormatSupInfo(mp->SupInfo.SupType, &mp->SupInfo));
    break;
  case mh_eEvent_Ack:
    strcpy(EventStr, _ACK);
    ap = (mh_sAck *) ip;
    FormatMsgInfo(EventStr, ip, FormatHighLow(ap->SupInfo.SupType, &ap->SupInfo));
    strcat(EventStr, FormatOperator(ap->Outunit));
    break;
  case mh_eEvent_Block:
    strcpy(EventStr, _BLC);
    bp = (mh_sBlock *) ip;
    FormatMsgInfo(EventStr, ip, " ");
    strcat(EventStr, FormatOperator(bp->Outunit));
    break;
  case mh_eEvent_Unblock:
    strcpy(EventStr, _UBLC);
    bp = (mh_sBlock *) ip;
    FormatMsgInfo(EventStr, ip, " ");
    strcat(EventStr, FormatOperator(bp->Outunit));
    break;
  case mh_eEvent_Reblock:
    strcpy(EventStr, _RBLC);
    bp = (mh_sBlock *) ip;
    FormatMsgInfo(EventStr, ip, " ");
    strcat(EventStr, FormatOperator(bp->Outunit));
    break;
  case mh_eEvent_CancelBlock:
    strcpy(EventStr, _CBLC);
    bp = (mh_sBlock *) ip;
    FormatMsgInfo(EventStr, ip, " ");
    break;
  case mh_eEvent_Return:
    strcpy(EventStr, _RET);
    rp = (mh_sReturn *) ip;
    FormatMsgInfo(EventStr, ip, FormatHighLow(rp->SupInfo.SupType, &rp->SupInfo));
    strcat(EventStr, rp->EventText);
    break;
  case mh_eEvent_Cancel:
    strcpy(EventStr, _CANC);
    rp = (mh_sReturn *) ip;
    FormatMsgInfo(EventStr, ip, FormatHighLow(rp->SupInfo.SupType, &rp->SupInfo));
    strcat(EventStr, rp->EventText);
    break;
  case mh_eEvent_Info:
    strcpy(EventStr, _MSG);
    mp = (mh_sMessage *) ip;
    FormatMsgInfo(EventStr, ip, FormatHighLow(mp->SupInfo.SupType, &mp->SupInfo));
    strcat(EventStr, mp->EventText);
    strcat(EventStr, "  ");
    strcat(EventStr, FormatSupInfo(mp->SupInfo.SupType, &mp->SupInfo));
    break;
  default :
    break;       
  }

  if (strlen(EventStr) > RowSize) {   
    strncpy(EventMsg, EventStr, RowSize);
    EventMsg[RowSize] = '\0';
    Idx = RowSize;

    while(Idx < strlen(EventStr)) {
      strcat(EventMsg, "\n");
      strcat(EventMsg, _INDENT_STR);
      strncat(EventMsg, &EventStr[Idx], RowSize - _NO_INDENT);
      Idx += RowSize - _NO_INDENT;
    }
  } else
    strcpy(EventMsg, EventStr);

  strcat(EventMsg, "\n");

}

/*------------------------------------------------------------------------------
*/
static void FormatMsgInfo(
  char *EventStr,
  mh_sMsgInfo *ip,
  char *HighLow
)
{
    char Time[80];

    strcat(EventStr, FormatPrio(ip));
    strcat(EventStr, HighLow);
    strcat(EventStr, " ");
    time_AtoFormAscii(&ip->EventTime ,_TIMESOL,_TIMEFORM,Time,sizeof(Time));
    strcat(EventStr, Time);
    strcat(EventStr, "  ");
    strcat(EventStr, StripActualValue(ip->EventName));
    strcat(EventStr, "  ");
}

/*------------------------------------------------------------------------------
*/
static char *FormatHighLow(
  mh_eSupType SupType,
  mh_uSupInfo *SupInfo
)
{
  static char Text[5];

  if (SupType == mh_eSupType_Analog)
    if (SupInfo->mh_uSupInfo_u.A.High)
      strcpy(Text,"H");
    else
      strcpy(Text,"L");
  else
    strcpy(Text," ");

  return Text;
}

/*------------------------------------------------------------------------------
*/
static char *FormatSupInfo(
  mh_eSupType SupType,
  mh_uSupInfo *sip
)
{
  static char Text[100];
  pwr_tString40	ActValue,LimitValue;

  Text[0] = '\0';

  if (SupType == mh_eSupType_Analog) {
    FormatValue(sip->mh_uSupInfo_u.A.ActualValue, ActValue);
    FormatValue(sip->mh_uSupInfo_u.A.CtrlLimit, LimitValue);
    sprintf(Text, " %s%s %s,  %s%s %s", _VALUE,	ActValue,
      sip->mh_uSupInfo_u.A.Unit, _LIMIT, LimitValue, sip->mh_uSupInfo_u.A.Unit
    );
  }

  return Text;
}

/*------------------------------------------------------------------------------
*/
static char *FormatPrio(
  mh_sMsgInfo *ip
)
{
  static char Text[5];

  switch (ip->EventPrio) {
  case mh_eEventPrio_A:
    strcpy(Text, "A");	    
    break;
  case mh_eEventPrio_B:
    strcpy(Text, "B");
    break;
  case mh_eEventPrio_C:
    strcpy(Text, "C");
    break;
  case mh_eEventPrio_D:
    strcpy(Text, "D");
    break;
  default:
    strcpy(Text, "M");
    break;
  }
  return Text;
}

/*------------------------------------------------------------------------------
*/
static char *FormatOperator(
  pwr_tObjid Outunit
)
{
  pwr_tStatus	  sts;
  static char	  Text[100];
  pwr_tClassId	  Class;
  pwr_sClass_User Operator;
  pwr_tFullName	  FullName;

  strcpy(Text,"--");
  sts = gdh_GetObjectClass(Outunit, &Class); 
  if (ODD(sts)) {
    if (Class == pwr_cClass_User) {
      sts = gdh_ObjidToName(Outunit, FullName, sizeof(FullName), cdh_mNName);
      if (ODD(sts)) {   
	sts = gdh_GetObjectInfo(FullName, &Operator, sizeof(pwr_sClass_User));
	if (ODD(sts)) {
	  sprintf(Text, "%s %d %s %s", _OPPLACE, Operator.OpNumber,
	    _OPERATOR, Operator.UserName
	  );
	}
      }
    }
  }

  return Text;
}

/*------------------------------------------------------------------------------
* Prints value to a string with number of decimals depending on the size
* of the value.
*/
static void FormatValue(
  pwr_tFloat32 Value, 
  char *ValString
)
{
  pwr_tFloat32 AbsValue;
  int  Dec;

  if (Value < 0)
    AbsValue = - Value;
  else
    AbsValue = Value;

  if (AbsValue >= 1000 || AbsValue == 0)
    Dec = 0;
  else if (AbsValue >= 100)
    Dec = 1;
  else if (AbsValue >= 10)
    Dec = 2;
  else if (AbsValue >= 1)
    Dec = 3;
  else if (AbsValue >= 0.1)
    Dec = 4;
  else if (AbsValue >= 0.01)
    Dec = 5;
  else
    Dec = 6;

  sprintf(ValString,"%.*f",Dec, Value);

}

static void PrintEvent(
  mh_sMsgInfo  *ip
)
{
  char EventMsg[512];

  FormatEventMsg(ip, EventMsg, EventPrinterObj.RowSize);
  fprintf(Printer, EventMsg);
}

#if 0
static pwr_tStatus Timer (
  pwr_tInt32 TimerId
)
{
  char string[200];
  mh_eEventPrio Prio;
  pwr_tObjid Object;
  pwr_tStatus sts;
  pwr_tInt32  TenthsOfSeconds = 10;
  pwr_tInt32  TimerIdentity = 10;

  for (;;) {
    printf("Outunit> ");
    if (gets(string) == NULL)
      exit(1);
    printf("\n");
    
    if (string[0] == 'r')
      break;

    switch (string[0]) {
    case 'b':
      printf("Block an object\n");
      printf("  Object name: ");
      gets(string);
      printf("\n");
      if (string == NULL)
	break;
      sts = gdh_NameToObjid(string, &Object);
      if (EVEN(sts)) {
	printf("Cannot find object, sts = %d\n", sts);
	break;
      }
      printf("  Priority: ");
      gets(string);
      printf("\n");
      if (string == NULL)
	break;
      Prio = atoi(string);
      printf("mh_OutunitBlock(Object, Prio = %d)\n", Prio);
      sts = mh_OutunitBlock(Object, Prio); 
      if (EVEN(sts)) {
	printf("Cannot block object, sts = %d\n", sts);
	break;
      }
      break;
    default:
      break;
    }
  }

  sts = mh_OutunitSetTimer(&TenthsOfSeconds, &TimerIdentity);
  return 1;
}
#endif

int main(int argc, char *argv[])
{
  pwr_tStatus	   sts;
  pwr_tObjid	   Object;
  pwr_tClassId	   Class;
  mh_eOutunitType  Type;
  mh_mOutunitFlags Flags;
  pwr_tInt32	   TenthsOfSeconds = 10;
  pwr_tInt32	   TimerId = 10;


  sts = gdh_Init("pwr_epri");
  if (EVEN(sts)) {
    exit (sts);
  }

  if (argc > 1)	{
    if (strcmp(argv[1], "-t") == 0) {
      /* test mode */
      Object = pwr_cNObjid;
      Type = mh_eOutunitType_Logger;
      Flags = mh_mOutunitFlags_ReadWait;
    } else {
      Type = mh_eOutunitType_Printer;
      Flags = mh_mOutunitFlags_ReadWait;
      strcpy(EventPrinter ,argv[1]);
      sts = gdh_NameToObjid(argv[1], &Object);
      if (EVEN(sts)) {
	printf("%%PRI-E-NOVALID_EVENTPRINT. No valid EventPrinter object\n");
	exit(sts);
      }
      sts = gdh_GetObjectClass(Object , &Class);
      if (Class != pwr_cClass_EventPrinter) {
	printf("%%PRI-E-NOVALID_EVENTPRINT. EventPrinter object, wrong class\n");
	exit(sts);
      }
    }
  } else {
    printf("%%PRI-E-NO_EVENT_PRINTER. No EventPrinter object\n");
    exit(2);
  }
  
  sts = mh_OutunitConnect(
    Object,
    Type,
    Flags,
    (mh_cbOutunitAck)PrintEvent,
    (mh_cbOutunitAlarm)PrintEvent,
    (mh_cbOutunitBlock)PrintEvent,
    (mh_cbOutunitCancel)PrintEvent,
    NULL,
    NULL,
    (mh_cbOutunitInfo)PrintEvent,
    (mh_cbOutunitReturn)PrintEvent
  );

  if (EVEN(sts)) {
    printf("%%PRI-E-CONNECT. Could not connect Outunit to message handler.\n");
    exit(sts);
  }  

  if (Type == mh_eOutunitType_Printer) {
    sts = gdh_GetObjectInfo(EventPrinter, &EventPrinterObj,
      sizeof(EventPrinterObj)
    );
    if (EVEN(sts)) {
      printf("%%PRI-E-EVENTPRINT Cannot get EventPrinter object\n");
      exit(sts);
    }
    if ((Printer = fopen(EventPrinterObj.DeviceName, "w")) == NULL) {
      printf("%%PRI-E-NOPRINT  Cannot open printer: %s\n", Printer);
      exit(1);
    }
    if (EventPrinterObj.RowSize == 0)
      EventPrinterObj.RowSize = 80;

  } else {
#if 0
    sts = mh_OutunitSetTimer(&TenthsOfSeconds, &TimerId);
#endif
    Printer = stdout;
    EventPrinterObj.RowSize = 80;
  }

  for(;;) {
    sts = mh_OutunitReceive();     
  }
}
