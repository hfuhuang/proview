/* 
 * Proview   $Id: xtt_xatt_gtk.cpp,v 1.1 2007-01-04 08:29:32 claes Exp $
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

/* xtt_xatt.cpp -- Display object attributes */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "co_cdh.h"
#include "co_dcli.h"
#include "co_time.h"
#include "rt_xnav_msg.h"

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "co_lng.h"
#include "xtt_xatt_gtk.h"
#include "xtt_xattnav_gtk.h"
#include "xtt_xnav.h"
#include "rt_xatt_msg.h"

CoWowRecall XAttGtk::value_recall;

void XAttGtk::message( char severity, char *message)
{
  gtk_label_set_text( GTK_LABEL(msg_label), message);
}

void XAttGtk::set_prompt( char *prompt)
{
  if ( strcmp(prompt, "") == 0) {
    g_object_set( cmd_prompt, "visible", FALSE, NULL);
    g_object_set( msg_label, "visible", TRUE, NULL);
  }
  else {
    g_object_set( msg_label, "visible", FALSE, NULL);
    g_object_set( cmd_prompt, "visible", TRUE, NULL);
  }
}

void XAttGtk::change_value( int set_focus)
{
  int		sts;
  GtkWidget	*text_w;
  int		multiline;
  char		*value;
  int		input_size;

  if ( input_open) {
    g_object_set( cmd_input, "visible", FALSE, NULL);
    g_object_set( cmd_scrolledinput, "visible", FALSE, NULL);
    set_prompt( "");
    input_open = 0;
    return;
  }

  sts = xattnav->check_attr( &multiline, &input_node, input_name,
		&value, &input_size);
  if ( EVEN(sts)) {
    if ( sts == XATT__NOATTRSEL)
      message( 'E', "No attribute is selected");
    else
      message( 'E', XNav::get_message( sts));
    return;
  }

  if ( multiline) {
    text_w = cmd_scrolledinput;
    g_object_set( text_w, "visible", TRUE, NULL);

    int w, h;
    gdk_drawable_get_size( pane->window, &w, &h);
    gtk_paned_set_position( GTK_PANED(pane), h - 170);
    if ( set_focus)
      gtk_widget_grab_focus( cmd_scrolledtextview);
    input_multiline = 1;
  }
  else {
    text_w = cmd_input;
    g_object_set( text_w, "visible", TRUE, NULL);
    if ( set_focus)
      gtk_widget_grab_focus( cmd_input);
    input_multiline = 0;
  }

  message( ' ', "");

  if ( value) {
    if ( multiline) {
      GtkTextIter start_iter, end_iter;
      gtk_text_buffer_get_start_iter( cmd_scrolled_buffer, &start_iter);
      gtk_text_buffer_get_end_iter( cmd_scrolled_buffer, &end_iter);
      gtk_text_buffer_delete( cmd_scrolled_buffer, &start_iter, &end_iter);

      gtk_text_buffer_get_start_iter( cmd_scrolled_buffer, &start_iter);
      gtk_text_buffer_insert( cmd_scrolled_buffer, &start_iter, value, -1);

      // Select the text
      // gtk_text_buffer_get_start_iter( cmd_scrolled_buffer, &start_iter);
      // gtk_text_buffer_get_end_iter( cmd_scrolled_buffer, &end_iter);
      // gtk_text_buffer_select_range( cmd_scrolled_buffer, &start_iter, &end_iter);
    }
    else {
      gint pos = 0;
      gtk_editable_delete_text( GTK_EDITABLE(cmd_input), 0, -1);
      gtk_editable_insert_text( GTK_EDITABLE(text_w), value, strlen(value), &pos);

      // Select the text
      gtk_editable_set_position( GTK_EDITABLE(cmd_input), -1);
      gtk_editable_select_region( GTK_EDITABLE(cmd_input), 0, -1);
    }
  }
  else {
    gtk_editable_delete_text( GTK_EDITABLE(cmd_input), 0, -1);
  }

  message( ' ', "");
  set_prompt( Lng::translate("value >"));
  input_open = 1;
}

