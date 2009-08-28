/* 
 * Proview   $Id: xtt_hist.cpp,v 1.13 2007-01-04 08:22:47 claes Exp $
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

/* xtt_hist.cpp -- Historical event window in xtt

   Author: Jonas Nylund.
   Last modification: 030217
*/

#if defined OS_LINUX

using namespace std;

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "co_wow.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_mh.h"
#include "rt_mh_outunit.h"
#include "rt_mh_util.h"
#include "rt_elog.h"
#include "co_dcli.h"
#include <db.h>
}

#include <deque>
#include <algorithm>
#include "co_lng.h"
#include "xtt_hist.h"
#include "rt_xnav_msg.h"
#include "xtt_evlist.h"

#define SENS 	1
#define INSENS  0
#define DONT_SET_SENS -1
/* 24 hours in seconds */
#define ONEDAY 86400


pwr_tStatus mh_clear_alarmlist_bc( pwr_tNodeIndex nix);
bool compDate(sEvent ev1, sEvent ev2);

Hist::Hist( void *hist_parent_ctx,
	    char *hist_name, pwr_tObjid objid,
	    pwr_tStatus *status) :
  parent_ctx(hist_parent_ctx),
  start_trace_cb(NULL), display_in_xnav_cb(NULL), update_info_cb(NULL),
  help_cb(NULL), hist(NULL), minTime_str(NULL), maxTime_str(NULL),
  eventName_str(NULL), eventText_str(NULL), eventPrio_A(false),
  eventPrio_B(false), eventPrio_C(false), eventPrio_D(false),
  eventType_Ack(false), eventType_Alarm(false), eventType_Info(false),
  eventType_Return(false), eventType_Cancel(false), eventType_Block(false),
  eventType_Unblock(false), eventType_Reblock(false),
  eventType_CancelBlock(false)
{
}


//
//  Delete hist
//
Hist::~Hist()
{
}

void Hist::hist_start_trace_cb( void *ctx, pwr_tObjid objid, char *name)
{
  if ( ((Hist *)ctx)->start_trace_cb)
    ((Hist *)ctx)->start_trace_cb( ((Hist *)ctx)->parent_ctx, objid, name);
}


void Hist::hist_popup_menu_cb( void *ctx, pwr_sAttrRef attrref,
			      unsigned long item_type, unsigned long utility,
			      char *arg, int x, int y)
{
  if ( ((Hist *)ctx)->popup_menu_cb)
    (((Hist *)ctx)->popup_menu_cb) ( ((Hist *)ctx)->parent_ctx, attrref, item_type,
				   utility, arg, x, y);
}

void Hist::hist_display_in_xnav_cb( void *ctx, pwr_sAttrRef *arp)
{
  if ( ((Hist *)ctx)->display_in_xnav_cb)
    ((Hist *)ctx)->display_in_xnav_cb( ((Hist *)ctx)->parent_ctx, arp);
}

void Hist::activate_print()
{
  char filename[200];
  dcli_translate_filename( filename, "$pwrp_tmp/xnav.ps");

  hist->print( filename);
}

void Hist::activate_help()
{
  if ( help_cb)
    (help_cb)( parent_ctx, "opg_eventlog");
}

void Hist::activate_helpevent()
{
  char	eventname[80];
  int 	sts;
  ItemAlarm *item;

  if ( help_cb) {
    sts = hist->get_selected_event( eventname, &item);
    if( ODD(sts)) {
      wow->DisplayText( eventname, item->eventmoretext);
    }
  }
}

void Hist::today_cb()
{
  int	    Sts;
  pwr_tTime StartTime;
  pwr_tTime StopTime;

  Sts = time_GetTime( &StopTime);
  Sts = AdjustForDayBreak( this, &StopTime, &StartTime);

  StopTime = StartTime;
  StopTime.tv_sec += ONEDAY;

  if ( time_Acomp( &StartTime, &StopTime) > 0)
    StartTime.tv_sec -= ONEDAY;

  SetListTime( StartTime, StopTime, INSENS);
}

