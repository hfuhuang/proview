/* 
 * Proview   $Id: xtt_hist_gtk.h,v 1.1 2007-01-04 08:29:32 claes Exp $
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

#ifndef xtt_hist_gtk_h
#define xtt_hist_gkt_h

/* xtt_hist_gtk.h -- Historical event window in xtt */

#if defined OS_LINUX

#ifndef xtt_hist_h
# include "xtt_hist.h"
#endif

class HistGtk : public Hist {
 public:
  HistGtk( void *hist_parent_ctx,
	   GtkWidget *hist_parent_wid,
	   char *hist_name, pwr_tObjid objid,
	   pwr_tStatus *status);
  ~HistGtk();

  GtkWidget		*parent_wid;
  GtkWidget		*parent_wid_hist;
  GtkWidget		*toplevel_hist;
  GtkWidget		*toplevel_search;
  GtkWidget		*form_hist;
  GtkWidget		*hist_widget;  
  GtkWidget		*start_time_help_lbl_w;
  GtkWidget		*start_time_entry_w;
  GtkWidget		*stop_time_entry_w;    
  GtkWidget		*event_text_entry_w;
  GtkWidget		*event_name_entry_w;
  GtkWidget		*alarm_toggle_w;
  GtkWidget		*info_toggle_w;
  GtkWidget		*ack_toggle_w;
  GtkWidget		*ret_toggle_w;
  GtkWidget		*prioA_toggle_w;
  GtkWidget		*prioB_toggle_w;
  GtkWidget		*prioC_toggle_w;
  GtkWidget		*prioD_toggle_w;
  GtkWidget		*nrofevents_string_lbl_w;
  GtkWidget		*search_string_lbl_w;
  GtkWidget		*search_string2_lbl_w;
  GtkWidget		*search_string3_lbl_w;
  GtkWidget		*search_string4_lbl_w;            

  void set_num_of_events( int nrOfEvents);
  void set_search_string( const char *s1, const char *s2, 
			  const char *s3, const char *s4);
  void SetListTime( pwr_tTime StartTime, pwr_tTime StopTime, 
		    int Sensitive);

  static gboolean action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data);
  static void activate_exit( GtkWidget *w, gpointer data);
  static void activate_print( GtkWidget *w, gpointer data);
  static void activate_zoom_in( GtkWidget *w, gpointer data);
  static void activate_zoom_out( GtkWidget *w, gpointer data);
  static void activate_zoom_reset( GtkWidget *w, gpointer data);
  static void activate_open_plc( GtkWidget *w, gpointer data);
  static void activate_display_in_xnav( GtkWidget *w, gpointer data);
  static void activate_disp_hundredth( GtkWidget *w, gpointer data);
  static void activate_hide_object( GtkWidget *w, gpointer data);
  static void activate_hide_text( GtkWidget *w, gpointer data);
  static void activate_help( GtkWidget *w, gpointer data);
  static void activate_helpevent( GtkWidget *w, gpointer data);
  static void create_form( GtkWidget *w, gpointer data);
  static void ok_btn( GtkWidget *w, gpointer data);
  //callbackfunctions from the searchdialog
  static void cancel_cb( GtkWidget *w, gpointer data);
  static void today_cb( GtkWidget *w, gpointer data);
  static void yesterday_cb( GtkWidget *w, gpointer data);
  static void thisw_cb( GtkWidget *w, gpointer data);
  static void lastw_cb( GtkWidget *w, gpointer data);
  static void thism_cb( GtkWidget *w, gpointer data);
  static void lastm_cb( GtkWidget *w, gpointer data);
  static void all_cb( GtkWidget *w, gpointer data);
  static void time_cb( GtkWidget *w, gpointer data);
  
  
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
