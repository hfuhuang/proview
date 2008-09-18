/* 
 * Proview   $Id: xtt_clog_gtk.cpp,v 1.3 2008-09-18 14:55:59 claes Exp $
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

/* xtt_clog_gtk.cpp -- Console log in xtt */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "rt_gdh.h"

#include "co_lng.h"
#include "co_wow_gtk.h"
#include "xtt_clog.h"
#include "rt_xnav_msg.h"
#include "xtt_clog_gtk.h"
#include "xtt_clognav_gtk.h"

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  CLogGtk::activate_exit( w, data);
  return TRUE;
}

static void destroy_event( GtkWidget *w, gpointer data)
{
}

CLogGtk::CLogGtk( void *clog_parent_ctx,
		  GtkWidget *clog_parent_wid,
		  char *clog_name,
		  pwr_tStatus *status) :
  CLog( clog_parent_ctx, clog_name, status), parent_wid(clog_parent_wid), filter_form(0),
  clock_cursor(0)
{

  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
			   "default-height", 800,
			   "default-width", 1000,
			   "title", clog_name,
			   NULL);

  g_signal_connect( toplevel, "delete_event", G_CALLBACK(delete_event), this);
  g_signal_connect( toplevel, "destroy", G_CALLBACK(destroy_event), this);
  g_signal_connect( toplevel, "focus-in-event", G_CALLBACK(action_inputfocus), this);

  CoWowGtk::SetWindowIcon( toplevel);

  GtkWidget *vbox = gtk_vbox_new( FALSE, 0);

  // Menu
  // Accelerators
  GtkAccelGroup *accel_g = (GtkAccelGroup *) g_object_new(GTK_TYPE_ACCEL_GROUP, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(toplevel), accel_g);

  GtkMenuBar *menu_bar = (GtkMenuBar *) g_object_new(GTK_TYPE_MENU_BAR, NULL);

  // File entry
  GtkWidget *file_filter = gtk_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("_Filter"));
  g_signal_connect( file_filter, "activate", 
		    G_CALLBACK(activate_filter), this);

  GtkWidget *file_select_file = gtk_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("_Select File"));
  g_signal_connect( file_select_file, "activate", 
		    G_CALLBACK(activate_select_file), this);

  GtkWidget *file_next_file = gtk_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("_Next File"));
  g_signal_connect( file_next_file, "activate", 
		    G_CALLBACK(activate_next_file), this);
  gtk_widget_add_accelerator( file_next_file, "activate", accel_g,
			      'n', GdkModifierType(GDK_CONTROL_MASK), 
			      GTK_ACCEL_VISIBLE);

  GtkWidget *file_prev_file = gtk_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("_Previous File"));
  g_signal_connect( file_prev_file, "activate", 
		    G_CALLBACK(activate_prev_file), this);
  gtk_widget_add_accelerator( file_prev_file, "activate", accel_g,
			      'p', GdkModifierType(GDK_CONTROL_MASK), 
			      GTK_ACCEL_VISIBLE);

  GtkWidget *file_update = gtk_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("_Update"));
  g_signal_connect( file_update, "activate", 
		    G_CALLBACK(activate_update), this);
  gtk_widget_add_accelerator( file_update, "activate", accel_g,
			      'u', GdkModifierType(GDK_CONTROL_MASK), 
			      GTK_ACCEL_VISIBLE);

  GtkWidget *file_close = gtk_image_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("_Close"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(file_close), 
				 gtk_image_new_from_stock( "gtk-close", GTK_ICON_SIZE_MENU));
  g_signal_connect(file_close, "activate", G_CALLBACK(activate_exit), this);
  gtk_widget_add_accelerator( file_close, "activate", accel_g,
			      'w', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkMenu *file_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_filter);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_select_file);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_next_file);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_prev_file);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_update);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close);

  GtkWidget *file = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_File"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), GTK_WIDGET(file_menu));

  // View menu
  GtkWidget *view_zoom_in = gtk_image_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("Zoom _In"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(view_zoom_in), 
				 gtk_image_new_from_stock( "gtk-zoom-in", GTK_ICON_SIZE_MENU));
  g_signal_connect(view_zoom_in, "activate", G_CALLBACK(activate_zoom_in), this);
  gtk_widget_add_accelerator( view_zoom_in, "activate", accel_g,
			      'i', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *view_zoom_out = gtk_image_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("Zoom _Out"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(view_zoom_out), 
				 gtk_image_new_from_stock( "gtk-zoom-out", GTK_ICON_SIZE_MENU));
  g_signal_connect(view_zoom_out, "activate", G_CALLBACK(activate_zoom_out), this);
  gtk_widget_add_accelerator( view_zoom_out, "activate", accel_g,
			      'o', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *view_zoom_reset = gtk_image_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("Zoom _Reset"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(view_zoom_reset), 
				 gtk_image_new_from_stock( "gtk-zoom-100", GTK_ICON_SIZE_MENU));
  g_signal_connect(view_zoom_reset, "activate", G_CALLBACK(activate_zoom_reset), this);

  GtkMenu *view_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_in);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_out);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_reset);

  GtkWidget *view = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_View"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), GTK_WIDGET(view_menu));


  // Menu Help
  GtkWidget *help_help = gtk_image_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Help on ConsoleLog"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(help_help), 
				 gtk_image_new_from_stock( "gtk-help", GTK_ICON_SIZE_MENU));
  g_signal_connect(help_help, "activate", G_CALLBACK(activate_help), this);
  gtk_widget_add_accelerator( help_help, "activate", accel_g,
			      'h', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *help_msg = gtk_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("Help on _Selected Message"));
  g_signal_connect( help_msg, "activate", 
		    G_CALLBACK(activate_helpmsg), this);

  GtkMenu *help_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_help);
  gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_msg);

  GtkWidget *help = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Help"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), GTK_WIDGET(help_menu));

  
  clognav = new CLogNavGtk( this, vbox, &clognav_widget);

  gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(menu_bar), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(vbox), GTK_WIDGET(clognav_widget), TRUE, TRUE, 0);

  gtk_container_add( GTK_CONTAINER(toplevel), vbox);
  gtk_widget_show_all( toplevel);

  wow = new CoWowGtk( toplevel);

  *status = 1;
}