void Hist::yesterday_cb()
{
  int	    Sts;
  pwr_tTime StartTime;
  pwr_tTime StopTime;

  Sts = time_GetTime( &StopTime);
  Sts = AdjustForDayBreak( this, &StopTime, &StartTime);

  if ( time_Acomp( &StartTime, &StopTime) > 0)
    StartTime.tv_sec -= ONEDAY;

  StopTime = StartTime;
  StartTime.tv_sec -= ONEDAY;
    
  SetListTime( StartTime, StopTime, INSENS);
}

void Hist::thisw_cb()
{
  int	    Sts;
  pwr_tTime CurrTime;
  pwr_tTime StartTime;
  pwr_tTime StopTime;

  Sts = time_GetTime( &CurrTime);

  Sts = GoBackWeek( CurrTime, &StartTime, &StopTime);

  StopTime.tv_sec += ONEDAY;
  CurrTime.tv_sec += ONEDAY;
  
  SetListTime( StopTime, CurrTime, INSENS);
}

void Hist::lastw_cb()
{
  int	    Sts;
  pwr_tTime CurrTime;
  pwr_tTime StartTime;
  pwr_tTime StopTime;

  Sts = time_GetTime( &CurrTime);

  Sts = GoBackWeek( CurrTime, &StartTime, &StopTime);

  SetListTime( StartTime, StopTime, INSENS);

}

void Hist::thism_cb()
{
  int	    Sts;
  pwr_tTime CurrTime;
  pwr_tTime StartTime;
  pwr_tTime StopTime;

  Sts = time_GetTime( &CurrTime);

  Sts = GoBackMonth( CurrTime, &StartTime, &StopTime);

  StopTime.tv_sec += ONEDAY;
  CurrTime.tv_sec += ONEDAY;
  
  SetListTime( StopTime, CurrTime, INSENS);

}

void Hist::lastm_cb()
{
  int	    Sts;
  pwr_tTime CurrTime;
  pwr_tTime StartTime;
  pwr_tTime StopTime;

  Sts = time_GetTime( &CurrTime);

  Sts = GoBackMonth( CurrTime, &StartTime, &StopTime);

  SetListTime( StartTime, StopTime, INSENS);
}

void Hist::all_cb()
{
  int	    Sts;
  pwr_tTime StartTime;
  pwr_tTime StopTime;
  char timestr[32] = "01-JAN-1970 00:00:00";

  time_AsciiToA(timestr, &StartTime);
    
  Sts = time_GetTime( &StopTime);

  StopTime.tv_sec += ONEDAY;
  
  SetListTime( StartTime, StopTime, INSENS);
}

void Hist::time_cb()
{
  int	    Sts;
  pwr_tTime StartTime;
  pwr_tTime StopTime;
  char timestr[32] = "01-JAN-1970 00:00:00";

  time_AsciiToA(timestr, &StartTime);
    
  Sts = time_GetTime( &StopTime);

  SetListTime( StartTime, StopTime, SENS);

}




pwr_tStatus Hist::hist_add_ack_mess( mh_sAck *MsgP)
{
  // Insert in hist
  hist->event_ack( MsgP);
  return 1;
}

pwr_tStatus Hist::hist_add_return_mess( mh_sReturn *MsgP)
{
  // Insert in hist
  hist->event_return( MsgP);
  return 1;
}

pwr_tStatus Hist::hist_add_alarm_mess( mh_sMessage *MsgP)
{
  hist->event_alarm( MsgP);
  return 1;
}

pwr_tStatus Hist::hist_add_info_mess( mh_sMessage *MsgP)
{
  hist->event_info( MsgP);
  return 1;
}

pwr_tStatus Hist::hist_clear_histlist()
{
//  hist->event_clear_list();
  return 1;
}

