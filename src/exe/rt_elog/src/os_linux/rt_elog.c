/* 
 * Proview   $Id: rt_elog.c,v 1.8 2006-01-25 14:50:59 claes Exp $
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

/* rt_eventlogger.c 

   Author Jonas Nylund.

   Receives events from the local MH and stores them in a
   historical event log database on disk.  */

#include "rt_elog.h"
#include "twolist.h"
#include "rt_aproc.h"
#include "rt_pwr_msg.h"
#include "rt_ini_event.h"
#include "rt_qcom.h"
#include "rt_qcom_msg.h"

#define Log_Error(a, b) errh_Error("%s\n%m",b, a)
#define Log(b) errh_Info(b)
#define Log_Error_Exit(a, b) {Log_Error(a, b); errh_SetStatus(PWR__SRVTERM); exit(a);}
#define Log_Error_Return(a, b) {Log_Error(a, b); return (a);}
#define If_Error_Log(a, b) if ((a & 1) != 1) Log_Error(a, b)
#define If_Error_Log_Return(a, b) if ((a & 1) != 1) Log_Error_Return(a, b)
#define If_Error_Log_Exit(a, b) if ((a & 1) != 1) Log_Error_Exit(a, b)

#define KEY_ARRAY_SIZE 50

void Init();

static pwr_tStatus Insert(mh_sMsgInfo *ip);

pwr_tInt32 GetOldestEvents(pwr_tUInt32 *nrOfEvents, pwr_tUInt32 *nrOfKeys);

void Store (  pwr_tBoolean *firstTime, pwr_tUInt32 *nrOfEvents, pwr_tUInt32 *nrOfKeys );

static sEvent *CopyEvent(
  mh_sMsgInfo *ip
);
int compTime(sKey d1, sKey d2);

int writeInfo(int nrOfEvents, int firstTimeEver, pwr_tTime *oldestEventTime);

sHelCB lHelCB;
DB *dataBaseP = NULL;
pwr_tUInt32 nrOfInsertsSinceLastTime = 0;
headtyp *listhead;
linktyp *lanklank;
char info_fname[200];

int 
main ()
{
  pwr_tStatus sts;
  pwr_tObjid  oid;
  qcom_sQid   my_q = qcom_cNQid;
  qcom_sGet   get;

  pwr_tBoolean firstTime = TRUE;
  pwr_tUInt32 nrOfEvents = 0;
  pwr_tUInt32 nrOfKeys = 0;
  
  errh_Init("pwr_elog", errh_eAnix_elog);
  errh_SetStatus( PWR__SRVSTARTUP);

  memset(&lHelCB, 0, sizeof(lHelCB));

  /* Declare process */
  sts = gdh_Init("pwr_elog");
  If_Error_Log_Exit(sts, "gdh_Init");

  Init();

  /* Create queue for receival of events */
  if (!qcom_CreateQ(&sts, &my_q, NULL, "events")) {
    errh_Fatal("qcom_CreateQ, %m", sts);
    errh_SetStatus( PWR__APPLTERM);
    exit(sts);
  } 

  if (!qcom_Bind(&sts, &my_q, &qcom_cQini)) {
    errh_Fatal("qcom_Bind(Qini), %m", sts);
    errh_SetStatus( PWR__APPLTERM);
    exit(-1);
  }
  
  oid.vid = lHelCB.Nid;
  oid.oix = pwr_cNVolumeId;

  sts = mh_OutunitConnect(
    oid, 
    mh_eOutunitType_Logger, 
    mh_mOutunitFlags_ReadWait, 
    (mh_cbOutunitAck)Insert, 
    (mh_cbOutunitAlarm)Insert, 
    (mh_cbOutunitBlock)Insert, 
    (mh_cbOutunitCancel)Insert, 
    NULL, 
    NULL, 
    (mh_cbOutunitInfo)Insert,
    (mh_cbOutunitReturn)Insert
  );
  If_Error_Log_Exit(sts, "mh_OutunitConnect");

  sts = mh_OutunitSetTimeout(lHelCB.ScanTime);

  errh_SetStatus( PWR__SRUN);

  for (;;) {
    sts = mh_OutunitReceive();
    if (EVEN(sts) && sts != MH__TMO)
      Log_Error(sts, "mh_OutunitReceive");
    Store(&firstTime, &nrOfEvents, &nrOfKeys);

    get.data = NULL;
    qcom_Get(&sts, &my_q, &get, 0);
    if (sts != QCOM__TMO && sts != QCOM__QEMPTY) {
      if (get.type.b == qcom_eBtype_event) {
        qcom_sEvent  *ep = (qcom_sEvent*) get.data;
        ini_mEvent    new_event;
        if (get.type.s == qcom_cIini) {
          new_event.m = ep->mask;
          if (new_event.b.terminate) {
            errh_SetStatus( PWR__APPLTERM);
            exit(0);
          }
        }
      }
      qcom_Free(&sts, get.data);
    }

    aproc_TimeStamp();
  }
}

