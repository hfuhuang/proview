/* 
 * Proview   $Id: ge_subgraphs_gtk.cpp,v 1.1 2007-01-04 08:21:58 claes Exp $
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

/* ge_subgraphs_gtk.cpp -- Display object info */

#include "glow_std.h"

#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "co_cdh.h"
#include "co_time.h"
#include "pwr_baseclasses.h"
#include "co_dcli.h"
}

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget_gtk.h"

#include "ge_attr_gtk.h"
#include "ge_subgraphs_gtk.h"
#include "ge_graph.h"

static gint delete_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  SubGraphsGtk *subgraphs = (SubGraphsGtk *)data;

  if ( subgraphs->close_cb)
    (subgraphs->close_cb)( subgraphs);
  else
    delete subgraphs;
  return FALSE;
}

static void destroy_event( GtkWidget *w, gpointer data)
{
}

static void attr_focus_in_event( GtkWidget *w, GdkEvent *event, gpointer data)
{
  SubGraphsGtk *subgraphs = (SubGraphsGtk *)data;

  gdk_window_focus( subgraphs->brow_widget->window, GDK_CURRENT_TIME);
}

static void subgraphs_activate_close( GtkWidget *w, gpointer data)
{
  SubGraphsGtk *subgraphs = (SubGraphsGtk *)data;

  if ( subgraphs->close_cb)
    (subgraphs->close_cb)( subgraphs);
  else
    delete subgraphs;
}

static void subgraphs_activate_attr( GtkWidget *w, gpointer data)
{
  SubGraphsGtk *subgraphs = (SubGraphsGtk *)data;
  ItemSubGraph 		*item;
  int			sts;

  sts = subgraphs->get_select( (void **) &item);
  if ( ODD(sts))
    subgraphs->edit_attributes( item->nodeclass);
}

static void subgraphs_activate_set_extern( GtkWidget *w, gpointer data)
{
  SubGraphsGtk *subgraphs = (SubGraphsGtk *)data;
  ItemSubGraph 		*item;
  int			sts;

  sts = subgraphs->get_select( (void **) &item);
  if ( ODD(sts))
    item->set_extern( 1);
}

static void subgraphs_activate_set_intern( GtkWidget *w, gpointer data)
{
  SubGraphsGtk *subgraphs = (SubGraphsGtk *)data;
  ItemSubGraph 		*item;
  int			sts;

  sts = subgraphs->get_select( (void **) &item);
  if ( ODD(sts))
    item->set_extern( 0);
}

static void subgraphs_activate_help( GtkWidget *w, gpointer data)
{
}

//
// Create the navigator widget
//
SubGraphsGtk::SubGraphsGtk(
	void 		*xn_parent_ctx,
	GtkWidget	*xn_parent_wid,
	char 		*xn_name,
	void 		*xn_growctx,
	GtkWidget      	**w,
	pwr_tStatus 	*status) :
  SubGraphs( xn_parent_ctx, xn_name, xn_growctx, status),
  parent_wid(xn_parent_wid), trace_timerid(0)	
{

  toplevel = (GtkWidget *) g_object_new( GTK_TYPE_WINDOW, 
			   "default-height", 500,
			   "default-width", 300,
			   "title", name,
			   NULL);

  g_signal_connect( toplevel, "delete_event", G_CALLBACK(delete_event), this);
  g_signal_connect( toplevel, "destroy", G_CALLBACK(destroy_event), this);
  g_signal_connect( toplevel, "focus_in_event", G_CALLBACK(attr_focus_in_event), this);

  // Menu
  // Accelerators
  GtkAccelGroup *accel_g = (GtkAccelGroup *) g_object_new(GTK_TYPE_ACCEL_GROUP, NULL);
  gtk_window_add_accel_group(GTK_WINDOW(toplevel), accel_g);

  GtkMenuBar *menu_bar = (GtkMenuBar *) g_object_new(GTK_TYPE_MENU_BAR, NULL);

  // File entry
  GtkWidget *file_attributes = gtk_menu_item_new_with_mnemonic( "_Attributes");
  g_signal_connect( file_attributes, "activate", 
		    G_CALLBACK(subgraphs_activate_attr), this);
  gtk_widget_add_accelerator( file_attributes, "activate", accel_g,
			      'a', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *file_setextern = gtk_menu_item_new_with_mnemonic( "Set _Extern");
  g_signal_connect( file_setextern, "activate", 
		    G_CALLBACK(subgraphs_activate_set_extern), this);
  gtk_widget_add_accelerator( file_setextern, "activate", accel_g,
			      'e', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *file_setintern = gtk_menu_item_new_with_mnemonic( "Set _Intern");
  g_signal_connect( file_setintern, "activate", 
		    G_CALLBACK(subgraphs_activate_set_intern), this);
  gtk_widget_add_accelerator( file_setintern, "activate", accel_g,
			      'i', GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

  GtkWidget *file_close = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE, accel_g);
  g_signal_connect(file_close, "activate", G_CALLBACK(subgraphs_activate_close), this);

  GtkMenu *file_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_attributes);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_setextern);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_setintern);
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), file_close);

  GtkWidget *file = gtk_menu_item_new_with_mnemonic("_File");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), GTK_WIDGET(file_menu));

  // Help entry
  GtkWidget *help_help = gtk_image_menu_item_new_from_stock(GTK_STOCK_HELP, accel_g);
  g_signal_connect(help_help, "activate", G_CALLBACK(subgraphs_activate_help), this);

  GtkMenu *help_menu = (GtkMenu *) g_object_new( GTK_TYPE_MENU, NULL);
  gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_help);

  GtkWidget *help = gtk_menu_item_new_with_mnemonic("_Help");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help);
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), GTK_WIDGET(help_menu));

  form_widget = scrolledbrowwidgetgtk_new(
	SubGraphs::init_brow_cb, this, &brow_widget);

  GtkWidget *vbox = gtk_vbox_new( FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(menu_bar), FALSE, FALSE, 0);
  gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(form_widget), TRUE, TRUE, 0);

  gtk_container_add( GTK_CONTAINER(toplevel), vbox);

  gtk_widget_show_all( toplevel);

  *w = toplevel;
  *status = 1;
}

//
//  Delete a nav context
//
SubGraphsGtk::~SubGraphsGtk()
{
  subgraphs_tAttr attrlist_p, next_p;

  if ( trace_started)
    g_source_remove( trace_timerid);

  // Delete all attr-widgets in attrlist
  attrlist_p = attrlist;
  next_p = NULL;
  while( attrlist_p)
  {
    next_p = attrlist_p->next;
    delete attrlist_p->attrctx;
    free( (char *) attrlist_p);
    attrlist_p = next_p;
  }

  delete brow;

  gtk_widget_destroy( toplevel);
}

void SubGraphsGtk::trace_start()
{
  SubGraphsGtk::trace_scan( this);
}

static gboolean subgraphsgtk_trace_scan( void *data)
{
  SubGraphsGtk::trace_scan( (SubGraphsGtk *)data);
  return FALSE;
}

void SubGraphsGtk::trace_scan( SubGraphsGtk *subgraphs)
{
  int time = 200;

  if ( subgraphs->trace_started) {
    brow_TraceScan( subgraphs->brow->ctx);

    ((SubGraphsGtk *)subgraphs)->trace_timerid = g_timeout_add( time,
	subgraphsgtk_trace_scan, subgraphs);
  }
}

Attr *SubGraphsGtk::new_attr( void *object, attr_sItem *items, int num) 
{
  return new AttrGtk( parent_wid, this, object, items, num);
}