void Hist::get_hist_list()
{
  DB *dataBaseP = NULL;
  pwr_tInt32 ret, sts;
  char dbName[200];
  
  //printf("I get_hist_list\n");
  
  dcli_translate_filename( dbName, DATABASE);
  
  switch(Lng::lang)
  {
    case lng_eLanguage_sv_se:
      printSearchStr_sv_se();
      break;
    default:
      printSearchStr_en_us();
      break;
  }  

  /*create the database if it's not already created*/
  if((ret = db_create(&dataBaseP, NULL, 0)) != 0)
  {
    /*error creating db-handle send the mess to errh, then exit*/
    printf("error db_create: %s\n", db_strerror(ret));
    printf(" Fel vid skapande av databashandtag avslutar\n");
    return;
  }

#if DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR > 0
  //        int  (*open) __P((DB *, DB_TXN *,
  //              const char *, const char *, DBTYPE, u_int32_t, int));

  ret = dataBaseP->open(dataBaseP, NULL, dbName, NULL, DATABASETYPE, DB_RDONLY, 0);
#else
  ret = dataBaseP->open(dataBaseP, dbName, NULL, DATABASETYPE, DB_RDONLY, 0);
#endif
  if(ret != 0)
  {
    /*error opening/creating db send the mess to errh, then exit*/
    printf("error db_open: %s\n", db_strerror(ret));
    //goto err;
  }

  pwr_tUInt32 nrOfEvents = 0;
  sEvent *eventp;
  DBT data, key;
  DBC *dbcp;
  deque<sEvent> evDeq;
    
  /* Acquire a cursor for the database. */ 
  if ((ret = dataBaseP->cursor(dataBaseP, NULL, &dbcp, 0)) != 0) 
  {
    printf("error dataBaseP->cursor: %s\n", db_strerror(ret)); 
    return;
  }

  /* Initialize the key/data return pair. */
  memset(&key, 0, sizeof(key));
  memset(&data, 0, sizeof(data));
  
  if((ret = dbcp->c_get(dbcp, &key, &data, DB_FIRST)) == 0)
  {
    eventp = (sEvent *)data.data;
    sts = check_conditions(eventp);
    if(sts == ERROR_TIME_CONVERT)
    {
      printf("Error trying to convert userinput in time-field\n");
    }
    if(ODD(sts))
    {
      nrOfEvents++;
      evDeq.push_front((sEvent)*eventp);
    }
    
      
  }
  
  while((ret = dbcp->c_get(dbcp, &key, &data, DB_NEXT)) == 0)
  {
    eventp = (sEvent *)data.data;
    sts = check_conditions(eventp);
    if(ODD(sts))
    {
      nrOfEvents++;
      evDeq.push_front(*eventp);
    }
    else if(sts == ERROR_TIME_CONVERT)
    {
      printf("Error trying to convert userinput in time-field\n");
    }
  }
  if(ret != DB_NOTFOUND)
  {
    printf("error dbcp->c_get: %s\n", db_strerror(ret));
    printf("Fel vid f�rs�k att l�sa post nr %u, avslutar\n", nrOfEvents);
  }

  set_num_of_events( nrOfEvents);

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
    
  deque<sEvent>::iterator it;
  it = evDeq.begin();
  //sort the deque
  sort(it, evDeq.end(), compDate);
  hist->set_nodraw();
  while(it != evDeq.end())
  {
    switch (it->EventType) 
    {
      case mh_eEvent_Alarm:
        hist_add_alarm_mess( &(it->Mess.message) );
        break;
      case mh_eEvent_Info:
        hist_add_info_mess( &(it->Mess.message) );
        break;
      case mh_eEvent_Ack:
        hist_add_ack_mess( &(it->Mess.ack) );
        break;
      case mh_eEvent_Cancel:
        //strcpy(ret, "Cancel");
        break;
      case mh_eEvent_Return:
        hist_add_return_mess( &(it->Mess.ret) );
        break;
      case mh_eEvent_Block:
        //strcpy(ret, "Block");
        break;
      case mh_eEvent_Unblock:
        //strcpy(ret, "Unblock");
        break;
      case mh_eEvent_Reblock:
        //strcpy(ret, "Reblock");
        break;
      case mh_eEvent_CancelBlock:
        //strcpy(ret, "CancelBlock");
        break;
      default:
        //strcpy(ret, " ");
        break;
    }
    
    it++;
  }
  hist->reset_nodraw();   
}

//sorting function that might be more complicated in the future
//we might want to sort the display of the events with more conditions than 
//just the time
bool compDate(sEvent ev1, sEvent ev2)
{
  return(time_Acomp(&(ev1.EventTime), &(ev2.EventTime)) > 0);
}