void
Init ()
{
  pwr_tUInt32 sts;
  pwr_tInt32 ret;
  char msg[80];
  pwr_tObjid	MHObjId;
  pwr_sAttrRef	AttrRef; 
  pwr_tDlid	DLId;
  pwr_sClass_MessageHandler *MH;
  char fname[200];
    
  dcli_translate_filename( fname, DATABASE);
  dcli_translate_filename( info_fname, DATABASE_INFO);
  
  sts = gdh_GetNodeIndex(&lHelCB.Nid);
  If_Error_Log_Exit(sts, "gdh_GetNodeIndex");

  sts = gdh_GetClassList(pwr_cClass_MessageHandler, &MHObjId);
  If_Error_Log_Exit(sts, "Couldn't find message handler object");

  AttrRef.Objid = MHObjId;
  AttrRef.Body = 0;
  AttrRef.Offset = 0;
  AttrRef.Size = sizeof(pwr_sClass_MessageHandler);
  AttrRef.Flags.m = 0;

  sts = gdh_DLRefObjectInfoAttrref(&AttrRef, (pwr_tAddress *)&MH, &DLId);
  If_Error_Log_Exit(sts,"Couldn't get direct link to message handler object");
  
  if (MH->EventLogSize == 0) {
    Log("EventLogSize = 0, no event logger will run on this node.");
    errh_SetStatus( pwr_cNStatus);
    exit(1);
  }

  lHelCB.MaxCardinality = MH->EventLogSize;
  lHelCB.MaxStoreLSize = 1000; /*not used*/
  lHelCB.ScanTime = 2000; /* 5 seconds */

  /*create the database if it's not already created*/
  if((ret = db_create(&dataBaseP, NULL, 0)) != 0)
  {
    /*error creating db-handle send the mess to errh, then exit*/
    sprintf(msg, "db_create: %s, no eventlogger will run", db_strerror(ret));
    errh_Error(msg);
    exit(1);
  }
  /*open the database*/

#if (DB_VERSION_MAJOR == 4) && (DB_VERSION_MINOR > 0)
  ret = dataBaseP->open(dataBaseP, NULL, fname, NULL, DATABASETYPE, DB_CREATE, 0664);
#else
  ret = dataBaseP->open(dataBaseP, fname, NULL, DATABASETYPE, DB_CREATE, 0664);
#endif
  if(ret != 0)
  {
    /*error opening/creating db send the mess to errh, then exit*/
    sprintf(msg, "db_open: %s, no eventlogger will run", db_strerror(ret));
    Log(msg);
    exit(1);
  }
  
  newhead(&listhead);
}

