/* 
 * Proview   $Id: jpwr_rt_hist.cpp,v 1.6 2007-02-06 15:12:36 claes Exp $
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


using namespace std;

#include <deque>
#include <string>
#include <string.h>
#include <stdio.h>
#include "jpwr_rt_hist.h"
#include <db.h>

extern "C" {
#include "pwr.h"
#include "rt_mh.h"
#include "rt_mh_util.h"
#include "rt_elog.h"
#include "rt_gdh.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "co_cdh_msg.h"
#include "rt_gdh_msg.h"
static void gdh_ConvertUTFstring( char *out, char *in);
}
jobject convertAckToMhrEvent( mh_sAck *MsgP);
jobject convertReturnToMhrEvent( mh_sReturn *MsgP);
jobject convertAlarmOrInfoToMhrEvent( mh_sMessage *MsgP);
int compareStr(char *ev, char *usr);


#define ERROR_TIME_CONVERT  -99


typedef struct HistQueryStruct
{
    char                *minTime_str;
    char                *maxTime_str;
    char                *eventName_str;
    char                *eventText_str;
    
    bool                eventPrio_A;
    bool                eventPrio_B;
    bool                eventPrio_C;
    bool                eventPrio_D;
    
    bool                eventType_Ack;
    bool                eventType_Alarm;
    bool                eventType_Info;
    bool                eventType_Return;
    bool                eventType_Cancel;
    bool                eventType_Block;
    bool                eventType_Unblock;
    bool                eventType_Reblock;
    bool                eventType_CancelBlock;
}HistQuery;

int check_conditions(sEvent *evp, HistQuery *query);


JavaVM *jvm;

JNIEXPORT void JNICALL Java_jpwr_rt_Hist_initHistIDs
  (JNIEnv *env, jclass cls)
{
  int sts;

  //initiera alla pekare till javametoderna som beh�vs
  //  Mh_id = env->FindClass("jpwr/rt/Mh");
  //  if(Mh_id == NULL)
  //  {
  //    printf("jpwr_rt_mh.c: couldnt find jpwr/rt/Mh class\n");
  //  }

  sts = env->GetJavaVM(&jvm);
  if(sts)
  {

    printf("Hittar ej JavaVM\n");
  }
}



/*
 * Class:     jpwr_rt_Mh
 * Method:    getHistList
 * Signature: (Ljava/lang/String;Ljava/lang/String;ZZZZZZZZLjava/lang/String;Ljava/lang/String;)[Ljpwr/rt/MhrEvent;
 */
