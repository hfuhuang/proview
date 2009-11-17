/* 
 * Proview   $Id: xtt_hist_motif.cpp,v 1.2 2008-10-31 12:51:36 claes Exp $
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

/* xtt_hist_motif.cpp -- Historical event window in xtt

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
#include "pwr_baseclasses.h"
#include "rt_gdh.h"
#include "rt_mh.h"
#include "rt_mh_outunit.h"
#include "rt_mh_util.h"
#include "rt_elog.h"
#include "co_dcli.h"
#include <db.h>
}

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/Text.h>
#include <Mrm/MrmPublic.h>
#include <Xm/ToggleB.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <deque>
#include <algorithm>

extern "C" {
#include "flow_x.h"
#include "cow_mrm_util.h"
}
#include "co_lng.h"
#include "xtt_hist_motif.h"
#include "rt_xnav_msg.h"
#include "xtt_evlist_motif.h"

#define SENS 	1
#define INSENS  0
#define DONT_SET_SENS -1
/* 24 hours in seconds */
#define ONEDAY 86400


HistMotif::HistMotif( void *hist_parent_ctx,
		      Widget	hist_parent_wid,
		      char *hist_name, pwr_tObjid objid,
		      pwr_tStatus *status) :
  Hist( hist_parent_ctx, hist_name, objid, status),
  parent_wid(hist_parent_wid), parent_wid_hist(NULL)
{
  char		uid_filename[120] = {"xtt_hist.uid"};
  char		*uid_filename_p = uid_filename;
  Arg 		args[20];
  pwr_tStatus	sts;
  int		i;
  MrmHierarchy s_DRMh;
  MrmType dclass;
  MrmType searchclass;

  static char hist_translations[] =
    "<FocusIn>: hist_inputfocus()\n";

  static XtTranslations hist_compiled_translations = NULL;

  static XtActionsRec hist_actions[] =
  {
    {(char*) "hist_inputfocus",      (XtActionProc) action_inputfocus}
  };


  static MrmRegisterArg	reglist[] = {
        {(char*) "hist_ctx", 0 },
	{(char*) "hist_activate_exit",(caddr_t)activate_exit },
	{(char*) "hist_activate_print",(caddr_t)activate_print },
	{(char*) "hist_activate_zoom_in",(caddr_t)activate_zoom_in },
	{(char*) "hist_activate_zoom_out",(caddr_t)activate_zoom_out },
	{(char*) "hist_activate_zoom_reset",(caddr_t)activate_zoom_reset },
	{(char*) "hist_activate_open_plc",(caddr_t)activate_open_plc },
	{(char*) "hist_activate_display_in_xnav",(caddr_t)activate_display_in_xnav },
	{(char*) "hist_activate_disp_hundredth",(caddr_t)activate_disp_hundredth },
	{(char*) "hist_activate_hide_object",(caddr_t)activate_hide_object },
	{(char*) "hist_activate_hide_text",(caddr_t)activate_hide_text },
	{(char*) "hist_activate_help",(caddr_t)activate_help },
	{(char*) "hist_activate_helpevent",(caddr_t)activate_helpevent },
	{(char*) "hist_create_form",(caddr_t)create_form },
	{(char*) "hist_ok_btn",(caddr_t)ok_btn },
	{(char*) "hist_cancel_cb",(caddr_t)cancel_cb },

	{(char*) "hist_start_time_entry_cr",(caddr_t)start_time_entry_cr },
	{(char*) "hist_start_time_entry_lf",(caddr_t)start_time_entry_lf },

	{(char*) "hist_stop_time_entry_cr",(caddr_t)stop_time_entry_cr },
	{(char*) "hist_stop_time_entry_lf",(caddr_t)stop_time_entry_lf },

	{(char*) "hist_today_cb",(caddr_t)today_cb },
	{(char*) "hist_yesterday_cb",(caddr_t)yesterday_cb },
	{(char*) "hist_thisw_cb",(caddr_t)thisw_cb },
	{(char*) "hist_lastw_cb",(caddr_t)lastw_cb },
	{(char*) "hist_thism_cb",(caddr_t)thism_cb },
	{(char*) "hist_lastm_cb",(caddr_t)lastm_cb },
	{(char*) "hist_all_cb",(caddr_t)all_cb },
	{(char*) "hist_time_cb",(caddr_t)time_cb },


	{(char*) "hist_alarm_toggle_cr",(caddr_t)alarm_toggle_cr },
	{(char*) "hist_info_toggle_cr",(caddr_t)info_toggle_cr },
	{(char*) "hist_ack_toggle_cr",(caddr_t)ack_toggle_cr },
	{(char*) "hist_ret_toggle_cr",(caddr_t)ret_toggle_cr },

	{(char*) "hist_prioA_toggle_cr",(caddr_t)prioA_toggle_cr },
	{(char*) "hist_prioB_toggle_cr",(caddr_t)prioB_toggle_cr },
	{(char*) "hist_prioC_toggle_cr",(caddr_t)prioC_toggle_cr },
	{(char*) "hist_prioD_toggle_cr",(caddr_t)prioD_toggle_cr },

	{(char*) "hist_event_text_entry_cr",(caddr_t)event_text_entry_cr },
	{(char*) "hist_event_text_entry_lf",(caddr_t)event_text_entry_lf },

	{(char*) "hist_event_name_entry_cr",(caddr_t)event_name_entry_cr },
	{(char*) "hist_event_name_entry_lf",(caddr_t)event_name_entry_lf },
	{(char*) "hist_nrofevents_string_label_cr",(caddr_t)nrofevents_string_label_cr },
	{(char*) "hist_search_string_label_cr",(caddr_t)search_string_label_cr },
	{(char*) "hist_search_string2_label_cr",(caddr_t)search_string2_label_cr },
	{(char*) "hist_search_string3_label_cr",(caddr_t)search_string3_label_cr },
	{(char*) "hist_search_string4_label_cr",(caddr_t)search_string4_label_cr },
	{(char*) "hist_start_time_help_label_cr",(caddr_t)start_time_help_label_cr }

	};
  static int	reglist_num = (sizeof reglist / sizeof reglist[0]);


  
  *status = 1;

  Lng::get_uid( uid_filename, uid_filename);

  //shall be MessageHandler.EventLogSize
  hist_size = 100000;
  
  reglist[0].value = (caddr_t) this;

  // Motif
  MrmInitialize();


  // Save the context structure in the widget
  i = 0;
  XtSetArg(args[i], XmNuserData, (unsigned int) this);i++;
  XtSetArg(args[i], XmNdeleteResponse, XmDO_NOTHING);i++;

  sts = MrmOpenHierarchy( 1, &uid_filename_p, NULL, &s_DRMh);
  if (sts != MrmSUCCESS) printf("can't open %s\n", uid_filename);

  MrmRegisterNames(reglist, reglist_num);

  parent_wid_hist = XtCreatePopupShell( hist_name, 
		topLevelShellWidgetClass, parent_wid, args, i);

  sts = MrmFetchWidgetOverride( s_DRMh, (char*) "hist_window", parent_wid_hist,
			hist_name, args, 1, &toplevel_hist, &dclass);
  if (sts != MrmSUCCESS)  printf("can't fetch %s\n", hist_name);


  sts = MrmFetchWidgetOverride( s_DRMh, (char*) "histSearchDialog", form_hist,
			(char*) "searchdialog", args, 1, &toplevel_search, &searchclass);
  if (sts != MrmSUCCESS)  printf("can't fetch %s\n", "histSearchDialog");


  MrmCloseHierarchy(s_DRMh);

  if ( hist_compiled_translations == NULL) 
  {
    XtAppAddActions( XtWidgetToApplicationContext( toplevel_hist), 
		hist_actions, XtNumber(hist_actions));
    hist_compiled_translations = XtParseTranslationTable( hist_translations);
  }
  XtOverrideTranslations( toplevel_hist, hist_compiled_translations);

  i = 0;
  XtSetArg(args[i],XmNwidth,700);i++;
  XtSetArg(args[i],XmNheight,300);i++;
  XtSetValues( toplevel_search ,args,i);

  XtManageChild( toplevel_search);


  i = 0;
  XtSetArg(args[i],XmNwidth,700);i++;
  XtSetArg(args[i],XmNheight,600);i++;
  XtSetValues( toplevel_hist ,args,i);

  XtManageChild( toplevel_hist);
  
  // Create hist...
  hist = new EvListMotif( this, form_hist, ev_eType_HistList, hist_size, &hist_widget);
  hist->start_trace_cb = &hist_start_trace_cb;
  hist->display_in_xnav_cb = &hist_display_in_xnav_cb;
  hist->popup_menu_cb = &hist_popup_menu_cb;


  XtPopup( parent_wid_hist, XtGrabNone);

  // Connect the window manager close-button to exit
  flow_AddCloseVMProtocolCb( parent_wid_hist, 
	(XtCallbackProc)activate_exit, this);

  char name_str[80];
  sts = gdh_ObjidToName ( objid, name_str, sizeof(name_str), cdh_mName_pathStrict);
  if (ODD(sts))
  {
    if(this->event_name_entry_w != NULL)
    {
      XmTextSetString(this->event_name_entry_w, name_str);
      this->eventName_str = name_str;
      this->get_hist_list();
    }
  }

  wow = new CoWowMotif( parent_wid_hist);   

}