//
//  Callbackfunctions from menu entries
//
void XAttGtk::activate_change_value( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  xatt->change_value(1);
}

void XAttGtk::activate_close_changeval( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  xatt->change_value_close();
}

void XAttGtk::activate_exit( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  if ( xatt->close_cb)
    (xatt->close_cb)( xatt->parent_ctx, (void *)xatt);
  else
    delete xatt;
}

void XAttGtk::activate_display_object( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  xatt->activate_display_object();
}

void XAttGtk::activate_show_cross( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  xatt->activate_show_cross();
}

void XAttGtk::activate_open_classgraph( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  xatt->activate_open_classgraph();
}

void XAttGtk::activate_open_plc( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  xatt->activate_open_plc();
}

void XAttGtk::activate_help( GtkWidget *w, gpointer data)
{
  XAtt *xatt = (XAtt *)data;

  xatt->activate_help();
}

gboolean XAttGtk::action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  XAttGtk *xatt = (XAttGtk *)data;
  gboolean scrolledinput_visible;
  gboolean input_visible;

  g_object_get( xatt->cmd_scrolledinput, "visible", &scrolledinput_visible, NULL);
  g_object_get( xatt->cmd_input, "visible", &input_visible, NULL);
  if ( scrolledinput_visible)
    gtk_widget_grab_focus( xatt->cmd_scrolledtextview);
  else if ( input_visible)
    gtk_widget_grab_focus( xatt->cmd_input);
  else if ( xatt->xattnav)
    xatt->xattnav->set_inputfocus();

  return FALSE;
}

#if 0
void XAttGtk::valchanged_cmd_input( Widget w, XEvent *event)
{
  XAtt 	*xatt;
  int 	sts;
  char 	*text;
  Arg 	args[2];

  XtSetArg(args[0], XmNuserData, &xatt);
  XtGetValues(w, args, 1);

  sts = mrm_TextInput( w, event, (char *)XAtt::value_recall, sizeof(XAtt::value_recall[0]),
	sizeof( XAtt::value_recall)/sizeof(XAtt::value_recall[0]),
	&xatt->value_current_recall);
  if ( sts)
  {
    text = XmTextGetString( w);
    if ( xatt->input_open)
    {
      sts = xatt->xattnav->set_attr_value( xatt->input_node, 
		xatt->input_name, text);
      XtUnmanageChild( w);
      xatt->set_prompt( "");
      xatt->input_open = 0;
      if ( xatt->redraw_cb)
        (xatt->redraw_cb)( xatt);

      xatt->xattnav->set_inputfocus();
    }
  }
}
#endif

void XAttGtk::change_value_close()
{
  int sts;

  if ( input_open) {
    if ( input_multiline) {
      gchar *text;
      GtkTextIter start_iter, end_iter;
      gtk_text_buffer_get_start_iter( cmd_scrolled_buffer, &start_iter);
      gtk_text_buffer_get_end_iter( cmd_scrolled_buffer, &end_iter);

      text = gtk_text_buffer_get_text( cmd_scrolled_buffer, &start_iter, &end_iter,
				       FALSE);

      sts = xattnav->set_attr_value( input_node,
				     input_name, text);
      g_free( text);
      g_object_set( cmd_scrolledinput, "visible", FALSE, NULL);
      set_prompt( "");
      input_open = 0;

      int w, h;
      gdk_drawable_get_size( pane->window, &w, &h);
      gtk_paned_set_position( GTK_PANED(pane), h - 50);

      xattnav->redraw();
      xattnav->set_inputfocus();
    }
    else {
      char *text;
      text = gtk_editable_get_chars( GTK_EDITABLE(cmd_input), 0, -1);
      sts = xattnav->set_attr_value( input_node, 
				     input_name, text);
      g_free( text);
      g_object_set( cmd_input, "visible", FALSE, NULL);
      set_prompt( "");
      input_open = 0;
      if ( redraw_cb)
        (redraw_cb)( this);

      xattnav->set_inputfocus();
    }
  }
}

