/* 
 * Proview   $Id: statusmon_nodelist_gtk.cpp,v 1.1 2007-05-11 15:04:14 claes Exp $
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

/* statusmon_nodelist_gtk.cpp -- Console log in statusmon */

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
#include "co_msgwindow_gtk.h"
#include "statusmon_nodelist.h"
#include "rt_xnav_msg.h"
#include "statusmon_nodelist_gtk.h"
#include "statusmon_nodelistnav_gtk.h"

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  NodelistGtk::activate_exit( w, data);
  return TRUE;
}

static void destroy_event( GtkWidget *w, gpointer data)
{
}

NodelistGtk::NodelistGtk( void *nodelist_parent_ctx,
		  GtkWidget *nodelist_parent_wid,
		  char *nodelist_name,
		  pwr_tStatus *status) :
  Nodelist( nodelist_parent_ctx, nodelist_name, status), parent_wid(nodelist_parent_wid),
  clock_cursor(0)
{
  pwr_tStatus sts;

  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
			   "default-height", 600,
			   "default-width", 700,
			   "title", nodelist_name,
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
  GtkWidget *file_close = gtk_image_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("_Close"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(file_close), 
				 gtk_image_new_from_stock( "gtk-close", GTK_ICON_SIZE_MENU));
  g_signal_connect(file_close, "activate", G_CALLBACK(activate_exit), this);
  gtk_widget_add_accelerator( file_close, "activate", accel_g,
			      'w', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *file_add_node = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Add Node"));
  g_signal_connect(file_add_node, "activate", G_CALLBACK(activate_add_node), this);

  GtkWidget *file_remove_node = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Remove Node"));
  g_signal_connect(file_remove_node, "activate", G_CALLBACK(activate_remove_node), this);

  GtkWidget *file_save = gtk_image_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Save Configuration"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(file_save), 
				 gtk_image_new_from_stock( "gtk-save", GTK_ICON_SIZE_MENU));
  g_signal_connect(file_save, "activate", G_CALLBACK(activate_save), this);

  GtkMenu *file_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_add_node);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_remove_node);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_save);

  GtkWidget *file = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_File"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), GTK_WIDGET(file_menu));

  // View menu
  GtkWidget *view_show_events = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("Show _Events"));
  g_signal_connect(view_show_events, "activate", G_CALLBACK(activate_show_events), this);

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
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_show_events);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_in);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_out);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_reset);

  GtkWidget *view = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_View"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), GTK_WIDGET(view_menu));


  // Menu Help
  GtkWidget *help_help = gtk_image_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Help"));
  gtk_image_menu_item_set_image( GTK_IMAGE_MENU_ITEM(help_help), 
				 gtk_image_new_from_stock( "gtk-help", GTK_ICON_SIZE_MENU));
  g_signal_connect(help_help, "activate", G_CALLBACK(activate_help), this);
  gtk_widget_add_accelerator( help_help, "activate", accel_g,
			      'h', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkMenu *help_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_help);

  GtkWidget *help = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Help"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), GTK_WIDGET(help_menu));

  MsgWindowGtk *msg_window = new MsgWindowGtk( this, toplevel, "Status Events", &sts);
  msg_window->find_wnav_cb = find_node_cb;
  MsgWindow::set_default( msg_window);
  MsgWindow::message( 'I', "Status Montitor started");
  
  nodelistnav = new NodelistNavGtk( this, vbox, &nodelistnav_widget);

  gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(menu_bar), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(vbox), GTK_WIDGET(nodelistnav_widget), TRUE, TRUE, 0);

  gtk_container_add( GTK_CONTAINER(toplevel), vbox);
  gtk_widget_show_all( toplevel);

  wow = new CoWowGtk( toplevel);

  *status = 1;
}


//
//  Delete nodelist
//
NodelistGtk::~NodelistGtk()
{
  free_cursor();
  delete nodelistnav;
  gtk_widget_destroy( toplevel);
}

void NodelistGtk::pop()
{
  gtk_window_present( GTK_WINDOW(toplevel));
}

void NodelistGtk::set_clock_cursor()
{
  if ( !clock_cursor)
    clock_cursor = gdk_cursor_new_for_display( gtk_widget_get_display( toplevel),
					       GDK_WATCH);

  gdk_window_set_cursor( toplevel->window, clock_cursor);
  gdk_display_flush( gtk_widget_get_display( toplevel));
}

void NodelistGtk::reset_cursor()
{
  gdk_window_set_cursor( toplevel->window, NULL);
}

void NodelistGtk::free_cursor()
{
  if (clock_cursor)
    gdk_cursor_unref( clock_cursor);
}

gboolean NodelistGtk::action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  NodelistGtk *nodelist = (NodelistGtk *)data;

  if ( nodelist && nodelist->nodelist_displayed)
    nodelist->nodelistnav->set_input_focus();

  return FALSE;
}

void NodelistGtk::activate_exit( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  if ( nodelist->close_cb)
    (nodelist->close_cb)( nodelist->parent_ctx);
  else
    delete nodelist;
}