int Hist::check_conditions(sEvent *evp)
{
  pwr_tTime minTime;
  pwr_tTime maxTime;
  int sts;

  //first we compare the time
  if( (minTime_str != NULL) && (strlen(minTime_str) != 0) )
  {
    sts = time_FormAsciiToA(minTime_str, SWE, SECOND, &minTime);
    if(EVEN(sts)) 
      return ERROR_TIME_CONVERT;
    if(time_Acomp(&minTime, &(evp->EventTime)) > 0)
      return 2; //evensts
  }
  if(maxTime_str != NULL && (strlen(maxTime_str) != 0))
  {
    sts = time_FormAsciiToA(maxTime_str, SWE, SECOND, &maxTime);
    if(EVEN(sts)) 
      return ERROR_TIME_CONVERT;
    if(time_Acomp(&maxTime, &(evp->EventTime)) < 0)
      return 2; //evensts
  }
  bool ret = false;
  //then we compare the EventType if nothing is selected everything is selected
  if(eventType_Ack || eventType_Alarm || eventType_Info || eventType_Return || eventType_Cancel || eventType_Block ||
     eventType_Unblock || eventType_Reblock || eventType_CancelBlock)
  {
    switch(evp->EventType) 
    {
      case mh_eEvent_Alarm:
        if(!eventType_Alarm)
	  ret = true;
        break;
      case mh_eEvent_Info:
        if(!eventType_Info)
	  ret = true;
        break;
      case mh_eEvent_Ack:
        if(!eventType_Ack)
	  ret = true;
        break;
      case mh_eEvent_Cancel:
        if(!eventType_Cancel)
	  ret = true;
        break;
      case mh_eEvent_Return:
        if(!eventType_Return)
	  ret = true;
        break;
      case mh_eEvent_Block:
        if(!eventType_Block)
	  ret = true;
        break;
      case mh_eEvent_Unblock:
        if(!eventType_Unblock)
	  ret = true;
        break;
      case mh_eEvent_Reblock:
        if(!eventType_Reblock)
	  ret = true;
        break;
      case mh_eEvent_CancelBlock:
        if(!eventType_CancelBlock)
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
  if(this->eventPrio_A || this->eventPrio_B || this->eventPrio_C || this->eventPrio_D)
  {
    //compare the EventPrio
    switch (msgInfop->EventPrio) {
      case mh_eEventPrio_A:
        if(!eventPrio_A)
          return 2;
        break;
      case mh_eEventPrio_B:
        if(!eventPrio_B)
          return 2;
        break;
      case mh_eEventPrio_C:
        if(!eventPrio_C)
          return 2;
        break;
      case mh_eEventPrio_D:
        if(!eventPrio_D)
          return 2;
        break;
      default:
        return 2;
    }
  }

  //compare the EventName
  if(eventName_str != NULL && (strlen(eventName_str) != 0) )
  {
    if( EVEN( compareStr(msgInfop->EventName, eventName_str) ) )
      return 2;
  }
  if(eventText_str != NULL && (strlen(eventText_str) != 0) )
  {
    if(mp != NULL)
    {
      if( EVEN( compareStr(mp->EventText, eventText_str) ) )
        return 2;
    }
    else if(rp != NULL)
    {
      if( EVEN( compareStr(rp->EventText, eventText_str) ) )
        return 2;
    }
    else
      return 2;
  }
  
  //every condition was true so the mess is alright
  return 1;
  
}

int Hist::compareStr(char *ev, char *usr)
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

void Hist::printSearchStr_en_us()
{
  //return;
  bool addAnd = false;
  bool prioPrinted = false;
  int i = 0;
  string searchStr[4];
  searchStr[0] = " All events";
  searchStr[1] = "";
  searchStr[2] = "";
  searchStr[3] = "";

  if((minTime_str != NULL) && (strlen(minTime_str) > 0) )  
  {
    searchStr[i] += " From ";
    searchStr[i] += minTime_str;
    addAnd = true;
  }
  if((maxTime_str != NULL) && (strlen(maxTime_str) > 0) )  
  {
    searchStr[i] += " To ";
    searchStr[i] += maxTime_str;
    addAnd = true;
  }
  if(eventPrio_A || eventPrio_B || eventPrio_C || eventPrio_D)
  {
    if(addAnd) searchStr[i] += " and";
    i++;
    prioPrinted = true;
    searchStr[i] += " with prio ";
    if(eventPrio_A)
      searchStr[i] += "A";
    if(eventPrio_B)
      searchStr[i] += "B";
    if(eventPrio_C)
      searchStr[i] += "C";
    if(eventPrio_D)
      searchStr[i] += "D";
    addAnd = true;
   }
  if(eventType_Ack || eventType_Alarm || eventType_Info || eventType_Return)
  {
    if(addAnd) searchStr[i] += " and";
    if(!prioPrinted)
      i++;
    searchStr[i] += " with type ";
    if(eventType_Ack)
      searchStr[i] += "Ack ";
    if(eventType_Alarm)
      searchStr[i] += "Active ";
    if(eventType_Info)
      searchStr[i] += "Info ";
    if(eventType_Return)
      searchStr[i] += "Return";
    addAnd = true;
  }
  if((eventName_str != NULL) && (strlen(eventName_str) > 0) )  
  {
    if(addAnd) searchStr[i] += " and";
    i++;
    searchStr[i] += " with EventName ";
    searchStr[i] += eventName_str;
    addAnd = true;
  }
  if((eventText_str != NULL) && (strlen(eventText_str) > 0) )  
  {
    if(addAnd) searchStr[i] += " and";
    i++;
    searchStr[i] += " with EventText ";
    searchStr[i] += eventText_str;
  }

  set_search_string( searchStr[0].c_str(), searchStr[1].c_str(),
		     searchStr[2].c_str(), searchStr[3].c_str());
}

void Hist::printSearchStr_sv_se()
{
  //return;
  bool addAnd = false;
  bool prioPrinted = false;
  int i = 0;
  string searchStr[4];
  searchStr[0] = " Alla h�ndelser";
  searchStr[1] = "";
  searchStr[2] = "";
  searchStr[3] = "";
  if((minTime_str != NULL) && (strlen(minTime_str) > 0) )  
  {
    searchStr[i] += " Fr�n ";
    searchStr[i] += minTime_str;
    addAnd = true;
  }
  if((maxTime_str != NULL) && (strlen(maxTime_str) > 0) )  
  {
    searchStr[i] += " till ";
    searchStr[i] += maxTime_str;
    addAnd = true;
  }
  if(eventPrio_A || eventPrio_B || eventPrio_C || eventPrio_D)
  {
    if(addAnd) searchStr[i] += " och";
    prioPrinted = true;
    i++;
    searchStr[i] += " med prio ";
    if(eventPrio_A)
      searchStr[i] += "A";
    if(eventPrio_B)
      searchStr[i] += "B";
    if(eventPrio_C)
      searchStr[i] += "C";
    if(eventPrio_D)
      searchStr[i] += "D";
    addAnd = true;
   }
  if(eventType_Ack || eventType_Alarm || eventType_Info || eventType_Return)
  {
    if(addAnd) searchStr[i] += " och";
    if(!prioPrinted)
      i++;
    searchStr[i] += " med typ ";
    if(eventType_Ack)
      searchStr[i] += "Kvittens ";
    if(eventType_Alarm)
      searchStr[i] += "Aktiv ";
    if(eventType_Info)
      searchStr[i] += "Info ";
    if(eventType_Return)
      searchStr[i] += "Retur";
    addAnd = true;
  }
  if((eventName_str != NULL) && (strlen(eventName_str) > 0) )  
  {
    if(addAnd) searchStr[i] += " och";
    i++;
    searchStr[i] += " med H�ndelsenamn ";
    searchStr[i] += eventName_str;
    addAnd = true;
  }
  if((eventText_str != NULL) && (strlen(eventText_str) > 0) )  
  {
    if(addAnd) searchStr[i] += " och";
    i++;
    searchStr[i] += " med h�ndelsetext ";
    searchStr[i] += eventText_str;
  }
  set_search_string( searchStr[0].c_str(), searchStr[1].c_str(),
		     searchStr[2].c_str(), searchStr[3].c_str());
}





/************************************************************************
*
* Name:	GoBackMonth (TimeIn, FromTime, ToTime)
*
* Type:	int
*
* TYPE		PARAMETER	IOGF	DESCRIPTION
* pwr_tTime	TimeIn		I	Start time	
* pwr_tTime	*FromTime	O	The first day of the month
* pwr_tTime     *ToTime		O	The last day of the month
*
* Description:	This function computes dates for the first to the last day
*		of the previous month, from the time TimeIn.
*		Output times are only date, e g 1-MAY-1992 00:00:00.00.
*************************************************************************/
int  Hist::GoBackMonth( pwr_tTime TimeIn, pwr_tTime *FromTime,
			pwr_tTime *ToTime)
{
    struct tm	*Tm;
    int		DaysOfMonth, Month, TmYear, Year;

    /* Get the time in and blank time values. */

    time_t sec = TimeIn.tv_sec;
    Tm = localtime(&sec);

    /* Get number of days in the previous month. */
    if (Tm->tm_mon == 0)
    {
	Month = 11;
	TmYear = Tm->tm_year - 1;
    }
    else
    {
	Month = Tm->tm_mon - 1;
	TmYear = Tm->tm_year;
    }


    switch (Month)
    {
	case 0:
	case 2:
	case 4:
	case 6:
	case 7:
	case 9:
	case 11:
	    DaysOfMonth = 31;
	    break;

	case 3:
	case 5:
	case 8:
	case 10:
	    DaysOfMonth = 30;
	    break;

	case 1: 
    	    Year = TmYear + 1900;
	    if ((Year % 4 == 0 && Year % 100 != 0) || (Year + 1900) % 400 == 0)
		DaysOfMonth = 29;
	    else
		DaysOfMonth = 28;
	    break;
    }

    memset(Tm, 0, sizeof(*Tm));
    Tm->tm_mday = 1; 
    Tm->tm_mon = Month; 
    Tm->tm_year = TmYear; 

    FromTime->tv_sec = mktime(Tm);
    FromTime->tv_nsec = 0;

    Tm->tm_mday = DaysOfMonth; 
    ToTime->tv_sec = mktime(Tm);
    ToTime->tv_nsec = 0;

    return(1);
}   /* END GoBackMonth */


/************************************************************************
*
* Name:	GoBackWeek( TimeIn, FromTime, ToTime)
*
* Type:	int
*
* TYPE		PARAMETER	IOGF	DESCRIPTION
* pwr_tTime	TimeIn		I	Start time	
* pwr_tTime	*FromTime	O	The first day of the month
* pwr_tTime     *ToTime		O	The last day of the month
*
* Description:	This function computes dates for monday to sunday in the
*		previous week from the time now.
*		Output times are only date, e g 1-MAY-1992 00:00:00.00.
*************************************************************************/
int Hist::GoBackWeek( pwr_tTime TimeIn, pwr_tTime *FromTime,
		      pwr_tTime *ToTime)
{
    struct tm	*Tm;
    int		Days;
    pwr_tTime   Time;

    time_t sec = TimeIn.tv_sec;
    Tm = localtime(&sec);
    if (Tm->tm_wday == 0) /* Sunday */
	Days = 13;
    else
    	Days = Tm->tm_wday + 6;

    Tm->tm_sec = 0;
    Tm->tm_min = 0;
    Tm->tm_hour = 0;
    Time.tv_sec = mktime(Tm);

    FromTime->tv_sec = Time.tv_sec - Days * ONEDAY;
    FromTime->tv_nsec = 0;

    ToTime->tv_sec = Time.tv_sec - (Days - 6) * ONEDAY;
    ToTime->tv_nsec = 0;

    return(1);
}   /* END GoBackWeek */


/************************************************************************
*
* Name: AdjustForDayBreak
*
* Type: void
*
* Type          Parameter       IOGF    Description
* pwr_tTime     *Time           I       The time
* pwr_tTime	*NewTime  	 O	The adjusted time
*
* Description:
*       
*
*************************************************************************/
pwr_tStatus Hist::AdjustForDayBreak( Hist *histOP, pwr_tTime *Time, 
				      pwr_tTime *NewTime)
{
    pwr_tStatus Sts;
    char timestr[32]; 

    Sts = time_AtoAscii(Time, time_eFormat_DateAndTime, timestr, sizeof(timestr));
    sprintf(&timestr[12],"00:00:00");

    return time_AsciiToA(timestr, NewTime);

} /* AdjustForDayBreak */

#endif

