void XAttGtk::activate_cmd_input( GtkWidget *w, gpointer data)
{
  char *text;
  XAttGtk *xatt = (XAttGtk *)data;
  int sts;

  g_object_set( xatt->cmd_prompt, "visible", FALSE, NULL);
  g_object_set( xatt->cmd_input, "visible", FALSE, NULL);

  xatt->xattnav->set_inputfocus();

  text = gtk_editable_get_chars( GTK_EDITABLE(w), 0, -1);
  if ( xatt->input_open) {
    sts = xatt->xattnav->set_attr_value( xatt->input_node, 
					 xatt->input_name, text);
    g_object_set( w, "visible", FALSE, NULL);
    xatt->set_prompt( "");
    xatt->input_open = 0;
    if ( xatt->redraw_cb)
      (xatt->redraw_cb)( xatt);
  }
  g_free( text);
}

void XAttGtk::activate_cmd_scrolled_ok( GtkWidget *w, gpointer data)
{
  XAttGtk *xatt = (XAttGtk *)data;
  gchar *text;
  int sts;

  if ( xatt->input_open) {
    GtkTextIter start_iter, end_iter;
    gtk_text_buffer_get_start_iter( xatt->cmd_scrolled_buffer, &start_iter);
    gtk_text_buffer_get_end_iter( xatt->cmd_scrolled_buffer, &end_iter);

    text = gtk_text_buffer_get_text( xatt->cmd_scrolled_buffer, &start_iter, &end_iter,
				     FALSE);

    sts = xatt->xattnav->set_attr_value( xatt->input_node,
						      xatt->input_name, text);
    g_object_set( xatt->cmd_scrolledinput, "visible", FALSE, NULL);
    xatt->set_prompt( "");
    xatt->input_open = 0;

    int w, h;
    gdk_drawable_get_size( xatt->pane->window, &w, &h);
    gtk_paned_set_position( GTK_PANED(xatt->pane), h - 50);

    xatt->xattnav->redraw();
    xatt->xattnav->set_inputfocus();
    g_free( text);
  }
}

void XAttGtk::activate_cmd_scrolled_ca( GtkWidget *w, gpointer data)
{
  XAttGtk *xatt = (XAttGtk *)data;

  if ( xatt->input_open) {
    g_object_set( xatt->cmd_scrolledinput, "visible", FALSE, NULL);

    int w, h;
    gdk_drawable_get_size( xatt->pane->window, &w, &h);
    gtk_paned_set_position( GTK_PANED(xatt->pane), h - 50);

    xatt->set_prompt( "");
    xatt->input_open = 0;
    xatt->xattnav->set_inputfocus();
  }
}

void XAttGtk::pop()
{
  gtk_window_present( GTK_WINDOW(toplevel));
}

XAttGtk::~XAttGtk()
{
  delete (XAttNav *)xattnav;
  delete cmd_entry;
  gtk_widget_destroy( toplevel);
}

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  XAttGtk *xatt = (XAttGtk *)data;

  if ( xatt->close_cb)
    (xatt->close_cb)( xatt->parent_ctx, (void *)xatt);
  else
    delete xatt;
  
  return TRUE;
}

static void destroy_event( GtkWidget *w, gpointer data)
{
}

