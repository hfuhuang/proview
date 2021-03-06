/* 
 * Proview   $Id$
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

#ifndef cow_pb_gsd_attr_gtk_h
#define cow_pb_gsd_attr_gtk_h

/* cow_pb_gsd_attr_gtk.h -- Profibus gsd attribute editor */

#include "cow_pb_gsd_attr.h"
#include "cow_wow_gtk.h"

class GsdAttrGtk : public GsdAttr {
 public:
  GtkWidget	*parent_wid;
  GtkWidget	*brow_widget;
  GtkWidget	*form_widget;
  GtkWidget	*toplevel;
  GtkWidget	*msg_label;
  GtkWidget	*cmd_prompt;
  GtkWidget	*cmd_input;
  GtkWidget	*attrnav_form;
  GtkWidget	*cmd_ok;
  GtkWidget	*cmd_cancel;
  GtkWidget	*menubutton_copy;
  GtkWidget	*menubutton_cut;
  GtkWidget	*menubutton_paste;
  GtkWidget	*menubutton_changevalue;
  CoWowFocusTimerGtk focustimer;

  GsdAttrGtk( GtkWidget *a_parent_wid,
	      void *a_parent_ctx,
	      void *a_object,
	      pb_gsd *a_gsd,
	      int a_edit_mode);
  ~GsdAttrGtk();

  void message( char severity, const char *message);
  void set_prompt( const char *prompt);
  void change_value();
    
  static void gsdattr_message( void *attr, char severity, char *message);
  static void gsdattr_change_value_cb( void *attr_ctx);
  static void activate_change_value( GtkWidget *w, gpointer data);
  static void activate_exit( GtkWidget *w, gpointer data);
  static void activate_help( GtkWidget *w, gpointer data);
  static void activate_copy( GtkWidget *w, gpointer data);
  static void activate_cut( GtkWidget *w, gpointer data);
  static void activate_paste( GtkWidget *w, gpointer data);
  static void activate_zoom_in( GtkWidget *w, gpointer data);
  static void activate_zoom_out( GtkWidget *w, gpointer data);
  static void activate_zoom_reset( GtkWidget *w, gpointer data);
  static void activate_print( GtkWidget *w, gpointer data);
  static void activate_cmd_input( GtkWidget *w, gpointer data);
  static void activate_cmd_ok( GtkWidget *w, gpointer data);
  static void cmd_close_apply_cb( void *ctx, void *data);
  static void cmd_close_no_cb( void *ctx, void *data);
  static void activate_cmd_ca( GtkWidget *w, gpointer dataxo);
  static gboolean action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data);
  //static void valchanged_cmd_input( GtkWidget *w, gpointer data);
};

#endif