//
//  Delete hist
//
HistMotif::~HistMotif()
{
  if ( parent_wid_hist)
    XtDestroyWidget( parent_wid_hist);
  if ( hist)
    delete hist;
}

void HistMotif::action_inputfocus( Widget w, XmAnyCallbackStruct *data)
{
  Arg args[1];
  HistMotif *hist;

  XtSetArg    (args[0], XmNuserData, &hist);
  XtGetValues (w, args, 1);


  if ( mrm_IsIconicState(w))
    return;

  if ( hist->hist) {
    if ( hist->focustimer.disabled())
      return;

    hist->hist->set_input_focus();
    hist->focustimer.disable( hist->toplevel_hist, 4000);
  }
  //printf("focus\n");
  //histOP->hist->set_input_focus();
//?????????????????????
  //if ( ev && ev->hist_displayed)
  //  ev->hist->set_input_focus();
  // hist->hist->set_input_focus();

}

void HistMotif::ok_btn( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  brow_DeleteAll(histOP->hist->brow->ctx);
  histOP->eventType_Alarm = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->alarm_toggle_w);

  histOP->eventType_Ack = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->ack_toggle_w);

  histOP->eventType_Info = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->info_toggle_w);

  histOP->eventType_Return = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->ret_toggle_w);

  histOP->eventPrio_A = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->prioA_toggle_w);

  histOP->eventPrio_B = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->prioB_toggle_w);

  histOP->eventPrio_C = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->prioC_toggle_w);

  histOP->eventPrio_D = (bool)XmToggleButtonGetState( ((HistMotif *)histOP)->prioD_toggle_w);

  histOP->minTime_str = XmTextGetString( ((HistMotif *)histOP)->start_time_entry_w);

  histOP->maxTime_str = XmTextGetString( ((HistMotif *)histOP)->stop_time_entry_w);

  histOP->eventText_str = XmTextGetString( ((HistMotif *)histOP)->event_text_entry_w);

  histOP->eventName_str = XmTextGetString( ((HistMotif *)histOP)->event_name_entry_w);


  histOP->get_hist_list();
  
  XtFree(histOP->minTime_str);
  XtFree(histOP->maxTime_str);
  XtFree(histOP->eventText_str);
  XtFree(histOP->eventName_str);

}

