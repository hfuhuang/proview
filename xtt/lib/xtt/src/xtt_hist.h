/* 
 * Proview   $Id: xtt_hist.h,v 1.6 2007-01-04 08:22:47 claes Exp $
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

#ifndef xtt_hist_h
#define xtt_hist_h

/* xtt_hist.h -- Historical event window in xtt */

#if defined OS_LINUX

#if defined __cplusplus
extern "C" {
#include "rt_elog.h"
}
#endif

#ifndef pwr_h
# include "pwr.h"
#endif

#ifndef xtt_evlist
# include "xtt_evlist.h"
#endif
#define ERROR_TIME_CONVERT  -99

class Hist {
  public:
    Hist( void *hist_parent_ctx,
	  char *hist_name, pwr_tObjid objid,
	  pwr_tStatus *status);
    virtual ~Hist();

    void 		*parent_ctx;
    char 		name[80];
    pwr_tObjid		user;
    int			hist_display_ack;
    int			hist_display_return;
    void 		(*close_cb)( void *);
    void 		(*start_trace_cb)( void *, pwr_tObjid, char *);
    void 		(*display_in_xnav_cb)( void *, pwr_sAttrRef *);
    void 		(*update_info_cb)( void *);
    void 		(*help_cb)( void *, char *);
    void 		(*popup_menu_cb)( void *, pwr_sAttrRef, unsigned long,
					  unsigned long, char *, int x, int y);
    EvList		*hist;

    int			hist_size;
    char 		*minTime_str;
    char 		*maxTime_str;
    char 		*eventName_str;
    char 		*eventText_str;
    
    bool		eventPrio_A;
    bool		eventPrio_B;
    bool		eventPrio_C;
    bool		eventPrio_D;
    
    bool		eventType_Ack;
    bool		eventType_Alarm;
    bool		eventType_Info;
    bool		eventType_Return;
    bool		eventType_Cancel;
    bool		eventType_Block;
    bool		eventType_Unblock;
    bool		eventType_Reblock;
    bool		eventType_CancelBlock;
    
    void		get_hist_list();
    pwr_tStatus 	hist_add_ack_mess( mh_sAck *MsgP);
    pwr_tStatus 	hist_add_return_mess( mh_sReturn *MsgP);
    pwr_tStatus 	hist_add_alarm_mess( mh_sMessage *MsgP);
    pwr_tStatus 	hist_add_info_mess( mh_sMessage *MsgP);
    pwr_tStatus 	hist_clear_histlist();
    int			check_conditions(sEvent *evp);
    int			compareStr(char *, char *);
    void		printSearchStr_en_us();
    void		printSearchStr_sv_se();

    virtual void set_num_of_events( int nrOfEvents) {}
    virtual void set_search_string( const char *s1, const char *s2, 
				    const char *s3, const char *s4) {}
    virtual void SetListTime( pwr_tTime StartTime, pwr_tTime StopTime, 
			      int Sensitive) {}


    void activate_print();
    void activate_help();
    void activate_helpevent();
    void today_cb();
    void yesterday_cb();
    void lastw_cb();
    void thism_cb();
    void lastm_cb();
    void thisw_cb();
    void all_cb();
    void time_cb();
    
    static int GoBackMonth( pwr_tTime TimeIn, pwr_tTime *FromTime,
			    pwr_tTime *ToTime);
    static int GoBackWeek( pwr_tTime TimeIn, pwr_tTime *FromTime,
			   pwr_tTime *ToTime);
    static void SetListTime( Hist *histOP, pwr_tTime StartTime, 
			     pwr_tTime StopTime, int Sensitive);
    static pwr_tStatus AdjustForDayBreak( Hist *histOP, pwr_tTime *Time, 
					  pwr_tTime *NewTime);

    static void hist_display_in_xnav_cb( void *ctx, pwr_sAttrRef *arp);
    static void hist_start_trace_cb( void *ctx, pwr_tObjid objid, char *name);
    static void hist_popup_menu_cb( void *ctx, pwr_sAttrRef attrref,
			      unsigned long item_type, unsigned long utility,
			      char *arg, int x, int y);
};

#else
// Dummy for other platforms then OS_LINUX
class Hist {
  public:
    Hist(
	void *hist_parent_ctx,
	Widget	hist_parent_wid,
	char *hist_name, pwr_tObjid objid,
	pwr_tStatus *status) : parent_ctx(hist_parent_ctx) {}
    void 		*parent_ctx;
    void 		(*close_cb)( void *);
    void 		(*start_trace_cb)( void *, pwr_tObjid, char *);
    void 		(*display_in_xnav_cb)( void *, pwr_tObjid);
    void 		(*update_info_cb)( void *);
    void 		(*help_cb)( void *, char *);
    void 		(*popup_menu_cb)( void *, pwr_sAttrRef, unsigned long,
					  unsigned long, char *, Widget * );
};

#endif

#endif
