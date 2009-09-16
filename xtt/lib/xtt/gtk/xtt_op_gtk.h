/* 
 * Proview   $Id: xtt_op_gtk.h,v 1.3 2008-01-24 09:35:26 claes Exp $
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

#ifndef xtt_op_gtk_h
#define xtt_op_gtk_h

/* xtt_op_gtk.h -- Operator window in xtt */

#ifndef xtt_op_h
# include "xtt_op.h"
#endif

#ifndef co_wow_gtk_h
# include "co_wow_gtk.h"
#endif

#include <vector>

class OpCmd {
 public:
  GtkWidget *w;
  pwr_tCmd  cmd;

  OpCmd( GtkWidget *widget, const char *command) : w(widget)
    { strncpy( cmd, command, sizeof(cmd));}
};

class OpGtk : public Op {
 public:
  OpGtk( void *op_parent_ctx,
	 GtkWidget *op_parent_wid,
	 char *opplace,
	 pwr_tStatus *status);
  ~OpGtk();

  GtkWidget		*parent_wid;
  GtkWidget		*parent_wid_op;
  GtkWidget		*toplevel;
  GtkWidget		*alarmcnt_label;
  GtkWidget		*aalarm_label[5];
  GtkWidget		*aalarm_active[5];
  GtkWidget		*aalarm_box[5];
  GtkWidget		*balarm_label;
  GtkWidget		*balarm_active;
  GtkWidget		*balarm_box;
  GtkWidget		*balarm_ebox;
  GtkWidget		*balarm_mark;
  GtkWidget		*appl_form;
  GtkWidget		*decr_button;
  GtkWidget		*tools_close;
  GtkWidget		*functions_close;
  GtkWidget		*funcbox[5];
  GtkMenuBar		*menu_bar;
  GtkWidget		*title_label;
  int			a_height;
  int			a_exist[5];
  int			a_active[5];
  int			text_size;
  CoWowFocusTimerGtk 	poptimer;
  vector<OpCmd> cmd_vect;

  void	map();
  int 	configure( char *opplace_str);
  void 	update_alarm_info();
  void  add_close_button();
  int   get_cmd( GtkWidget *w, char *cmd);
  int   create_menu_item( const char *name, int pixmap, int append, const char *cmd);
  int   delete_menu_item( const char *name);
  void  change_sup_color( void *imagew, op_eSupColor color);
  void  set_title( char *user);

  static void activate_exit( GtkWidget *w, gpointer data);
  static void activate_aalarm_ack( GtkWidget *w, gpointer data);
  static void activate_balarm_ack( GtkWidget *w, gpointer data);
  static void activate_aalarm_incr( GtkWidget *w, gpointer data);
  static void activate_aalarm_decr( GtkWidget *w, gpointer data);
  static void activate_zoom_in( GtkWidget *w, gpointer data);
  static void activate_zoom_out( GtkWidget *w, gpointer data);
  static void activate_alarmlist( GtkWidget *w, gpointer data);
  static void activate_eventlist( GtkWidget *w, gpointer data);
  static void activate_eventlog( GtkWidget *w, gpointer data);
  static void activate_blocklist( GtkWidget *w, gpointer data);
  static void activate_navigator( GtkWidget *w, gpointer data);
  static void activate_help( GtkWidget *w, gpointer data);
  static void activate_help_overview( GtkWidget *w, gpointer data);
  static void activate_help_opwin( GtkWidget *w, gpointer data);
  static void activate_help_proview( GtkWidget *w, gpointer data);
  static void activate_trend( GtkWidget *w, gpointer data);
  static void activate_fast( GtkWidget *w, gpointer data);
  static void activate_history( GtkWidget *w, gpointer data);
  static void activate_switch_user( GtkWidget *w, gpointer data);
  static void activate_show_user( GtkWidget *w, gpointer data);
  static void activate_logout( GtkWidget *w, gpointer data);
  static void activate_cmd_menu_item( GtkWidget *w, gpointer data);
  static void activate_sup_node( GtkWidget *w, gpointer data);
  static void activate_graph( GtkWidget *w, gpointer data);
  static void activate_appl1( GtkWidget *w, gpointer data);
  static void activate_appl2( GtkWidget *w, gpointer data);
  static void activate_appl3( GtkWidget *w, gpointer data);
  static void activate_appl4( GtkWidget *w, gpointer data);
  static void activate_appl5( GtkWidget *w, gpointer data);
  static void activate_appl6( GtkWidget *w, gpointer data);
  static void activate_appl7( GtkWidget *w, gpointer data);
  static void activate_appl8( GtkWidget *w, gpointer data);
  static void activate_appl9( GtkWidget *w, gpointer data);
  static void activate_appl10( GtkWidget *w, gpointer data);
  static void activate_appl11( GtkWidget *w, gpointer data);
  static void activate_appl12( GtkWidget *w, gpointer data);
  static void activate_appl13( GtkWidget *w, gpointer data);
  static void activate_appl14( GtkWidget *w, gpointer data);
  static void activate_appl15( GtkWidget *w, gpointer data);
  static void activate_appl16( GtkWidget *w, gpointer data);
  static void activate_appl17( GtkWidget *w, gpointer data);
  static void activate_appl18( GtkWidget *w, gpointer data);
  static void activate_appl19( GtkWidget *w, gpointer data);
  static void activate_appl20( GtkWidget *w, gpointer data);
  static void activate_appl21( GtkWidget *w, gpointer data);
  static void activate_appl22( GtkWidget *w, gpointer data);
  static void activate_appl23( GtkWidget *w, gpointer data);
  static void activate_appl24( GtkWidget *w, gpointer data);
  static void activate_appl25( GtkWidget *w, gpointer data);
};

#endif
