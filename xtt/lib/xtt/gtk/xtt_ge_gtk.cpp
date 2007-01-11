/* 
 * Proview   $Id: xtt_ge_gtk.cpp,v 1.2 2007-01-11 11:40:31 claes Exp $
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

typedef void *Widget;

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "glow.h"

#include "rt_gdh.h"
#include "rt_gdh_msg.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "co_wow_gtk.h"

#include "glow_growctx.h"
#include "glow_growapi.h"
#include "co_lng.h"
#include "xtt_ge_gtk.h"
#include "ge_graph_gtk.h"
#include "xtt_xnav.h"

gboolean XttGeGtk::action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  XttGeGtk *ge = (XttGeGtk *)data;

  if ( ge->focustimer.disabled())
    return TRUE;

  if ( ge->graph)
    ge->graph->set_inputfocus(1);

  ge->focustimer.disable( 400);
  return FALSE;
}

void XttGeGtk::set_size( int width, int height)
{
  int		default_width;
  int		default_height;
  GdkGeometry   geometry;

  default_width = width + 20;
  default_height = height + 20;

  gtk_window_resize( GTK_WINDOW(toplevel), default_width, default_height);

  geometry.min_aspect = geometry.max_aspect = gdouble(default_width)/default_height;
  gtk_window_set_geometry_hints( GTK_WINDOW(toplevel), GTK_WIDGET(toplevel),
				 &geometry, GDK_HINT_ASPECT);
}

void XttGeGtk::ge_change_value_cb( void *ge_ctx, void *value_object, char *text)
{
  XttGeGtk   *ge = (XttGeGtk *)ge_ctx;

  if ( ge->value_input_open)  {
    g_object_set( ge->value_dialog, "visible", FALSE, NULL);
    ge->value_input_open = 0;
    return;
  }

  g_object_set( ge->value_dialog, "visible", TRUE, NULL);

  ge->message( ' ', "");
  gtk_widget_grab_focus( ge->value_input);

  gint pos = 0;
  gtk_editable_delete_text( GTK_EDITABLE(ge->value_input), 0, -1);
  gtk_editable_insert_text( GTK_EDITABLE(ge->value_input), text, strlen(text), &pos);
  gtk_editable_set_position( GTK_EDITABLE(ge->value_input), -1);
  gtk_editable_select_region( GTK_EDITABLE(ge->value_input), 0, -1);

  ge->value_input_open = 1;
  ge->current_value_object = value_object;
}

void XttGeGtk::confirm_cb( void *ge_ctx, void *confirm_object, char *text)
{
  XttGe	*ge = (XttGe *)ge_ctx;

  if ( ge->confirm_open) {
    g_object_set( ((XttGeGtk *)ge)->confirm_widget, "visible", FALSE, NULL);
    ge->confirm_open = 0;
    return;
  }

  ((XttGeGtk *)ge)->create_confirm_dialog();

  ge->message( ' ', "");

  gtk_label_set_text( GTK_LABEL(((XttGeGtk *)ge)->confirm_label), text);
  ge->confirm_open = 1;
  ge->current_confirm_object = confirm_object;
}

void XttGeGtk::message_dialog_cb( void *ge_ctx, char *text)
{
  XttGe 	*ge = (XttGe *)ge_ctx;
  CoWowGtk 	wow( ((XttGeGtk *)ge)->toplevel);
  
  wow.DisplayError( "Message", CoWowGtk::translate_utf8(text));

  // g_object_set( ((XttGeGtk *)ge)->message_dia_widget, "visible", TRUE, NULL);
  // gtk_label_set_text( GTK_LABEL(((XttGeGtk *)ge)->message_dia_label), text);
}

void XttGeGtk::activate_value_input( GtkWidget *w, gpointer data)
{
  XttGe 	*ge = (XttGe *)data;
  char *text;

  text = gtk_editable_get_chars( GTK_EDITABLE(w), 0, -1);
  if ( ge->value_input_open) {
    ge->graph->change_value( ge->current_value_object, text);
    g_object_set( ((XttGeGtk *)ge)->value_dialog, "visible", FALSE, NULL);
    ge->value_input_open = 0;
  }
  g_free( text);
}

void XttGeGtk::activate_confirm_ok( GtkWidget *w, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  g_object_set( ((XttGeGtk *)ge)->confirm_widget, "visible", FALSE, NULL);
  ge->confirm_open = 0;
  ge->graph->confirm_ok( ge->current_confirm_object);
}

void XttGeGtk::activate_confirm_cancel( GtkWidget *w, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  ge->confirm_open = 0;
  g_object_set( ((XttGeGtk *)ge)->confirm_widget, "visible", FALSE, NULL);
}

void XttGeGtk::activate_exit( GtkWidget *w, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  delete ge;
}

void XttGeGtk::activate_zoom_in( GtkWidget *w, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  ge->graph->zoom( 1.2);
}

void XttGeGtk::activate_zoom_out( GtkWidget *w, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  ge->graph->zoom( 5.0/6);
}

void XttGeGtk::activate_zoom_reset( GtkWidget *w, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  ge->graph->unzoom();
}

void XttGeGtk::activate_help( GtkWidget *w, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  char key[80];

  if ( ge->help_cb) {
    cdh_ToLower( key, ge->name);
    (ge->help_cb)( ge, key);
  }
}

void XttGeGtk::action_resize( GtkWidget *w, GtkAllocation *allocation, gpointer data)
{
  XttGe *ge = (XttGe *)data;

  if ( ge->graph && !ge->scrollbar && !ge->navigator && ge->graph->grow)
    ge->graph->set_default_layout();
}

XttGeGtk::~XttGeGtk()
{
  if ( close_cb)
    (close_cb)( this);
  if ( nav_shell)
    gtk_widget_destroy( nav_shell);
  delete graph;
  gtk_widget_destroy( toplevel);
}

void XttGeGtk::pop()
{
  gtk_window_present( GTK_WINDOW(toplevel));
}

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer ge)
{
  XttGeGtk::activate_exit(w, ge);

  return TRUE;
}

static void destroy_event( GtkWidget *w, gpointer data)
{
}

static gint nav_delete_event( GtkWidget *w, GdkEvent *event, gpointer ge)
{
  return TRUE;
}

XttGeGtk::XttGeGtk( GtkWidget *xg_parent_wid, void *xg_parent_ctx, char *xg_name, char *xg_filename,
		    int xg_scrollbar, int xg_menu, int xg_navigator, int xg_width, int xg_height,
		    int x, int y, double scan_time, char *object_name,
		    int use_default_access, unsigned int access,
		    int (*xg_command_cb) (XttGe *, char *),
		    int (*xg_get_current_objects_cb) (void *, pwr_sAttrRef **, int **),
		    int (*xg_is_authorized_cb) (void *, unsigned int)) :
  XttGe( xg_parent_ctx, xg_name, xg_filename, xg_scrollbar, xg_menu, xg_navigator, xg_width,
	 xg_height, x, y, scan_time, object_name, use_default_access, access,
	 xg_command_cb, xg_get_current_objects_cb, xg_is_authorized_cb), 
  parent_wid(xg_parent_wid), nav_shell(0), value_dialog(0), confirm_widget(0), message_dia_widget(0)
{
  int	window_width = 600;
  int   window_height = 500;
  GdkGeometry   geometry;
  pwr_tStatus sts;
  GtkMenuBar *menu_bar;
  char 		title[300];

  if ( xg_width != 0 && xg_height != 0) {
    window_width = xg_width;
    window_height = xg_height;
  }
  cdh_StrncpyCutOff( title, name, sizeof(title), 1);

  // Gtk
  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
					 "default-height", window_height,
					 "default-width", window_width,
					 "title", title,
					 NULL);

  geometry.min_aspect = geometry.max_aspect = gdouble(window_width)/window_height;
  gtk_window_set_geometry_hints( GTK_WINDOW(toplevel), GTK_WIDGET(toplevel),
				 &geometry, GDK_HINT_ASPECT);

  g_signal_connect( toplevel, "delete_event", G_CALLBACK(delete_event), this);
  g_signal_connect( toplevel, "destroy", G_CALLBACK(destroy_event), this);
  g_signal_connect( toplevel, "focus-in-event", G_CALLBACK(action_inputfocus), this);

  CoWowGtk::SetWindowIcon( toplevel);

  if ( xg_menu) {
    GtkAccelGroup *accel_g = (GtkAccelGroup *) g_object_new(GTK_TYPE_ACCEL_GROUP, NULL);
    gtk_window_add_accel_group(GTK_WINDOW(toplevel), accel_g);

    menu_bar = (GtkMenuBar *) g_object_new(GTK_TYPE_MENU_BAR, NULL);

    // File Entry
    GtkWidget *file_close = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE, accel_g);
    g_signal_connect(file_close, "activate", G_CALLBACK(activate_exit), this);

    GtkMenu *file_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close);

    GtkWidget *file = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_File"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), GTK_WIDGET(file_menu));

    // View menu
    GtkWidget *view_zoom_in = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_IN, NULL);
    g_signal_connect(view_zoom_in, "activate", G_CALLBACK(activate_zoom_in), this);
    gtk_widget_add_accelerator( view_zoom_in, "activate", accel_g,
				'i', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    GtkWidget *view_zoom_out = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_OUT, NULL);
    g_signal_connect(view_zoom_out, "activate", G_CALLBACK(activate_zoom_out), this);
    gtk_widget_add_accelerator( view_zoom_out, "activate", accel_g,
				'o', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    GtkWidget *view_zoom_reset = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_100, NULL);
    g_signal_connect(view_zoom_reset, "activate", G_CALLBACK(activate_zoom_reset), this);

    GtkMenu *view_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_in);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_out);
    gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_reset);

    GtkWidget *view = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_View"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), GTK_WIDGET(view_menu));

    // Menu Help
    GtkWidget *help_help = gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, accel_g);
    g_signal_connect(help_help, "activate", G_CALLBACK(activate_help), this);

    GtkMenu *help_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_help);

    GtkWidget *help = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Help"));
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), GTK_WIDGET(help_menu));
  }

  graph_form = gtk_vbox_new( FALSE, 0);
  graph = new GraphGtk( this, graph_form, "Plant", 
		&grow_widget, &sts, "pwrp_exe:", graph_eMode_Runtime, 
		scrollbar, 1, object_name, 0, 0);
  // graph->set_scantime( scan_time);
  graph->message_cb = &message_cb;
  graph->close_cb = &graph_close_cb;
  graph->init_cb = &graph_init_cb;
  graph->change_value_cb = &ge_change_value_cb;
  graph->confirm_cb = &confirm_cb;
  graph->message_dialog_cb = &message_dialog_cb;
  graph->command_cb = &ge_command_cb;
  graph->display_in_xnav_cb = &ge_display_in_xnav_cb;
  graph->is_authorized_cb = &ge_is_authorized_cb;
  graph->get_current_objects_cb = &ge_get_current_objects_cb;
  graph->popup_menu_cb = &ge_popup_menu_cb;
  graph->call_method_cb = &ge_call_method_cb;
  graph->sound_cb = &ge_sound_cb;

  //g_signal_connect( graph_form, "check_resize", G_CALLBACK(action_resize), this);
  g_signal_connect( ((GraphGtk *)graph)->grow_widget, "size_allocate", G_CALLBACK(action_resize), this);

  if ( xg_menu)
    gtk_box_pack_start( GTK_BOX(graph_form), GTK_WIDGET(menu_bar), FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(graph_form), GTK_WIDGET(grow_widget), TRUE, TRUE, 0);

  gtk_container_add( GTK_CONTAINER(toplevel), graph_form);

  gtk_widget_show_all( toplevel);

  if ( navigator) {
    // Create navigator popup
    nav_shell = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
					    "default-height", 200,
					    "default-width", 200,
					    "title", "Navigator",
					    NULL);
    g_signal_connect( nav_shell, "delete_event", G_CALLBACK(nav_delete_event), this);

    ((GraphGtk *)graph)->create_navigator( nav_shell);
    gtk_container_add( GTK_CONTAINER(nav_shell), ((GraphGtk *)graph)->nav_widget);

    gtk_widget_show_all( nav_shell);
    ((Graph *)graph)->set_nav_background_color();
  }

  if ( !(x == 0 && y == 0)) {
    // Set position
    gtk_window_move( GTK_WINDOW(toplevel), x, y);
  }
}

static gint confirm_delete_event( GtkWidget *w, GdkEvent *event, gpointer ge)
{
  g_object_set( ((XttGeGtk *)ge)->confirm_widget, "visible", FALSE, NULL);
  return TRUE;
}

void XttGeGtk::create_confirm_dialog()
{
  if ( confirm_widget) {
    g_object_set( confirm_widget, "visible", TRUE, NULL);
    return;
  }

  // Create a confirm window
  confirm_widget = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
			   "default-height", 150,
			   "default-width", 400,
			   "title", CoWowGtk::translate_utf8("Confirm"),
			   NULL);
  g_signal_connect( confirm_widget, "delete_event", G_CALLBACK(confirm_delete_event), this);
  confirm_label = gtk_label_new("");
  GtkWidget *confirm_image = (GtkWidget *)g_object_new( GTK_TYPE_IMAGE, 
				"stock", GTK_STOCK_DIALOG_QUESTION,
				"icon-size", GTK_ICON_SIZE_DIALOG,
				"xalign", 0.5,
				"yalign", 1.0,
				NULL);

  GtkWidget *confirm_ok = gtk_button_new_with_label( CoWowGtk::translate_utf8("Yes"));
  gtk_widget_set_size_request( confirm_ok, 70, 25);
  g_signal_connect( confirm_ok, "clicked", 
  		    G_CALLBACK(activate_confirm_ok), this);

  GtkWidget *confirm_cancel = gtk_button_new_with_label( CoWowGtk::translate_utf8("No"));
  gtk_widget_set_size_request( confirm_cancel, 70, 25);
  g_signal_connect( confirm_cancel, "clicked", 
  		    G_CALLBACK(activate_confirm_cancel), this);

  GtkWidget *confirm_hboxtext = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(confirm_hboxtext), confirm_image, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(confirm_hboxtext), confirm_label, TRUE, TRUE, 15);

  GtkWidget *confirm_hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(confirm_hboxbuttons), confirm_ok, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(confirm_hboxbuttons), confirm_cancel, FALSE, FALSE, 0);

  GtkWidget *confirm_vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(confirm_vbox), confirm_hboxtext, TRUE, TRUE, 30);
  gtk_box_pack_start( GTK_BOX(confirm_vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(confirm_vbox), confirm_hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(confirm_widget), confirm_vbox);
  gtk_widget_show_all( confirm_widget);
}