//
//  Delete clog
//
CLogGtk::~CLogGtk()
{
  free_cursor();
  delete clognav;
  if ( filter_form)
    gtk_widget_destroy( filter_form);
  gtk_widget_destroy( toplevel);
}

void CLogGtk::pop()
{
  gtk_window_present( GTK_WINDOW(toplevel));
}

void CLogGtk::set_clock_cursor()
{
  if ( !clock_cursor)
    clock_cursor = gdk_cursor_new_for_display( gtk_widget_get_display( toplevel),
					       GDK_WATCH);

  gdk_window_set_cursor( toplevel->window, clock_cursor);
  gdk_display_flush( gtk_widget_get_display( toplevel));
}

void CLogGtk::reset_cursor()
{
  gdk_window_set_cursor( toplevel->window, NULL);
}

void CLogGtk::free_cursor()
{
  if (clock_cursor)
    gdk_cursor_unref( clock_cursor);
}

gboolean CLogGtk::action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  CLogGtk *clog = (CLogGtk *)data;

  if ( clog && clog->clog_displayed)
    clog->clognav->set_input_focus();

  return FALSE;
}

void CLogGtk::activate_exit( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  if ( clog->close_cb)
    (clog->close_cb)( clog->parent_ctx);
  else
    delete clog;
}

void CLogGtk::activate_zoom_in( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  clog->clognav->zoom( 1.2);
}

void CLogGtk::activate_zoom_out( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  clog->clognav->zoom( 5.0/6);
}

void CLogGtk::activate_zoom_reset( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  clog->clognav->unzoom();
}

void CLogGtk::activate_filter( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;
  bool success, info, warning, error, fatal, text;

  ((CLogGtk *)clog)->create_filter_dialog();

  clog->clognav->get_filter( &success, &info, &warning, &error, &fatal, &text);
  gtk_toggle_button_set_active( 
        GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_success_w), success ? TRUE : FALSE);
  gtk_toggle_button_set_active( 
        GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_info_w), info ? TRUE : FALSE);
  gtk_toggle_button_set_active( 
        GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_warning_w), warning ? TRUE : FALSE);
  gtk_toggle_button_set_active( 
        GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_error_w), error ? TRUE : FALSE);
  gtk_toggle_button_set_active( 
        GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_fatal_w), fatal ? TRUE : FALSE);
  gtk_toggle_button_set_active( 
        GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_text_w), text ? TRUE : FALSE);
}

void CLogGtk::activate_select_file( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;
  char *s;
  pwr_tString80 *str;

  str = (pwr_tString80 *) calloc( clog->clognav->file_list.size() + 1, sizeof( *str));
  for ( int i = 0; i < (int)clog->clognav->file_list.size(); i++) {
    time_AtoAscii( &clog->clognav->file_list[i].time, time_eFormat_ComprDateAndTime, 
		   str[i], sizeof(str[i]));
    str[i][17] = 0;
    strcat( str[i], "    ");
    s = strrchr( clog->clognav->file_list[i].name, '/');
    if ( s)
      strcat( str[i], s+1);
    else
      strcat( str[i], clog->clognav->file_list[i].name);

  }
  clog->wow->CreateList( "Select File", (char *)str, file_selected_cb, 0, clog);

  free( str);
}

void CLogGtk::file_selected_cb( void *ctx, char *text)
{
  CLog *clog = (CLog *)ctx;
  int idx = -1;
  char *s;

  // Indentify the index of the selected text
  for ( int i = 0; i < (int) clog->clognav->file_list.size(); i++) {
    s = strrchr( clog->clognav->file_list[i].name, '/');
    if ( s)
      s++;
    else
      s = clog->clognav->file_list[i].name;
    if ( strcmp( s, &text[21]) == 0) {
      idx = i + 1;
      break;
    }    
  }
  if ( idx == -1) 
    return;

  clog->set_clock_cursor();
  clog->clognav->read( &idx, 1);
  clog->reset_cursor();
}