/* Inserts an event into the DBs cache. */
static pwr_tStatus
Insert (
  mh_sMsgInfo *ip
)
{
  sEvent *sp;
  char msg[80];
  pwr_tInt32 ret;
  sKey eventKey;
  DBT key, data;
  
  /*copy the event into the struct that is to be stored*/
  sp = CopyEvent(ip);

  /*clear the DBT-structs for future avoidance of errors(if DB is upgraded)*/
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  eventKey.Id = ip->Id;
  eventKey.EventTime = ip->EventTime;
  
  /* copy the data do the DBT-structs*/
  key.data = (void *)&eventKey;
  key.size = sizeof(sKey);
  data.data = (void *)sp;
  data.size = sizeof(sEvent);

  switch(ret = dataBaseP->put(dataBaseP, NULL, &key, &data, DB_NOOVERWRITE))
  {
    case 0:
      /*everything is good*/
      nrOfInsertsSinceLastTime++;
      break;
    case DB_KEYEXIST:
      /*data elready stored*/
      break;
    default:
      /*error storing the event in DB send errormess to errh*/
      sprintf(msg, "dbp->put: %s, no mess stored", db_strerror(ret));
      Log(msg);
      break;
  }
  free(sp);
  return MH__SUCCESS;
}

void Store( pwr_tBoolean *firstTime, pwr_tUInt32 *nrOfEvents, pwr_tUInt32 *nrOfKeys )
{
  char msg[80];
  pwr_tInt32 ret = 0;

  DBT key;
  sKey skey;

  /*flush the cache to the DB-file*/
  dataBaseP->sync(dataBaseP, 0);
  
  /*check if it's time to clean the DB*/
  if( *firstTime || ((*nrOfEvents + nrOfInsertsSinceLastTime) > lHelCB.MaxCardinality ) )
  {
    *firstTime = FALSE;
    if(nrOfInsertsSinceLastTime > 0)
    {
      nrOfInsertsSinceLastTime--;
      *nrOfEvents = *nrOfEvents + 1;
    }
    if( *nrOfKeys <= 0)
    {
      ret = GetOldestEvents(nrOfEvents, nrOfKeys);
      nrOfInsertsSinceLastTime = 0;
      switch(ret)
      {
        case RT_ELOG_UNKNOWN_ERROR:
          errh_Error("RT_ELOG_UNKNOWN_ERROR");
          break;
        case RT_ELOG_OK:
          break;
        case RT_ELOG_DB_EMPTY:
          break;
        default:
          errh_Error("Undefined return: %d", ret);
          break;
      }
    }
    else
    {      
      lanklank = firstlink(listhead);
      skey = getlink(lanklank);
      memset(&key, 0, sizeof(key));
      key.data = &skey;
      key.size = sizeof(sKey);

      if( (ret = dataBaseP->del(dataBaseP, NULL, &key, 0)) != 0 )
      {
        sprintf(msg, "Error deleting Record in HistDB nrOfKeys = %d Errmess=%s\n", *nrOfKeys, db_strerror(ret));
	Log(msg);
      }
      else
      {
        *nrOfEvents = *nrOfEvents - 1;
      }
      
      elimlink(&lanklank);
      *nrOfKeys = *nrOfKeys - 1;
    }
      
  }
  if(*nrOfKeys > 0)
  {
      lanklank = firstlink(listhead);
      if(lanklank != 0)
      {
        writeInfo(*nrOfEvents+nrOfInsertsSinceLastTime, 0, &lanklank->data.EventTime);
      }
      else
      {
        writeInfo(*nrOfEvents+nrOfInsertsSinceLastTime, 0, NULL);
      }
  }
  else
  {
    writeInfo(*nrOfEvents+nrOfInsertsSinceLastTime, 0, NULL);
  }
}  
  
