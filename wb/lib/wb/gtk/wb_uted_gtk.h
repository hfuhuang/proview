/* 
 * Proview   $Id: wb_uted_gtk.h,v 1.2 2008-10-31 12:51:31 claes Exp $
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
 **/

#ifndef wb_uted_gtk_h
#define wb_uted_gtk_h

#ifndef wb_uted_h
#include "wb_uted.h"
#endif

struct uted_s_widgets 
{
  GtkWidget	*uted_window;
  GtkWidget	*label;
  GtkWidget     *adb;
  GtkWidget	*file_entry;
  GtkWidget	*quit;
  GtkWidget	*batchoptmenu;
  GtkWidget	*commandlabel;
  GtkWidget	*batch;
  GtkWidget	*currsess;
  GtkWidget	*help;
  GtkWidget	*timelabel;
  GtkWidget	*timevalue;
  GtkWidget	*qualifier[UTED_QUALS];
  GtkWidget	*value[UTED_QUALS];
  GtkWidget	*present[UTED_QUALS];
  GtkWidget	*optmenubuttons[UTED_MAX_COMMANDS];
  GtkWidget	*optmenubuttonslabel[UTED_MAX_COMMANDS];
  GtkWidget	*command_window;
  GtkWidget	*questionbox;
  GtkWidget	*commandwind_button;
};

class WUtedGtk : public WUted {
 public:
  GtkWidget		*parent_wid;
  GtkWidget		*toplevel;
  struct		uted_s_widgets widgets ;
  GdkCursor		*cursor;

  WUtedGtk( void	       	*wu_parent_ctx,
	    GtkWidget		*wu_parent_wid,
	    const char	       	*wu_name,
	    const char	       	*wu_iconname,
	    ldh_tWBContext	wu_ldhwb,
	    ldh_tSesContext wu_ldhses,
	    int	       		wu_editmode,
	    void 	       	(*wu_quit_cb)(void *),
	    pwr_tStatus  	*status);
  ~WUtedGtk();
  void remove_command_window();
  void reset_qual();
  void message( const char *new_label);
  void set_command_window( char *cmd);
  void raise_window();
  void clock_cursor();
  void reset_cursor();
  void configure_quals( char *label);
  void enable_entries( int enable);
  void get_value( int idx, char *str, int len);
  void questionbox( char 	*question_title,
		    char	*question_text,
		    void	(* yes_procedure) (WUted *),
		    void	(* no_procedure) (WUted *),
		    void	(* cancel_procedure) (WUted *), 
		    pwr_tBoolean cancel);

  static void activate_command( GtkWidget *w, gpointer data);
  static void create_command( GtkWidget *w, gpointer data);
  static void activate_helputils( GtkWidget *w, gpointer data);
  static void activate_helppwr_plc( GtkWidget *w, gpointer data);
  static void activate_batch( GtkWidget *w, gpointer data);
  static void activate_currsess( GtkWidget *w, gpointer data);
  static void activate_quit( GtkWidget *w, gpointer data);
  static void activate_ok( GtkWidget *w, gpointer data);
  static void activate_cancel( GtkWidget *w, gpointer data);
  static void activate_show_cmd( GtkWidget *w, gpointer data);
  static void activate_cmd_wind( GtkWidget *w, gpointer data);
  static void commandchanged( GtkWidget *w, gpointer data);
  static void activate_present1( GtkWidget *w, gpointer data);
  static void activate_present2( GtkWidget *w, gpointer data);
  static void activate_present3( GtkWidget *w, gpointer data);
  static void activate_present4( GtkWidget *w, gpointer data);
  static void activate_present5( GtkWidget *w, gpointer data);
  static void activate_present6( GtkWidget *w, gpointer data);
  static void activate_present7( GtkWidget *w, gpointer data);
  static void activate_present8( GtkWidget *w, gpointer data);
  static void activate_present9( GtkWidget *w, gpointer data);
  static void activate_present10( GtkWidget *w, gpointer data);
  static void qbox_cr( GtkWidget *w, gpointer data);
  static void qbox_yes_cb( GtkWidget *w, gpointer data);
  static void qbox_no_cb( GtkWidget *w, gpointer data);
  static void qbox_cancel_cb( GtkWidget *w, gpointer data);
};

#endif