void CLogGtk::activate_next_file( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  clog->activate_next_file();
}

void CLogGtk::activate_prev_file( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  clog->activate_prev_file();
}

void CLogGtk::activate_update( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  clog->set_clock_cursor();
  clog->clognav->update();
  clog->reset_cursor();
}

void CLogGtk::activate_help( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  clog->activate_help();
}

void CLogGtk::activate_helpmsg( GtkWidget *w, gpointer data)
{
}

void CLogGtk::filter_ok_cb( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  filter_apply_cb( w, data);
  g_object_set( ((CLogGtk *)clog)->filter_form, "visible", FALSE, NULL);
}

void CLogGtk::filter_cancel_cb( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;

  g_object_set( ((CLogGtk *)clog)->filter_form, "visible", FALSE, NULL);
}

void CLogGtk::filter_apply_cb( GtkWidget *w, gpointer data)
{
  CLog *clog = (CLog *)data;
  char *str;

  bool success = (bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_success_w));
  bool info = (bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_info_w));
  bool warning = (bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_warning_w));
  bool error = (bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_error_w));
  bool fatal = (bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_fatal_w));
  bool text = (bool) gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(((CLogGtk *)clog)->show_text_w));

  str = gtk_editable_get_chars( GTK_EDITABLE(((CLogGtk *)clog)->filter_string_w), 0, -1);

  clog->set_clock_cursor();
  clog->clognav->set_filter( success, info, warning, error, fatal, text, str);
  clog->reset_cursor();

  g_free( str);
}

static gboolean filter_delete_event( GtkWidget *w, GdkEvent *event, gpointer clog)
{
  g_object_set( ((CLogGtk *)clog)->filter_form, "visible", FALSE, NULL);
  return TRUE;
}

void CLogGtk::create_filter_dialog()
{
  if ( filter_form) {
    g_object_set( filter_form, "visible", TRUE, NULL);
    return;
  }

  // Create the options dialog
  filter_form = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
					     "default-height", 300,
					     "default-width", 450,
					     "title", "Filter Messages",
					     NULL);
  g_signal_connect( filter_form, "delete_event", G_CALLBACK(filter_delete_event), this);

  GtkWidget *severity_label = gtk_label_new( "Message Severity");
  gtk_misc_set_alignment( GTK_MISC(severity_label), 0.0, 0.05);
  gtk_widget_set_size_request( severity_label, 140, -1);
  GtkWidget *string_label = gtk_label_new( "String");
  gtk_misc_set_alignment( GTK_MISC(string_label), 0.0, 0.5);
  gtk_widget_set_size_request( string_label, 140, -1);
  filter_string_w = gtk_entry_new();

  show_success_w = gtk_check_button_new_with_label( "Success");
  show_info_w = gtk_check_button_new_with_label( "Info");
  show_warning_w = gtk_check_button_new_with_label( "Warning");
  show_error_w = gtk_check_button_new_with_label( "Error");
  show_fatal_w = gtk_check_button_new_with_label( "Fatal");
  show_text_w = gtk_check_button_new_with_label( "Text");

  GtkWidget *severity_vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(severity_vbox), show_success_w, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(severity_vbox), show_info_w, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(severity_vbox), show_warning_w, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(severity_vbox), show_error_w, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(severity_vbox), show_fatal_w, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(severity_vbox), show_text_w, FALSE, FALSE, 7);

  GtkWidget *severity_hbox = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(severity_hbox), severity_label, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(severity_hbox), severity_vbox, FALSE, FALSE, 7);

  GtkWidget *string_hbox = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(string_hbox), string_label, FALSE, FALSE, 7);
  gtk_box_pack_start( GTK_BOX(string_hbox), filter_string_w, TRUE, TRUE, 7);

  GtkWidget *filter_ok = gtk_button_new_with_label( "Ok");
  gtk_widget_set_size_request( filter_ok, 70, 25);
  g_signal_connect( filter_ok, "clicked", 
  		    G_CALLBACK(filter_ok_cb), this);
  GtkWidget *filter_apply = gtk_button_new_with_label( "Apply");
  gtk_widget_set_size_request( filter_apply, 70, 25);
  g_signal_connect( filter_apply, "clicked", 
  		    G_CALLBACK(filter_apply_cb), this);
  GtkWidget *filter_cancel = gtk_button_new_with_label( "Cancel");
  gtk_widget_set_size_request( filter_cancel, 70, 25);
  g_signal_connect( filter_cancel, "clicked", 
  		    G_CALLBACK(filter_cancel_cb), this);

  GtkWidget *filter_hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(filter_hboxbuttons), filter_ok, FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(filter_hboxbuttons), filter_apply, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(filter_hboxbuttons), filter_cancel, FALSE, FALSE, 0);

  GtkWidget *filter_vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(filter_vbox), severity_hbox, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(filter_vbox), string_hbox, TRUE, TRUE, 15);
  gtk_box_pack_start( GTK_BOX(filter_vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(filter_vbox), filter_hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(filter_form), filter_vbox);
  gtk_widget_show_all( filter_form);

}


