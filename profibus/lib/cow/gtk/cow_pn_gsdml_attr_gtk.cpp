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

/* cow_pn_gsdml_attr_gtk.cpp -- Display gsd attributes */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <gtk/gtk.h>

#include "co_cdh.h"
#include "co_time.h"
#include "co_dcli.h"
#include "cow_wow_gtk.h"
#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "glow.h"
#include "glow_growctx.h"
#include "glow_growapi.h"
#include "flow_msg.h"
#include "rt_pb_msg.h"
#include "cow_pn_gsdml_attr_gtk.h"
#include "cow_pn_gsdml_attrnav_gtk.h"


void GsdmlAttrGtk::message( char severity, const char *message)
{
  gtk_label_set_text( GTK_LABEL(msg_label), message);
}

void GsdmlAttrGtk::set_prompt( const char *prompt)
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

void GsdmlAttrGtk::change_value()
{
  int		sts;
  GtkWidget	*text_w;
  char		*value;

  if ( input_open) {
    g_object_set( cmd_input, "visible", FALSE, NULL);
    set_prompt( "");
    input_open = 0;
    return;
  }

  if ( !edit_mode) {
    message( 'E', "Not in edit mode");
    return;
  }

  sts = attrnav->check_attr_value( &value);
  if ( EVEN(sts)) {
    if ( sts == PB__NOATTRSEL)
      message( 'E', "No attribute is selected");
    else
      message( 'E', "Attribute can't be modified");
    return;
  }

  text_w = cmd_input; 
  g_object_set( text_w, "visible", TRUE, NULL);

  message( ' ', "");
  gtk_widget_grab_focus( cmd_input);

  if ( value) {
    gint pos = 0;
    gtk_editable_delete_text( GTK_EDITABLE(cmd_input), 0, -1);
    gtk_editable_insert_text( GTK_EDITABLE(cmd_input), value, strlen(value), &pos);

    // Select the text
    gtk_editable_set_position( GTK_EDITABLE(cmd_input), -1);
    gtk_editable_select_region( GTK_EDITABLE(cmd_input), 0, -1);
  }
  else {
    gtk_editable_delete_text( GTK_EDITABLE(cmd_input), 0, -1);
  }
  set_prompt( "value >");
  input_open = 1;
}

//
//  Callbackfunctions from menu entries
//
void GsdmlAttrGtk::activate_change_value( GtkWidget *w, gpointer data)
{
  GsdmlAttrGtk *attr = (GsdmlAttrGtk *)data;

  attr->change_value();
}

void GsdmlAttrGtk::activate_exit( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_exit();
}

void GsdmlAttrGtk::activate_help( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_help();
}

void GsdmlAttrGtk::activate_copy( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_copy();
}

void GsdmlAttrGtk::activate_cut( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_cut();
}

void GsdmlAttrGtk::activate_paste( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_paste();
}

void GsdmlAttrGtk::activate_viewio( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  int set = (int) gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM(w));
  attr->activate_viewio( set);
}


void GsdmlAttrGtk::activate_zoom_in( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_zoom_in();
}

void GsdmlAttrGtk::activate_zoom_out( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_zoom_out();
}

void GsdmlAttrGtk::activate_zoom_reset( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_zoom_reset();
}

void GsdmlAttrGtk::activate_print( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_print();
}

void GsdmlAttrGtk::activate_cmd_ok( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_cmd_ok();
}

void GsdmlAttrGtk::activate_cmd_ca( GtkWidget *w, gpointer data)
{
  GsdmlAttr *attr = (GsdmlAttr *)data;

  attr->activate_cmd_ca();
}

void GsdmlAttrGtk::activate_cmd_input( GtkWidget *w, gpointer data)
{
  char *text;
  GsdmlAttrGtk *attr = (GsdmlAttrGtk *)data;
  int sts;

  g_object_set( attr->cmd_prompt, "visible", FALSE, NULL);
  g_object_set( attr->cmd_input, "visible", FALSE, NULL);

  attr->attrnav->set_inputfocus();

  text = gtk_editable_get_chars( GTK_EDITABLE(w), 0, -1);
  if ( attr->input_open) {
    sts = attr->attrnav->set_attr_value( text);
    g_object_set( w, "visible", FALSE, NULL);
    attr->set_prompt( "");
    attr->input_open = 0;
  }
  g_free( text);
}

gboolean GsdmlAttrGtk::action_inputfocus( GtkWidget *w, GdkEvent *event, gpointer data)
{
  GsdmlAttrGtk *attr = (GsdmlAttrGtk *)data;
  gboolean input_visible;

  if ( attr->focustimer.disabled())
    return FALSE;

  g_object_get( attr->cmd_input, "visible", &input_visible, NULL);
  if ( input_visible)
    gtk_widget_grab_focus( attr->cmd_input);
  else if ( attr->attrnav)
    attr->attrnav->set_inputfocus();

  attr->focustimer.disable( 400);

  return FALSE;
}