pwr_tInt32 GetOldestEvents(pwr_tUInt32 *nrOfEvents , pwr_tUInt32 *nrOfKeys)
{
  sKey *tmpData;
  pwr_tInt32 ret;
  DBT data, key;
  DBC *dbcp;

  pwr_tUInt32 evCount = 0;
  pwr_tUInt32 tmp = 0;

  dbcp = NULL;

  /* Acquire a cursor for the database. */ 
  if ((ret = dataBaseP->cursor(dataBaseP, NULL, &dbcp, 0)) != 0) 
  { 
    errh_Error("error dataBaseP->cursor: %s\n", db_strerror(ret));
    return RT_ELOG_UNKNOWN_ERROR;
  }

  /* Initialize the key/data return pair. */
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  /*Get the first event*/
  if((ret = dbcp->c_get(dbcp, &key, &data, DB_FIRST)) != 0)
  {
    *nrOfEvents = 0;
    return RT_ELOG_DB_EMPTY;
  }
  evCount++;

  tmpData = (sKey *)key.data;
  newlink(&lanklank);
  putlink(*tmpData, lanklank);
  insort(lanklank, listhead, &compTime);
  tmp++;

  while((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0)
  {
    evCount++;
    
    tmpData = (sKey *)key.data;
    newlink(&lanklank);
    putlink(*tmpData, lanklank);
    insort(lanklank, listhead, &compTime);
    tmp++;
    if(nrlinks(listhead) > KEY_ARRAY_SIZE)
    {
      tmp--;
      lanklank = lastlink(listhead);
      elimlink(&lanklank);
    }
  }
    
  if(ret != DB_NOTFOUND)
  {
    errh_Error("Error dbcp->c_get(DB_NEXT): %s\n", db_strerror(ret));
    return RT_ELOG_UNKNOWN_ERROR;
  }

  /*Close the cursor*/
  if((ret = dbcp->c_close(dbcp)) != 0)
  {
    errh_Error("Error dbcp_oldest->c_close(): %s\n", db_strerror(ret));
  }
  *nrOfEvents = evCount;
  *nrOfKeys = tmp;
  return RT_ELOG_OK;
}  

sEvent *
CopyEvent (
  mh_sMsgInfo *ip
)
{
  sEvent *sp;
  
  mh_sMessage *mp = (mh_sMessage*) ip;
  mh_sAck *ap = (mh_sAck*) ip;
  mh_sReturn *rp = (mh_sReturn*) ip;
  mh_sBlock *bp = (mh_sBlock*) ip;
  
  sp = malloc(sizeof(sEvent));

  sp->EventType = ip->EventType;
  sp->EventTime = ip->EventTime;

  switch (ip->EventType) {
  case mh_eEvent_Alarm:
  case mh_eEvent_Info:
    sp->Mess.message = *mp;
    break;
  case mh_eEvent_Ack:
    sp->Mess.ack = *ap;
    break;
  case mh_eEvent_Cancel:
  case mh_eEvent_Return:
    sp->Mess.ret = *rp;
    break;
  case mh_eEvent_Block:
  case mh_eEvent_Unblock:
  case mh_eEvent_Reblock:
  case mh_eEvent_CancelBlock:
    sp->Mess.block = *bp;
    break;
  default:
    Log("rt_elog: Error in CopyEvent unknown EventType");
    break;
  }
  return sp;
}

int compTime(sKey d1, sKey d2)
{
  if(time_Acomp( &(d1.EventTime), &(d2.EventTime)) < 0 )
  {
    return 1;
  }
  return 0;
}


int writeInfo(int nrOfEvents, int firstTimeEver, pwr_tTime *oldestEventTime)
{
  FILE *info_file = NULL;
  char time_str[40];

  time_str[0] = 0;
  if(oldestEventTime != NULL)
  {
    time_AtoAscii(oldestEventTime, time_eFormat_DateAndTime, time_str, sizeof(time_str));
  }  
  //open a file to print info about the database to
  info_file = fopen( info_fname, "w" );
  if(info_file == NULL) return 2;

  fprintf(info_file, "NrOfEvents:%d OldestEventTime:%s\n", nrOfEvents, time_str);
  fclose(info_file);
  return 1;
}