void HistMotif::activate_exit( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->close_cb(histOP);
}

void HistMotif::activate_print( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->activate_print();
}

void HistMotif::activate_zoom_in( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->hist->zoom( 1.2);
}

void HistMotif::activate_zoom_out( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->hist->zoom( 5.0/6);
}

void HistMotif::activate_zoom_reset( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->hist->unzoom();
}

void HistMotif::activate_open_plc( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->hist->start_trace();
}

void HistMotif::activate_display_in_xnav( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->hist->display_in_xnav();
}

void HistMotif::activate_disp_hundredth( Widget w, Hist *histOP, XmToggleButtonCallbackStruct *data)
{
  histOP->hist->set_display_hundredth( data->set);
}

void HistMotif::activate_hide_object( Widget w, Hist *histOP, XmToggleButtonCallbackStruct *data)
{
  histOP->hist->set_hide_object( data->set);
}

void HistMotif::activate_hide_text( Widget w, Hist *histOP, XmToggleButtonCallbackStruct *data)
{
  histOP->hist->set_hide_text( data->set);
}

void HistMotif::activate_help( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->activate_help();
}

void HistMotif::activate_helpevent( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->activate_helpevent();
}

void HistMotif::create_form( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->form_hist = w;
}


//callbackfunctions from the searchdialog
void HistMotif::cancel_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_cancel_cb\n");
}
void HistMotif::start_time_entry_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->start_time_entry_w = w;
  XmTextSetString( ((HistMotif *)histOP)->start_time_entry_w, (char*) "1970-05-05 00:00:00");
  XtSetSensitive( ((HistMotif *)histOP)->start_time_entry_w, INSENS);
}