JNIEXPORT jobjectArray JNICALL Java_jpwr_rt_Hist_getHistList
  (JNIEnv *env, jclass obj, jstring jstartTime, jstring jstopTime, jboolean jtypeAlarm, jboolean jtypeInfo, jboolean jtypeReturn, jboolean jtypeAck, jboolean jprioA, jboolean jprioB, jboolean jprioC, jboolean jprioD, jstring jname, jstring jtext)
{
  const char *str;
  char *cstr_minTime;
  char *cstr_maxTime;
  char *cstr_eventName;
  char *cstr_eventText;

  DB *dataBaseP = NULL;
  pwr_tInt32 ret, sts;
  char dbName[200];

  pwr_tUInt32 nrOfEvents = 0;
  sEvent *eventp;
  DBT data, key;
  DBC *dbcp;
  deque<sEvent> evDeq;
  HistQuery query;
  jobjectArray jobjectArr = NULL;
  unsigned int i = 0;

  //find the class for MhrEvent[]
  jclass mhrEventArrCls = env->FindClass("jpwr/rt/MhrEvent");

  if(mhrEventArrCls == NULL)
  {
      printf("Hittade inte jpwr/rt/MhrEvent\n");
    return (jobjectArray)NULL;
  }

  
  //  printf("I get_hist_list\n");
  
  memset(&query, 0, sizeof(HistQuery));

  //translate the jstrings to char[]
  str = env->GetStringUTFChars( jstartTime, 0);
  cstr_minTime = (char *)str;  
  gdh_ConvertUTFstring( cstr_minTime, cstr_minTime);
  query.minTime_str = cstr_minTime;

  str = env->GetStringUTFChars( jstopTime, 0);
  cstr_maxTime = (char *)str;  
  gdh_ConvertUTFstring( cstr_maxTime, cstr_maxTime);
  query.maxTime_str = cstr_maxTime;

  str = env->GetStringUTFChars( jname, 0);
  cstr_eventName = (char *)str;  
  gdh_ConvertUTFstring( cstr_eventName, cstr_eventName);
  query.eventName_str = cstr_eventName;

  str = env->GetStringUTFChars( jtext, 0);
  cstr_eventText = (char *)str;  
  gdh_ConvertUTFstring( cstr_eventText, cstr_eventText);
  query.eventText_str = cstr_eventText;

  query.eventType_Ack = jtypeAck;
  query.eventType_Alarm = jtypeAlarm;
  query.eventType_Return = jtypeReturn;
  query.eventType_Info = jtypeInfo;
  query.eventPrio_A = jprioA;
  query.eventPrio_B = jprioB;
  query.eventPrio_C = jprioC;
  query.eventPrio_D = jprioD;

  //printf("I get_hist_list1\n");
  
  dcli_translate_filename( dbName, DATABASE);
  
  /*create the database if it's not already created*/
  if((ret = db_create(&dataBaseP, NULL, 0)) != 0)
  {
    /*error creating db-handle send the mess to errh, then exit*/
    printf("error db_create: %s\n", db_strerror(ret));
    printf(" Fel vid skapande av databashandtag avslutar\n");
    goto err;
  }
#if DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR > 0
  ret = dataBaseP->open(dataBaseP, NULL, dbName, NULL, DATABASETYPE, DB_RDONLY, 0);
#else
  ret = dataBaseP->open(dataBaseP, dbName, NULL, DATABASETYPE, DB_RDONLY, 0);
#endif

  if(ret != 0)
  {
    /*error opening/creating db send the mess to errh, then exit*/
    printf("error db_open: %s\n", db_strerror(ret));
    goto err;
  }

    
  /* Acquire a cursor for the database. */ 
  if ((ret = dataBaseP->cursor(dataBaseP, NULL, &dbcp, 0)) != 0) 
  {
    printf("error dataBaseP->cursor: %s\n", db_strerror(ret)); 
    goto err;
  }

  /* Initialize the key/data return pair. */
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  if((ret = dbcp->c_get(dbcp, &key, &data, DB_FIRST)) == 0)
  {
    eventp = (sEvent *)data.data;
    sts = check_conditions(eventp, &query);
    if(sts == ERROR_TIME_CONVERT)
    {
      printf("Error trying to convert userinput in time-field 1\n");
    }
    else if(ODD(sts))
    {
      nrOfEvents++;
      evDeq.push_front((sEvent)*eventp);
    }
    
      
  }
  
  while((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0)
  {
    eventp = (sEvent *)data.data;
    sts = check_conditions(eventp, &query);
    if(sts == ERROR_TIME_CONVERT)
    {
      printf("Error trying to convert userinput in time-field\n");
    }
    else if(ODD(sts))
    {
      nrOfEvents++;
      evDeq.push_front(*eventp);
    }
  }
  if(ret != DB_NOTFOUND)
  {
    printf("error dbcp->c_get: %s\n", db_strerror(ret));
    printf("Fel vid f�rs�k att l�sa post nr %u, avslutar\n", nrOfEvents);
  }

  /*Close the cursor*/
  if((ret = dbcp->c_close(dbcp)) != 0)
  {
    printf("Error dbcp->c_close(): %s\n", db_strerror(ret));
  }
  /*close the database*/  

  if((ret = dataBaseP->close(dataBaseP,0) != 0))
  {
    printf("error db_close: %s\n", db_strerror(ret));
  }


  //create a new MhrEvent[]
  jobjectArr = (jobjectArray)env->NewObjectArray(nrOfEvents, mhrEventArrCls, NULL);
  
  printf("nrOfEvents: %d\n", nrOfEvents);
  sEvent ev;
  //put the result in an objectarray of MhrEvent
  for(i=0;i<nrOfEvents;i++)
  {
    ev = evDeq.front();
    
    switch (ev.EventType) {
      case mh_eEvent_Alarm:
      case mh_eEvent_Info:
        env->SetObjectArrayElement(jobjectArr, i, convertAlarmOrInfoToMhrEvent( (mh_sMessage *)(&(ev.Mess) ) ) );
        //printMess(sp, outFile);
      break;
      case mh_eEvent_Ack:
        env->SetObjectArrayElement(jobjectArr, i, convertAckToMhrEvent( (mh_sAck *)(&(ev.Mess) ) ) );
        //printAck(sp, outFile);
      break;
      case mh_eEvent_Cancel:
      case mh_eEvent_Return:
        env->SetObjectArrayElement(jobjectArr, i, convertReturnToMhrEvent( (mh_sReturn *)(&(ev.Mess) ) ) );
        //printRet(sp, outFile);
      break;
      case mh_eEvent_Block:
      case mh_eEvent_Unblock:
      case mh_eEvent_Reblock:
      case mh_eEvent_CancelBlock:
        env->SetObjectArrayElement(jobjectArr, i, NULL);
        //printBlock(sp, outFile);
      break;
      default:
        printf("histlist: Error in Write unknown EventType");
      break;

    }  
      evDeq.pop_front();
  }

err:
  env->ReleaseStringUTFChars( jstartTime, cstr_minTime );
  env->ReleaseStringUTFChars( jstopTime, cstr_maxTime );
  env->ReleaseStringUTFChars( jname, cstr_eventName );
  env->ReleaseStringUTFChars( jtext, cstr_eventText );
  return jobjectArr;
}

int check_conditions(sEvent *evp, HistQuery *query)
{
  pwr_tTime minTime;
  pwr_tTime maxTime;
  int sts;

  //first we compare the time
  if( (query->minTime_str != NULL) && (strlen(query->minTime_str) != 0) )
  {
    sts = time_FormAsciiToA(query->minTime_str, SWE, SECOND, &minTime);
    if(EVEN(sts)) 
      return ERROR_TIME_CONVERT;
    if(time_Acomp(&minTime, &(evp->EventTime)) > 0)
      return 2; //evensts
  }
  if(query->maxTime_str != NULL && (strlen(query->maxTime_str) != 0))
  {
    sts = time_FormAsciiToA(query->maxTime_str, SWE, SECOND, &maxTime);
    if(EVEN(sts)) 
      return ERROR_TIME_CONVERT;
    if(time_Acomp(&maxTime, &(evp->EventTime)) < 0)
      return 2; //evensts
  }
  bool ret = false;
  //then we compare the EventType if nothing is selected everything is selected
  if(query->eventType_Ack || 
     query->eventType_Alarm || 
     query->eventType_Info || 
     query->eventType_Return || 
     query->eventType_Cancel || 
     query->eventType_Block ||
     query->eventType_Unblock || 
     query->eventType_Reblock || 
     query->eventType_CancelBlock)
  {
    switch(evp->EventType) 
    {
      case mh_eEvent_Alarm:
        if(!query->eventType_Alarm)
	  ret = true;
        break;
      case mh_eEvent_Info:
        if(!query->eventType_Info)
	  ret = true;
        break;
      case mh_eEvent_Ack:
        if(!query->eventType_Ack)
	  ret = true;
        break;
      case mh_eEvent_Cancel:
        if(!query->eventType_Cancel)
	  ret = true;
        break;
      case mh_eEvent_Return:
        if(!query->eventType_Return)
	  ret = true;
        break;
      case mh_eEvent_Block:
        if(!query->eventType_Block)
	  ret = true;
        break;
      case mh_eEvent_Unblock:
        if(!query->eventType_Unblock)
	  ret = true;
        break;
      case mh_eEvent_Reblock:
        if(!query->eventType_Reblock)
	  ret = true;
        break;
      case mh_eEvent_CancelBlock:
        if(!query->eventType_CancelBlock)
	  ret = true;
        break;
      default:
        ret = true;
    }
  }
  if(ret)
    return 2;
  
  
  mh_sMsgInfo *msgInfop = NULL;
  mh_sMessage *mp = NULL;
  mh_sReturn  *rp = NULL;
  
  switch (evp->EventType) 
  {
    case mh_eEvent_Alarm:
    case mh_eEvent_Info:
      msgInfop = &(evp->Mess.message.Info);
      mp = &(evp->Mess.message);
      break;
    case mh_eEvent_Ack:
      msgInfop = &(evp->Mess.ack.Info);
      break;
    case mh_eEvent_Cancel:
    case mh_eEvent_Return:
      msgInfop = &(evp->Mess.ret.Info);
      rp = &(evp->Mess.ret);
      break;
    case mh_eEvent_Block:
    case mh_eEvent_Unblock:
    case mh_eEvent_Reblock:
    case mh_eEvent_CancelBlock:
      msgInfop = &(evp->Mess.block.Info);
      break;
    default:
      return 2;
  }
  
  //compare the prio, if nothing is selected everything is selected
  if(query->eventPrio_A || query->eventPrio_B || query->eventPrio_C || query->eventPrio_D)
  {
    //compare the EventPrio
    switch (msgInfop->EventPrio) {
      case mh_eEventPrio_A:
        if(!query->eventPrio_A)
          return 2;
        break;
      case mh_eEventPrio_B:
        if(!query->eventPrio_B)
          return 2;
        break;
      case mh_eEventPrio_C:
        if(!query->eventPrio_C)
          return 2;
        break;
      case mh_eEventPrio_D:
        if(!query->eventPrio_D)
          return 2;
        break;
      default:
        return 2;
    }
  }

  //compare the EventName
  if(query->eventName_str != NULL && (strlen(query->eventName_str) != 0) )
  {
    if( EVEN( compareStr(msgInfop->EventName, query->eventName_str) ) )
      return 2;
  }
  if(query->eventText_str != NULL && (strlen(query->eventText_str) != 0) )
  {
    if(mp != NULL)
    {
      if( EVEN( compareStr(mp->EventText, query->eventText_str) ) )
        return 2;
    }
    else if(rp != NULL)
    {
      if( EVEN( compareStr(rp->EventText, query->eventText_str) ) )
        return 2;
    }
    else
      return 2;
  }
  
  //every condition was true so the mess is alright
  return 1;
  
}

int compareStr(char *ev, char *usr)
{
  int sts;
  int startPos = 0;
  unsigned int endPos = 0;
  char *str1;
  sts = dcli_toupper(usr,usr);
  if(ODD(sts))
  {
    sts = dcli_wildcard(usr,ev);
    sts += 1;
    if(ODD(sts))
      return sts;

    string s = usr;	
    endPos = s.find_first_of(';', startPos);
    while(endPos != string::npos)
    {
      str1 = (char *)(s.substr(startPos, endPos-startPos)).c_str();
      sts = dcli_wildcard(str1,ev);
      sts += 1;
      startPos = endPos+1;
      if(ODD(sts))
        return sts;
      endPos = s.find_first_of(';', startPos);
    }
    str1 = (char *)(s.substr(startPos)).c_str();
    sts = dcli_wildcard(str1,ev);
    sts += 1;
  }
  return sts;
}


jobject convertAlarmOrInfoToMhrEvent( mh_sMessage *MsgP)
{
  JNIEnv *env;
  jclass PwrtObjid_id;
  jmethodID PwrtObjid_cid;
  jobject objid_obj = NULL;
  jstring jevText;
  jstring jevName;
  jstring jevTime;
  jstring jevBirthTime;
  jint jevFlags;
  jint jevPrio;
  jint jevStatus;
  jint jevNix;
  jint jevIdx;
  jint jevType;
  jint oix, vid;

  jstring jevTargetBirthTime;
  jint jevTargetNix = (jint) 0;
  jint jevTargetIdx = (jint) 0;


  char time_str[40];
  
  char birthTime_str[40];
  
  pwr_tObjid objid = MsgP->Info.Object;
  pwr_tTime time = MsgP->Info.EventTime;
  pwr_tTime birthTime = MsgP->Info.Id.BirthTime;
  
  //h�mta enviormentpekaren
  jvm->AttachCurrentThread((void **)&env,NULL);
  if(env == NULL) printf("env �r null");
  
  PwrtObjid_id = env->FindClass( "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = env->GetMethodID( PwrtObjid_id,
    	"<init>", "(II)V");

  oix = (jint) objid.oix;
  vid = (jint) objid.vid;
  objid_obj = env->NewObject( PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);

  
  time_AtoAscii( &time, time_eFormat_ComprDateAndTime, time_str, sizeof(time_str));
  time_AtoAscii( &birthTime, time_eFormat_ComprDateAndTime, birthTime_str, sizeof(birthTime_str));
  
  //g�r om till Java-str�ngar
  jevText = env->NewStringUTF( MsgP->EventText);
  jevName = env->NewStringUTF( MsgP->Info.EventName);
  jevTime = env->NewStringUTF( time_str);
  jevBirthTime = env->NewStringUTF( birthTime_str);

  jevTargetBirthTime = env->NewStringUTF( " ");

  
  //g�r om till Java-int
  jevFlags = (jint)MsgP->Info.EventFlags;
  jevPrio = (jint)MsgP->Info.EventPrio;
  jevStatus = (jint)MsgP->Status;
  jevNix = (jint)MsgP->Info.Id.Nix;
  jevIdx = (jint)MsgP->Info.Id.Idx;
  jevType = (jint)MsgP->Info.EventType;


  jclass MhrEvent_id = env->FindClass( "jpwr/rt/MhrEvent");
  jmethodID MhrEvent_cid = env->GetMethodID( MhrEvent_id,
    	"<init>",   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIILjava/lang/String;IILjava/lang/String;IILjpwr/rt/PwrtObjid;)V");


  //anropa callback metoden i Mh-klassen
  jobject return_obj = env->NewObject( MhrEvent_id, MhrEvent_cid, jevText, jevName,
                                jevTime, jevFlags, jevPrio, jevStatus, jevNix, jevBirthTime, jevIdx,
				jevTargetNix, jevTargetBirthTime, jevTargetIdx,
				jevType, objid_obj);
  //important:check if an exception was raised 
  if (env->ExceptionCheck())
  {
    printf("exception i convertAlarmToMhrEvent\n");
    return NULL;
  }
  
  return return_obj;
}


jobject convertReturnToMhrEvent( mh_sReturn *MsgP)
{
  JNIEnv *env;
  jclass PwrtObjid_id;
  jmethodID PwrtObjid_cid;
  jobject objid_obj = NULL;
  jstring jevText;
  jstring jevName;
  jstring jevTime;
  jstring jevBirthTime;
  jint jevFlags;
  jint jevPrio;
  jint jevStatus;
  jint jevNix;
  jint jevIdx;
  jint jevType;
  jint oix, vid;

  jstring jevTargetBirthTime;
  jint jevTargetNix;
  jint jevTargetIdx;

  char targetBirthTime_str[40];

  char time_str[40];
  
  char birthTime_str[40];
  
  pwr_tObjid objid = MsgP->Info.Object;
  pwr_tTime time = MsgP->Info.EventTime;
  pwr_tTime birthTime = MsgP->Info.Id.BirthTime;
  pwr_tTime targetBirthTime = MsgP->TargetId.BirthTime;
  
  //h�mta enviormentpekaren
  jvm->AttachCurrentThread((void **)&env,NULL);
  if(env == NULL) printf("env �r null");
  
  PwrtObjid_id = env->FindClass( "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = env->GetMethodID( PwrtObjid_id,
    	"<init>", "(II)V");

  oix = (jint) objid.oix;
  vid = (jint) objid.vid;
  objid_obj = env->NewObject( PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);

  
  time_AtoAscii( &time, time_eFormat_ComprDateAndTime, time_str, sizeof(time_str));
  time_AtoAscii( &birthTime, time_eFormat_ComprDateAndTime, birthTime_str, sizeof(birthTime_str));
  time_AtoAscii( &targetBirthTime, time_eFormat_ComprDateAndTime, targetBirthTime_str, sizeof(targetBirthTime_str));
  
  //g�r om till Java-str�ngar
  jevText = env->NewStringUTF( MsgP->EventText);
  jevName = env->NewStringUTF( MsgP->Info.EventName);
  jevTime = env->NewStringUTF( time_str);
  jevBirthTime = env->NewStringUTF( birthTime_str);
  jevTargetBirthTime = env->NewStringUTF( targetBirthTime_str);

  
  //g�r om till Java-int
  jevFlags = (jint)MsgP->Info.EventFlags;
  jevPrio = (jint)MsgP->Info.EventPrio;
  jevStatus = (jint)1;//mh_sReturn har ingen status
  jevNix = (jint)MsgP->Info.Id.Nix;
  jevIdx = (jint)MsgP->Info.Id.Idx;

  jevTargetNix = (jint)MsgP->TargetId.Nix;
  jevTargetIdx = (jint)MsgP->TargetId.Idx;


  jevType = (jint)MsgP->Info.EventType;

  jclass MhrEvent_id = env->FindClass( "jpwr/rt/MhrEvent");
  jmethodID MhrEvent_cid = env->GetMethodID( MhrEvent_id,
    	"<init>",   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIILjava/lang/String;IILjava/lang/String;IILjpwr/rt/PwrtObjid;)V");


  //anropa callback metoden i Mh-klassen
  jobject return_obj = env->NewObject( MhrEvent_id, MhrEvent_cid, jevText, jevName,
                                jevTime, jevFlags, jevPrio, jevStatus, jevNix, jevBirthTime, jevIdx,
				jevTargetNix, jevTargetBirthTime, jevTargetIdx,
				jevType, objid_obj);
  //important:check if an exception was raised 
  if (env->ExceptionCheck())
  {
    printf("exception i return\n");
    return NULL;
  }
  
  return return_obj;
}

jobject convertAckToMhrEvent( mh_sAck *MsgP)
{
  JNIEnv *env;
  jclass PwrtObjid_id;
  jmethodID PwrtObjid_cid;
  jobject objid_obj = NULL;
  jstring jevText;
  jstring jevName;
  jstring jevTime;
  jstring jevBirthTime;


  jint jevFlags;
  jint jevPrio;
  jint jevStatus;
  jint jevNix;
  jint jevIdx;

  jstring jevTargetBirthTime;
  jint jevTargetNix;
  jint jevTargetIdx;
  char targetBirthTime_str[40];
  
  jint jevType;
  jint oix, vid;
  char time_str[40];
  char birthTime_str[40];
  pwr_tObjid objid = MsgP->Info.SupObject;
  
  pwr_tTime time = MsgP->Info.EventTime;
  pwr_tTime birthTime = MsgP->Info.Id.BirthTime;
  pwr_tTime targetBirthTime = MsgP->TargetId.BirthTime;
  
  //h�mta enviormentpekaren
  jvm->AttachCurrentThread((void **)&env,NULL);
  if(env == NULL) printf("env �r null");
  
  
  
  PwrtObjid_id = env->FindClass( "jpwr/rt/PwrtObjid");
  PwrtObjid_cid = env->GetMethodID( PwrtObjid_id,
    	"<init>", "(II)V");

  oix = (jint) objid.oix;
  vid = (jint) objid.vid;
  objid_obj = env->NewObject( PwrtObjid_id, PwrtObjid_cid, 
    	oix, vid);

  
  
  time_AtoAscii( &time, time_eFormat_ComprDateAndTime, time_str, sizeof(time_str));
  time_AtoAscii( &birthTime, time_eFormat_ComprDateAndTime, birthTime_str, sizeof(birthTime_str));
  time_AtoAscii( &targetBirthTime, time_eFormat_ComprDateAndTime, targetBirthTime_str, sizeof(targetBirthTime_str));
  
  //g�r om till Java-str�ngar
  jevText = env->NewStringUTF( " "); //eventText anv�nds inte vid ack
  jevName = env->NewStringUTF( MsgP->Info.EventName);
  jevTime = env->NewStringUTF( time_str);
  jevBirthTime = env->NewStringUTF( birthTime_str);
  jevTargetBirthTime = env->NewStringUTF( targetBirthTime_str);
    
  //g�r om till Java-int
  jevFlags = (jint)MsgP->Info.EventFlags;
  jevPrio = (jint)MsgP->Info.EventPrio;
  jevStatus = (jint)1; //finns ej i mh_sAck och anv�nds ej heller  
  jevNix = (jint)MsgP->Info.Id.Nix;
  jevIdx = (jint)MsgP->Info.Id.Idx;

  jevTargetNix = (jint)MsgP->TargetId.Nix;
  jevTargetIdx = (jint)MsgP->TargetId.Idx;

  jevType = (jint)MsgP->Info.EventType;

  jclass MhrEvent_id = env->FindClass( "jpwr/rt/MhrEvent");
  jmethodID MhrEvent_cid = env->GetMethodID( MhrEvent_id,
    	"<init>",   "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IIIILjava/lang/String;IILjava/lang/String;IILjpwr/rt/PwrtObjid;)V");


  //anropa callback metoden i Mh-klassen
  jobject return_obj = env->NewObject( MhrEvent_id, MhrEvent_cid, jevText, jevName,
                                jevTime, jevFlags, jevPrio, jevStatus, jevNix, jevBirthTime, jevIdx,
				jevTargetNix, jevTargetBirthTime, jevTargetIdx,
				jevType, objid_obj);
  //important:check if an exception was raised 
  if (env->ExceptionCheck())
  {
    printf("exception i ack\n");
    return NULL;
  }
  
  return return_obj;
}

static void gdh_ConvertUTFstring( char *out, char *in)
{
  char *s, *t;

  s = in;
  t = out;
  while ( *s)
  {
    if ( *s == -61)
    {
      if ( *(s+1) == -91)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -92)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -74)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -123)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -124)
      {
        *t = '�';
        s++;
      }
      else if ( *(s+1) == -106)
      {
        *t = '�';
        s++;
      }
      else
        *t = *s;
    }   
    else
     *t = *s;
    s++;
    t++;
  }
  *t = 0;
}



