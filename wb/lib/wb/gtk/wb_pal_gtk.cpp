/* 
 * Proview   $Id: wb_pal_gtk.cpp,v 1.1 2007-01-04 07:29:02 claes Exp $
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

/* wb_pal_gtk.cpp -- Palette of configurator or plc-editor */

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

extern "C" {
#include "wb_ldh.h"
#include "co_cdh.h"
#include "co_dcli.h"
#include "pwr_baseclasses.h"
}

#include "flow.h"
#include "flow_browctx.h"
#include "flow_browapi.h"
#include "flow_browwidget_gtk.h"
#include "wb_pal_gtk.h"

#include "glow.h"
#include "glow_growctx.h"

static void pal_sel_lose_cb( GtkWidget *w, GdkEventSelection *event,
			     gpointer data);
static void pal_sel_convert_cb( GtkWidget *w, GtkSelectionData *selection_data,
				guint info, guint time_stamp, gpointer data);

//
// Create the palette widgets
//
PalGtk::PalGtk (
	void *pal_parent_ctx,
	GtkWidget *pal_parent_wid,
	char *pal_name,
	ldh_tSesContext pal_ldhses,
	char *pal_root_name,
	GtkWidget **w,
	pwr_tStatus *status) : 
  Pal( pal_parent_ctx, pal_name, pal_ldhses, pal_root_name, status),
  parent_wid(pal_parent_wid)
{
  GtkWidget *scrolledbrow = scrolledbrowwidgetgtk_new(
	Pal::init_brow_cb, this, &brow_widget);

  form_widget = gtk_frame_new( NULL);
  gtk_container_add( GTK_CONTAINER(form_widget), scrolledbrow);
  gtk_container_set_border_width( GTK_CONTAINER(scrolledbrow), 3);

  selection_widget = gtk_invisible_new();
  gtk_selection_add_target( selection_widget, GDK_SELECTION_PRIMARY,
			    GDK_SELECTION_TYPE_STRING, 1);
  g_signal_connect( selection_widget, "selection-get", 
		    G_CALLBACK( pal_sel_convert_cb), this);
  g_signal_connect( selection_widget, "selection-clear-event", 
		    G_CALLBACK( pal_sel_lose_cb), this);
  gtk_widget_show_all( brow_widget);

  set_inputfocus(0);

  *w = form_widget;
  *status = 1;
}

//
//  Delete a pal context
//
PalGtk::~PalGtk()
{
  PalFile::config_tree_free( menu);
  free_pixmaps();
  gtk_widget_destroy( form_widget);
}

void PalGtk::create_popup_menu( pwr_tCid cid, int x, int y)
{
#if 0
  TODO

  int x1, y1;
  Arg arg[2];

  if ( avoid_deadlock)
    return;


  XtSetArg( arg[0], XmNx, &x1);
  XtSetArg( arg[1], XmNy, &y1);
  XtGetValues( XtParent(brow_widget), arg, 2);

  (create_popup_menu_cb)( parent_ctx, cid, x + x1, y + y1);
  pal_set_avoid_deadlock( this, 2000);

#endif
}

void PalGtk::set_inputfocus( int focus)
{
  if ( !displayed)
    return;

  if ( focus) {
    GdkColor color;

    gdk_color_parse( "Black", &color);
    gtk_widget_modify_bg( form_widget, GTK_STATE_NORMAL, &color);
    gtk_widget_grab_focus( brow_widget);
  }
  else {
    GdkColor color;

    gdk_color_parse( "White", &color);
    gtk_widget_modify_bg( form_widget, GTK_STATE_NORMAL, &color);
  }
}

void PalGtk::set_selection_owner()
{
  gboolean sts;

  sts = gtk_selection_owner_set( selection_widget, GDK_SELECTION_PRIMARY,
			   gtk_get_current_event_time());
  if ( !sts) {
     brow_SelectClear( brow_ctx);
     return;
  }	
  selection_owner = 1;
}

static void pal_sel_convert_cb( GtkWidget *w, GtkSelectionData *selection_data,
				guint info, guint time_stamp, gpointer data)
{
  PalGtk     	*pal = (PalGtk *)data;
  char		name[200];
  PalItem	*item;
  brow_tNode	*node_list;
  int		node_count;
  
  if ( !pal->selection_owner)
    return;

  brow_GetSelectedNodes( pal->brow_ctx, &node_list, &node_count);
  if ( !node_count) {
    strcpy( name, "");
  }
  else {
    brow_GetUserData( node_list[0], (void **)&item);

    switch( item->type) {
    case pal_ePalItemType_ClassVolume: 
    case pal_ePalItemType_Class: 
    case pal_ePalItemType_Object: 
    default:
      brow_GetAnnotation( node_list[0], 0, name, sizeof(name));
      free( node_list);
    }
  }
  gtk_selection_data_set( selection_data, GDK_SELECTION_TYPE_STRING,
			  8, (const guchar *)name, strlen(name));
}

static void pal_sel_lose_cb( GtkWidget *w, GdkEventSelection *event,
			     gpointer data)
{
  PalGtk     	*pal = (PalGtk *)data;

  brow_SelectClear( pal->brow_ctx);
  pal->selection_owner = 0;
}