XAttGtk::XAttGtk( GtkWidget 		*xa_parent_wid,
		      void 		*xa_parent_ctx, 
		      pwr_sAttrRef 	*xa_objar,
		      int 		xa_advanced_user,
		      int               *xa_sts) :
  XAtt( xa_parent_ctx, xa_objar, xa_advanced_user, xa_sts),
  parent_wid(xa_parent_wid)
{
  int sts;
  pwr_tAName   	title;

  *xa_sts = gdh_AttrrefToName( &objar, title, sizeof(title), cdh_mNName);
  if ( EVEN(*xa_sts)) return;

  cdh_StrncpyCutOff( title, title, 100, 1);

  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
			   "default-height", 600,
			   "default-width", 420,
			   "title", title,
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
			      'w', GdkModifierType(GDK_CONTROL_MASK), GTK_ACCEL_VISIBLE);

  GtkMenu *file_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close);

  GtkWidget *file = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_File"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), GTK_WIDGET(file_menu));

  // Functions entry
  GtkWidget *func_changevalue = gtk_menu_item_new_with_mnemonic( CoWowGtk::translate_utf8("Change _Value"));
  g_signal_connect( func_changevalue, "activate", 
		    G_CALLBACK(activate_change_value), this);
  gtk_widget_add_accelerator( func_changevalue, "activate", accel_g,
			      'q', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *func_close_changeval = gtk_menu_item_new_with_mnemonic( 
                      CoWowGtk::translate_utf8("C_lose Change Value"));
  g_signal_connect( func_close_changeval, "activate", 
		    G_CALLBACK(activate_close_changeval), this);
  gtk_widget_add_accelerator( func_close_changeval, "activate", accel_g,
			      't', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *functions_open_plc = gtk_menu_item_new_with_mnemonic( 
				CoWowGtk::translate_utf8("Open _Program"));
  g_signal_connect( functions_open_plc, "activate", 
		    G_CALLBACK(activate_open_plc), this);
  gtk_widget_add_accelerator( functions_open_plc, "activate", accel_g,
  			      'l', GdkModifierType(GDK_CONTROL_MASK), 
  			      GTK_ACCEL_VISIBLE);

  GtkWidget *functions_display_object = gtk_menu_item_new_with_mnemonic( 
                         CoWowGtk::translate_utf8("_Display object in Navigator"));
  g_signal_connect( functions_display_object, "activate", 
		    G_CALLBACK(activate_display_object), this);
  gtk_widget_add_accelerator( functions_display_object, "activate", accel_g,
  			      'd', GdkModifierType(GDK_CONTROL_MASK), 
  			      GTK_ACCEL_VISIBLE);

  GtkWidget *functions_show_cross = gtk_menu_item_new_with_mnemonic( 
                            CoWowGtk::translate_utf8("Show C_rossreferences"));
  g_signal_connect( functions_show_cross, "activate", 
		    G_CALLBACK(activate_show_cross), this);
  gtk_widget_add_accelerator( functions_show_cross, "activate", accel_g,
  			      'r', GdkModifierType(GDK_CONTROL_MASK), 
  			      GTK_ACCEL_VISIBLE);

  GtkWidget *functions_open_classgraph = gtk_menu_item_new_with_mnemonic( 
                     CoWowGtk::translate_utf8("Open _ClassGraph"));
  g_signal_connect( functions_open_classgraph, "activate", 
		    G_CALLBACK(activate_open_classgraph), this);
  gtk_widget_add_accelerator( functions_open_classgraph, "activate", accel_g,
  			      'g', GdkModifierType(GDK_CONTROL_MASK), 
  			      GTK_ACCEL_VISIBLE);


  GtkMenu *func_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(func_menu), func_changevalue);
  gtk_menu_shell_append(GTK_MENU_SHELL(func_menu), func_close_changeval);
  gtk_menu_shell_append(GTK_MENU_SHELL(func_menu), functions_open_plc);
  gtk_menu_shell_append(GTK_MENU_SHELL(func_menu), functions_display_object);
  gtk_menu_shell_append(GTK_MENU_SHELL(func_menu), functions_show_cross);
  gtk_menu_shell_append(GTK_MENU_SHELL(func_menu), functions_open_classgraph);

  GtkWidget *functions = gtk_menu_item_new_with_mnemonic(CoWowGtk::translate_utf8("_Functions"));
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), functions);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(functions), GTK_WIDGET(func_menu));

  // Help entry
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

  pane = gtk_vpaned_new();

  xattnav = new XAttNavGtk( (void *)this, pane, xattnav_eType_Object,
		"Plant", &objar, xa_advanced_user, &brow_widget, &sts);
  xattnav->message_cb = &message_cb;
  xattnav->change_value_cb = &change_value_cb;
  xattnav->popup_menu_cb = &xatt_popup_menu_cb;
  xattnav->is_authorized_cb = &xatt_is_authorized_cb;

  GtkWidget *statusbar = gtk_hbox_new( FALSE, 0);
  msg_label = gtk_label_new( "");
  gtk_widget_set_size_request( msg_label, -1, 25);
  cmd_prompt = gtk_label_new( "value > ");
  gtk_widget_set_size_request( cmd_prompt, -1, 25);
  cmd_entry = new CoWowEntryGtk( &value_recall);
  cmd_input = cmd_entry->widget();
  gtk_widget_set_size_request( cmd_input, -1, 25);
  g_signal_connect( cmd_input, "activate", 
		    G_CALLBACK(activate_cmd_input), this);

  gtk_box_pack_start( GTK_BOX(statusbar), msg_label, FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(statusbar), cmd_prompt, FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(statusbar), cmd_input, TRUE, TRUE, 0);

  gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(menu_bar), FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(vbox), GTK_WIDGET(pane), TRUE, TRUE, 0);

  gtk_paned_pack1( GTK_PANED(pane), GTK_WIDGET(brow_widget), TRUE, TRUE);
  gtk_paned_pack2( GTK_PANED(pane), GTK_WIDGET(statusbar), FALSE, TRUE);
  
  gtk_container_add( GTK_CONTAINER(toplevel), vbox);

  cmd_scrolled_buffer = gtk_text_buffer_new( NULL);

  cmd_scrolledtextview = gtk_text_view_new_with_buffer( cmd_scrolled_buffer);
  GtkWidget *viewport = gtk_viewport_new( NULL, NULL);
  GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add( GTK_CONTAINER(viewport), cmd_scrolledtextview);
  gtk_container_add( GTK_CONTAINER(scrolledwindow), viewport);

  cmd_scrolled_ok = gtk_button_new_with_label( "Ok");
  gtk_widget_set_size_request( cmd_scrolled_ok, 70, 25);
  g_signal_connect( cmd_scrolled_ok, "clicked", 
 		    G_CALLBACK(activate_cmd_scrolled_ok), this);
  cmd_scrolled_ca = gtk_button_new_with_label( "Cancel");
  gtk_widget_set_size_request( cmd_scrolled_ca, 70, 25);
  g_signal_connect( cmd_scrolled_ca, "clicked", 
 		    G_CALLBACK(activate_cmd_scrolled_ca), this);

  GtkWidget *hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(hboxbuttons), cmd_scrolled_ok, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(hboxbuttons), cmd_scrolled_ca, FALSE, FALSE, 0);

  cmd_scrolledinput = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(cmd_scrolledinput), scrolledwindow, TRUE, TRUE, 0);
  gtk_box_pack_start( GTK_BOX(cmd_scrolledinput), hboxbuttons, FALSE, FALSE, 5);

  gtk_box_pack_start( GTK_BOX(statusbar), cmd_scrolledinput, TRUE, TRUE, 0);

  gtk_widget_show_all( toplevel);

  g_object_set( cmd_prompt, "visible", FALSE, NULL);
  g_object_set( cmd_input, "visible", FALSE, NULL);
  g_object_set( cmd_scrolledinput, "visible", FALSE, NULL);
  
  int w, h;
  gdk_drawable_get_size( pane->window, &w, &h);
  gtk_paned_set_position( GTK_PANED(pane), h - 50);

  *xa_sts = XATT__SUCCESS;
}