void HistMotif::start_time_entry_lf( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_start_time_entry_lf\n");
}

void HistMotif::stop_time_entry_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->stop_time_entry_w = w;
  char buf[80];
  int	    Sts;
  pwr_tTime StopTime;
  pwr_tTime StartTime;

  Sts = time_GetTime( &StopTime);
  Sts = AdjustForDayBreak(histOP, &StopTime, &StartTime);

  StopTime = StartTime;
  StopTime.tv_sec += ONEDAY;
  
  time_AtoFormAscii(&StopTime, SWE, SECOND, buf, sizeof(buf));
  XmTextSetString( ((HistMotif *)histOP)->stop_time_entry_w, buf);
  XtSetSensitive( ((HistMotif *)histOP)->stop_time_entry_w, INSENS);
}

void HistMotif::stop_time_entry_lf( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_stop_time_entry_lf\n");
}

void HistMotif::alarm_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->alarm_toggle_w = w;
  //printf("hist_alarm_toggle_cr\n");
}

void HistMotif::info_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->info_toggle_w = w;
  //printf("hist_info_toggle_cr\n");
}

void HistMotif::ack_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->ack_toggle_w = w;
  //printf("hist_ack_toggle_cr\n");
}

void HistMotif::ret_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->ret_toggle_w = w;
  //printf("hist_ret_toggle_cr\n");
}


void HistMotif::prioA_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->prioA_toggle_w = w;
  //printf("hist_prioA_toggle_cr\n");
}

void HistMotif::prioB_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->prioB_toggle_w = w;
  //printf("hist_prioC_toggle_cr\n");
}

void HistMotif::prioC_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->prioC_toggle_w = w;
  //printf("hist_prioC_toggle_cr\n");
}

void HistMotif::prioD_toggle_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->prioD_toggle_w = w;
  //printf("hist_prioD_toggle_cr\n");
}

void HistMotif::event_text_entry_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->event_text_entry_w = w;
  //printf("hist_event_text_entry_cr\n");
}

void HistMotif::event_text_entry_lf( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_event_text_entry_lf\n");
}

void HistMotif::event_name_entry_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  ((HistMotif *)histOP)->event_name_entry_w = w;
  //printf("hist_event_name_entry_cr\n");
}

void HistMotif::event_name_entry_lf( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_event_name_entry_lf\n");
}

void HistMotif::nrofevents_string_label_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_search_string_label_cr\n");
  ((HistMotif *)histOP)->nrofevents_string_lbl_w = w;
}