#if 0
void GsdmlAttrGtk::valchanged_cmd_input( Widget w, XEvent *event)
{
  GsdmlAttr *attr;
  int 	sts;
  char 	*text;
  Arg 	args[2];

  XtSetArg(args[0], XmNuserData, &attr);
  XtGetValues(w, args, 1);

  sts = mrm_TextInput( w, event, (char *)attr->value_recall, sizeof(attr->value_recall[0]),
	sizeof( attr->value_recall)/sizeof(attr->value_recall[0]),
	&attr->value_current_recall);
  if ( sts) {
    text = XmTextGetString( w);
    if ( attr->input_open)
    {
      sts = ((GsdmlAttrNav *)attr->attrnav)->set_attr_value( text);
      XtUnmanageChild( w);
      attr->set_prompt( "");
      attr->input_open = 0;

      ((GsdmlAttrNav *)attr->attrnav)->set_inputfocus();
    }
  }
}
#endif

GsdmlAttrGtk::~GsdmlAttrGtk()
{
  delete (GsdmlAttrNav *)attrnav;
  gtk_widget_destroy( toplevel);
}

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  GsdmlAttrGtk::activate_exit( w, data);
  return TRUE;
}

static void destroy_event( GtkWidget *w, gpointer data)
{
}

GsdmlAttrGtk::GsdmlAttrGtk( GtkWidget *a_parent_wid,
			    void *a_parent_ctx,
			    void *a_object,
			    pn_gsdml *a_gsdml,
			    int a_edit_mode,
			    const char *a_data_filename) :
  GsdmlAttr( a_parent_ctx, a_object, a_gsdml, a_edit_mode, a_data_filename)
{
  int sts;

  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
			   "default-height", 600,
			   "default-width", 500,
			   "title", "profinetConfigurator",
			   NULL);

  g_signal_connect( toplevel, "delete_event", G_CALLBACK(delete_event), this);
  g_signal_connect( toplevel, "destroy", G_CALLBACK(destroy_event), this);
  g_signal_connect( toplevel, "focus-in-event", G_CALLBACK(action_inputfocus), this);

  GtkWidget *vbox = gtk_vbox_new( FALSE, 0);

  // Menu
  // Accelerators
  GtkAccelGroup *accel_g = (GtkAccelGroup *) g_object_new(GTK_TYPE_ACCEL_GROUP, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(toplevel), accel_g);

  GtkMenuBar *menu_bar = (GtkMenuBar *) g_object_new(GTK_TYPE_MENU_BAR, NULL);

  // File entry
  GtkWidget *file_print = gtk_image_menu_item_new_from_stock(GTK_STOCK_PRINT, accel_g);
  g_signal_connect(file_print, "activate", G_CALLBACK(activate_print), this);

  GtkWidget *file_close = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE, accel_g);
  g_signal_connect(file_close, "activate", G_CALLBACK(activate_exit), this);

  GtkMenu *file_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_print);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close);

  GtkWidget *file = gtk_menu_item_new_with_mnemonic("_File");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), GTK_WIDGET(file_menu));

  // Edit entry
  menubutton_copy = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, accel_g);
  g_signal_connect(menubutton_copy, "activate", G_CALLBACK(activate_copy), this);

  menubutton_cut = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, accel_g);
  g_signal_connect( menubutton_cut, "activate", G_CALLBACK(activate_cut), this);

  menubutton_paste = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, accel_g);
  g_signal_connect( menubutton_paste, "activate", G_CALLBACK(activate_paste), this);

  GtkMenu *edit_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), menubutton_copy);
  gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), menubutton_cut);
  gtk_menu_shell_append(GTK_MENU_SHELL(edit_menu), menubutton_paste);

  GtkWidget *edit = gtk_menu_item_new_with_mnemonic("_Edit");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), edit);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit), GTK_WIDGET(edit_menu));

  // Functions entry
  menubutton_changevalue = gtk_menu_item_new_with_mnemonic( "_Change Value");
  g_signal_connect( menubutton_changevalue, "activate", 
		    G_CALLBACK(activate_change_value), this);
  gtk_widget_add_accelerator( menubutton_changevalue, "activate", accel_g,
			      'q', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkMenu *func_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(func_menu), menubutton_changevalue);

  GtkWidget *functions = gtk_menu_item_new_with_mnemonic("_Functions");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), functions);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(functions), GTK_WIDGET(func_menu));

  // View Entry

  // Edit entry
  menubutton_viewio = gtk_check_menu_item_new_with_mnemonic( "_View I/O");
  g_signal_connect(menubutton_viewio, "activate", G_CALLBACK(activate_viewio), this);

  GtkWidget *view_zoom_in = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_IN, NULL);
  g_signal_connect( view_zoom_in, "activate", 
		    G_CALLBACK(activate_zoom_in), this);
  gtk_widget_add_accelerator( view_zoom_in, "activate", accel_g,
  			      'i', GdkModifierType(GDK_CONTROL_MASK), 
  			      GTK_ACCEL_VISIBLE);

  GtkWidget *view_zoom_out = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_OUT, NULL);
  g_signal_connect( view_zoom_out, "activate", 
		    G_CALLBACK(activate_zoom_out), this);
  gtk_widget_add_accelerator( view_zoom_out, "activate", accel_g,
  			      'o', GdkModifierType(GDK_CONTROL_MASK), 
  			      GTK_ACCEL_VISIBLE);

  GtkWidget *view_zoom_reset = gtk_image_menu_item_new_from_stock(GTK_STOCK_ZOOM_100, NULL);
  g_signal_connect( view_zoom_reset, "activate", 
		    G_CALLBACK(activate_zoom_reset), this);
  gtk_widget_add_accelerator( view_zoom_reset, "activate", accel_g,
  			      'b', GdkModifierType(GDK_CONTROL_MASK), 
  			      GTK_ACCEL_VISIBLE);

  GtkMenu *view_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), menubutton_viewio);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_in);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_out);
  gtk_menu_shell_append(GTK_MENU_SHELL(view_menu), view_zoom_reset);

  GtkWidget *view = gtk_menu_item_new_with_mnemonic("_View");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), view);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(view), GTK_WIDGET(view_menu));

  // Help entry
  GtkWidget *help_help = gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, accel_g);
  g_signal_connect(help_help, "activate", G_CALLBACK(activate_help), this);

  GtkMenu *help_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_help);

  GtkWidget *help = gtk_menu_item_new_with_mnemonic("_Help");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), GTK_WIDGET(help_menu));

  // Navigator 
  attrnav = new GsdmlAttrNavGtk( this, vbox, "Plant",
		gsdml, edit_mode, &brow_widget, &sts);
  attrnav->message_cb = &GsdmlAttr::gsdmlattr_message;
  attrnav->change_value_cb = &GsdmlAttr::gsdmlattr_change_value_cb;

  // Status bar and value input
  GtkWidget *statusbar = gtk_hbox_new( FALSE, 0);
  msg_label = gtk_label_new( "");
  gtk_widget_set_size_request( msg_label, -1, 25);
  cmd_prompt = gtk_label_new( "value > ");
  gtk_widget_set_size_request( cmd_prompt, -1, 25);
  cmd_input = gtk_entry_new();
  gtk_widget_set_size_request( cmd_input, -1, 25);
  g_signal_connect( cmd_input, "activate", 
		    G_CALLBACK(activate_cmd_input), this);

  gtk_box_pack_start( GTK_BOX(statusbar), msg_label, FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(statusbar), cmd_prompt, FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(statusbar), cmd_input, TRUE, TRUE, 0);

  // Apply and Ok buttons
  cmd_ok = gtk_button_new_with_label( "Ok");
  gtk_widget_set_size_request( cmd_ok, 70, 25);
  g_signal_connect( cmd_ok, "clicked", 
 		    G_CALLBACK(activate_cmd_ok), this);
  cmd_cancel = gtk_button_new_with_label( "Cancel");
  gtk_widget_set_size_request( cmd_cancel, 70, 25);
  g_signal_connect( cmd_cancel, "clicked", 
 		    G_CALLBACK(activate_cmd_ca), this);

  GtkWidget *hboxbuttons = gtk_hbox_new( TRUE, 40);
  gtk_box_pack_start( GTK_BOX(hboxbuttons), cmd_ok, FALSE, FALSE, 0);
  gtk_box_pack_end( GTK_BOX(hboxbuttons), cmd_cancel, FALSE, FALSE, 0);

  gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(menu_bar), FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), brow_widget, TRUE, TRUE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), statusbar, FALSE, FALSE, 3);
  gtk_box_pack_start( GTK_BOX(vbox), gtk_hseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), hboxbuttons, FALSE, FALSE, 5);

  gtk_container_add( GTK_CONTAINER(toplevel), vbox);
  gtk_widget_show_all( toplevel);

  g_object_set( cmd_prompt, "visible", FALSE, NULL);
  g_object_set( cmd_input, "visible", FALSE, NULL);

  if ( !edit_mode) {
    gtk_widget_set_sensitive( cmd_ok, FALSE);
    gtk_widget_set_sensitive( menubutton_copy, FALSE);
    gtk_widget_set_sensitive( menubutton_cut, FALSE);
    gtk_widget_set_sensitive( menubutton_paste, FALSE);
    gtk_widget_set_sensitive( menubutton_changevalue, FALSE);
  }

  wow = new CoWowGtk( toplevel);

  attrnav->open( data_filename);
}