void NodelistGtk::activate_add_node( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  nodelist->activate_add_node();
}

void NodelistGtk::activate_remove_node( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  nodelist->activate_remove_node();
}

void NodelistGtk::activate_save( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  nodelist->activate_save();
}

void NodelistGtk::activate_show_events( GtkWidget *w, gpointer data)
{
  MsgWindow::map_default();
}

void NodelistGtk::activate_zoom_in( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  nodelist->nodelistnav->zoom( 1.2);
}

void NodelistGtk::activate_zoom_out( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  nodelist->nodelistnav->zoom( 5.0/6);
}

void NodelistGtk::activate_zoom_reset( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  nodelist->nodelistnav->unzoom();
}

void NodelistGtk::activate_help( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  nodelist->activate_help();
}

void NodelistGtk::open_input_dialog( char *text, char *title,
			     char *init_text,
			     void (*ok_cb)( Nodelist *, char *))
{
  create_input_dialog();

  g_object_set( india_widget, 
		"visible", TRUE, 
		"title", title,
		NULL);

  gtk_label_set_text( GTK_LABEL(india_label), text);

  gint pos = 0;
  gtk_editable_delete_text( GTK_EDITABLE(india_text), 0, -1);
  gtk_editable_insert_text( GTK_EDITABLE(india_text), init_text, 
			    strlen(init_text), &pos);

  india_ok_cb = ok_cb;
}

void NodelistGtk::activate_india_ok( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;
  char *text, *textutf8;

  textutf8 = gtk_editable_get_chars( GTK_EDITABLE(((NodelistGtk *)nodelist)->india_text), 
				 0, -1);
  text = g_convert( textutf8, -1, "ISO8859-1", "UTF-8", NULL, NULL, NULL);
  g_free( textutf8);

  g_object_set( ((NodelistGtk *)nodelist)->india_widget, "visible", FALSE, NULL);

  printf( "Add node %s\n", text);
  (nodelist->india_ok_cb)( nodelist, text);
  g_free( text);
}

void NodelistGtk::activate_india_cancel( GtkWidget *w, gpointer data)
{
  Nodelist *nodelist = (Nodelist *)data;

  g_object_set( ((NodelistGtk *)nodelist)->india_widget, "visible", FALSE, NULL);
}

static gint india_delete_event( GtkWidget *w, GdkEvent *event, gpointer nodelist)
{
  g_object_set( ((NodelistGtk *)nodelist)->india_widget, "visible", FALSE, NULL);
  return TRUE;
}

void NodelistGtk::create_input_dialog()
{
  if ( india_widget) {
    g_object_set( india_widget, "visible", TRUE, NULL);
    return;
  }

  // Create an input dialog
  india_widget = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
					     "default-height", 150,
					     "default-width", 350,
					     "title", "Input Dialog",
					     "window-position", GTK_WIN_POS_CENTER,
					     NULL);
  g_signal_connect( india_widget, "delete_event", G_CALLBACK(india_delete_event), this);
  india_text = gtk_entry_new();
  g_signal_connect( india_text, "activate", 
  		    G_CALLBACK(NodelistGtk::activate_india_ok), this);
  india_label = gtk_label_new("");
  GtkWidget *india_image = (GtkWidget *)g_object_new( GTK_TYPE_IMAGE, 
				"stock", GTK_STOCK_DIALOG_QUESTION,
				"icon-size", GTK_ICON_SIZE_DIALOG,
				"xalign", 0.5,
				"yalign", 1.0,
				NULL);

  GtkWidget *india_ok = gtk_button_new_with_label( "Ok");
  gtk_widget_set_size_request( india_ok, 70, 25);
  g_signal_connect( india_ok, "clicked", 
  		    G_CALLBACK(NodelistGtk::activate_india_ok), this);
  GtkWidget *india_cancel = gtk_button_new_with_label( "Cancel");
  gtk_widget_set_size_request( india_cancel, 70, 25);
  g_signal_connect( india_cancel, "clicked", 
  		    G_CALLBACK(NodelistGtk::activate_india_cancel), this);

  GtkWidget *india_hboxtext = gtk_hbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(india_hboxtext), india_image, FALSE, FALSE, 15);
  gtk_box_pack_start( GTK_BOX(india_hboxtext), india_label, FALSE, FALSE, 15);
  gtk_box_pack_end( GTK_BOX(india_hboxtext), india_text, TRUE, TRUE, 30);

  GtkWidget *india_hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(india_hboxbuttons), india_ok, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(india_hboxbuttons), india_cancel, FALSE, FALSE, 0);

  GtkWidget *india_vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(india_vbox), india_hboxtext, TRUE, TRUE, 30);
  gtk_box_pack_start( GTK_BOX(india_vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(india_vbox), india_hboxbuttons, FALSE, FALSE, 15);
  gtk_container_add( GTK_CONTAINER(india_widget), india_vbox);
  gtk_widget_show_all( india_widget);
  g_object_set( india_widget, "visible", FALSE, NULL);
}