void HistMotif::search_string_label_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_search_string_label_cr\n");
  ((HistMotif *)histOP)->search_string_lbl_w = w;
}
void HistMotif::search_string2_label_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_search_string_label_cr\n");
  ((HistMotif *)histOP)->search_string2_lbl_w = w;
}
void HistMotif::search_string3_label_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_search_string_label_cr\n");
  ((HistMotif *)histOP)->search_string3_lbl_w = w;
}
void HistMotif::search_string4_label_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_search_string_label_cr\n");
  ((HistMotif *)histOP)->search_string4_lbl_w = w;
}

void HistMotif::start_time_help_label_cr( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
//  printf("hist_start_time_help_label_cr\n");
  ((HistMotif *)histOP)->start_time_help_lbl_w = w;
  char buf[40] = "Timeformat: 1970-05-05 23:30:00";
  XmString str = XmStringCreateLtoR(buf, (char*) "tag1");
  XtVaSetValues( ((HistMotif *)histOP)->start_time_help_lbl_w, XmNlabelString,str,NULL);
  XmStringFree(str);
}


void HistMotif::today_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->today_cb();
}

void HistMotif::yesterday_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->yesterday_cb();
}

void HistMotif::thisw_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->thisw_cb();
}

void HistMotif::lastw_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->lastw_cb();
}

void HistMotif::thism_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->thism_cb();
}

void HistMotif::lastm_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->lastm_cb();
}

void HistMotif::all_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->all_cb();
}

void HistMotif::time_cb( Widget w, Hist *histOP, XmAnyCallbackStruct *data)
{
  histOP->time_cb();
}

void HistMotif::set_num_of_events( int nrOfEvents)
{
  char buf[20];
  sprintf(buf, "    %u", nrOfEvents);
  XmString str = XmStringCreateLtoR(buf, (char*) "tag1");
  XtVaSetValues(this->nrofevents_string_lbl_w, XmNlabelString,str,NULL);
  XmStringFree(str);
}

void HistMotif::set_search_string( const char *s1, const char *s2, 
			      const char *s3, const char *s4)
{
  char buf[500];

  strncpy(buf, s1, sizeof(buf));
  XmString str = XmStringCreateLtoR(buf, (char*) "tag1");
  XtVaSetValues(this->search_string_lbl_w, XmNlabelString,str,NULL);
  XmStringFree(str);

  strncpy(buf, s2, sizeof(buf));
  XmString str2 = XmStringCreateLtoR(buf, (char*) "tag1");
  XtVaSetValues(this->search_string2_lbl_w, XmNlabelString,str2,NULL);
  XmStringFree(str2);

  strncpy(buf, s3, sizeof(buf));
  XmString str3 = XmStringCreateLtoR(buf, (char*) "tag1");
  XtVaSetValues(this->search_string3_lbl_w, XmNlabelString,str3,NULL);
  XmStringFree(str3);

  strncpy(buf, s4, sizeof(buf));
  XmString str4 = XmStringCreateLtoR(buf, (char*) "tag1");
  XtVaSetValues(this->search_string4_lbl_w, XmNlabelString,str4,NULL);
  XmStringFree(str4);

}


/************************************************************************
*
* Name:		SetListTime
*
* Type:		
*
* TYPE		PARAMETER	IOGF	DESCRIPTION
* 
*
* Description:	Sets the Time field for start and stop
* 
*************************************************************************/
void HistMotif::SetListTime( pwr_tTime StartTime, pwr_tTime StopTime, 
			int Sensitive)
{
  char timestr[32];

  /* Show the resulting times */
  time_AtoFormAscii(&StopTime,SWE, SECOND,timestr,sizeof(timestr));
  XmTextSetString( stop_time_entry_w, timestr);
    
 
  time_AtoFormAscii(&StartTime,SWE, SECOND,timestr,sizeof(timestr));
  XmTextSetString( start_time_entry_w, timestr);   
    
  if (Sensitive != DONT_SET_SENS) {
    XtSetSensitive( start_time_entry_w, Sensitive);
    XtSetSensitive( stop_time_entry_w, Sensitive);
  }

}/* SetListTime */

#endif

















