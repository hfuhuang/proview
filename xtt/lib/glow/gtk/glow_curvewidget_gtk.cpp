/* 
 * Proview   $Id: glow_curvewidget_gtk.cpp,v 1.7 2008-05-13 13:51:20 claes Exp $
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

#include <stdlib.h>
#include "glow_std.h"

#include <gtk/gtk.h>
#include <gtk/gtkprivate.h>
#include "glow_curvewidget_gtk.h"
#include "glow.h"
#include "glow_ctx.h"
#include "glow_curvectx.h"
#include "glow_draw.h"
#include "glow_draw_gtk.h"

typedef struct _CurveWidgetGtk		CurveWidgetGtk;
typedef struct _CurveWidgetGtkClass	CurveWidgetGtkClass;

typedef struct {
	GtkWidget  	*curve;
	GtkWidget	*form;
	GtkWidget	*scroll_h;
	GtkWidget	*scroll_v;
	int		scroll_h_managed;
	int		scroll_v_managed;
	} curvewidget_sScroll;

struct _CurveWidgetGtk {
  GtkDrawingArea parent;

  /* Private */
  void		*curve_ctx;
  void 		*draw_ctx;
  int		(*init_proc)(GlowCtx *ctx, void *clien_data);
  int		is_navigator;
  int		is_realized;
  int		realize_navigator;
  void		*client_data;
  GtkWidget	*main_curve_widget;
  GtkWidget	*navigator_widget;
  GtkWidget    	*scroll_h;
  GtkWidget    	*scroll_v;
  GtkWidget    	*form;
  int		scroll_h_ignore;
  int		scroll_v_ignore;
  gdouble       scroll_h_value;
  gdouble       scroll_v_value;
  int       	scroll_h_pagesize;
  int       	scroll_v_pagesize;
  int       	scroll_h_upper;
  int       	scroll_v_upper;
  gint 		scroll_timerid;
  glow_sScroll  scroll_data;
  int           scroll_configure;
};

struct _CurveWidgetGtkClass {
  GtkDrawingAreaClass parent_class;
};

G_DEFINE_TYPE( CurveWidgetGtk, curvewidgetgtk, GTK_TYPE_DRAWING_AREA);
static gboolean scroll_callback_cb( void *d);

static void scroll_callback( glow_sScroll *data)
{
  curvewidget_sScroll *scroll_data = (curvewidget_sScroll *) data->scroll_data;

  if ( ((CurveWidgetGtk *)scroll_data->curve)->scroll_timerid) 
    g_source_remove( ((CurveWidgetGtk *)scroll_data->curve)->scroll_timerid);

  ((CurveWidgetGtk *)scroll_data->curve)->scroll_timerid = 
    g_timeout_add( 200, scroll_callback_cb, scroll_data->curve);
  ((CurveWidgetGtk *)scroll_data->curve)->scroll_data = *data;
}

static gboolean scroll_callback_cb( void *d)
{
  glow_sScroll *data = &((CurveWidgetGtk *)d)->scroll_data;
  curvewidget_sScroll *scroll_data = (curvewidget_sScroll *) data->scroll_data;

  if ( data->total_width <= data->window_width) {
    if ( data->offset_x == 0)
      data->total_width = data->window_width;
    if ( scroll_data->scroll_h_managed) {
      // Remove horizontal scrollbar
    }
  }
  else {
    if ( !scroll_data->scroll_h_managed) {
      // Insert horizontal scrollbar
    }
  }

  if ( data->total_height <= data->window_height) {
    if ( data->offset_y == 0)
      data->total_height = data->window_height;
    if ( scroll_data->scroll_v_managed) {
      // Remove vertical scrollbar
    }
  }
  else {
    if ( !scroll_data->scroll_v_managed) {
      // Insert vertical scrollbar
    }
  }
  if ( data->offset_x < 0) {
    data->total_width += -data->offset_x;
    data->offset_x = 0;
  }
  if ( data->offset_y < 0) {
    data->total_height += -data->offset_y;
    data->offset_y = 0;
  }
  if ( data->total_height < data->window_height + data->offset_y)
    data->total_height = data->window_height + data->offset_y;
  if ( data->total_width < data->window_width + data->offset_x)
    data->total_width = data->window_width + data->offset_x;
  if ( data->window_width < 1)
    data->window_width = 1;
  if ( data->window_height < 1)
    data->window_height = 1;

  if ( scroll_data->scroll_v_managed) {
    ((CurveWidgetGtk *)scroll_data->curve)->scroll_v_ignore = 1;
    if ( data->window_height != ((CurveWidgetGtk *)scroll_data->curve)->scroll_v_pagesize ||
	 data->total_height != ((CurveWidgetGtk *)scroll_data->curve)->scroll_v_upper ||
	 ((CurveWidgetGtk *)scroll_data->curve)->scroll_configure) {
      g_object_set( ((GtkScrollbar *)scroll_data->scroll_v)->range.adjustment,
		    "upper", (gdouble)data->total_height,
		    "page-size", (gdouble)data->window_height,
		    "value", (gdouble)data->offset_y,
		    NULL);
      gtk_adjustment_changed( ((GtkScrollbar *)scroll_data->scroll_v)->range.adjustment);
    }
    else {
      g_object_set( ((GtkScrollbar *)scroll_data->scroll_v)->range.adjustment,
		    "value", (gdouble)data->offset_y,
		    NULL);
      gtk_adjustment_value_changed( ((GtkScrollbar *)scroll_data->scroll_v)->range.adjustment);
    }
    ((CurveWidgetGtk *)scroll_data->curve)->scroll_v_value = (gdouble)data->offset_y;
    ((CurveWidgetGtk *)scroll_data->curve)->scroll_h_pagesize = data->window_width;
    ((CurveWidgetGtk *)scroll_data->curve)->scroll_h_upper = data->total_width;
  }
  ((CurveWidgetGtk *)scroll_data->curve)->scroll_configure = 0;
  return FALSE;
#if 0
  if ( scroll_data->scroll_h_managed) {
    ((CurveWidgetGtk *)scroll_data->curve)->scroll_h_ignore = 1;
    g_object_set( ((GtkScrollbar *)scroll_data->scroll_h)->range.adjustment,
		 "upper", (gdouble)data->total_width,
		 "page-size", (gdouble)data->window_width,
		 "value", (gdouble)data->offset_x,
		 NULL);
    gtk_adjustment_changed( 
        ((GtkScrollbar *)scroll_data->scroll_h)->range.adjustment);
  }

  if ( scroll_data->scroll_v_managed) {
    ((CurveWidgetGtk *)scroll_data->curve)->scroll_v_ignore = 1;
    g_object_set( ((GtkScrollbar *)scroll_data->scroll_v)->range.adjustment,
		 "upper", (gdouble)data->total_height,
		 "page-size", (gdouble)data->window_height,
		 "value", (gdouble)data->offset_y,
		 NULL);
    gtk_adjustment_changed( 
        ((GtkScrollbar *)scroll_data->scroll_v)->range.adjustment);
  }
#endif
}

static void scroll_h_action( 	GtkWidget      	*w,
				gpointer 	data)
{
  CurveWidgetGtk *curvew = (CurveWidgetGtk *)data;
  if ( curvew->scroll_h_ignore) {
    curvew->scroll_h_ignore = 0;
    return;
  }

  CurveCtx *ctx = (CurveCtx *) curvew->curve_ctx;
  gdouble value;
  g_object_get( w,
		"value", &value,
		NULL);
  glow_scroll_horizontal( ctx, int(value), 0);

}

static void scroll_v_action( 	GtkWidget 	*w,
				gpointer 	data)
{
  CurveWidgetGtk *curvew = (CurveWidgetGtk *)data;

  if ( curvew->scroll_v_ignore) {
    curvew->scroll_v_ignore = 0;
    return;
  }
    
  CurveCtx *ctx = (CurveCtx *) curvew->curve_ctx;
  gdouble value;
  g_object_get( w,
		"value", &value,
		NULL);
  glow_scroll_vertical( ctx, int(value), 0);
}

static int curve_init_proc( GtkWidget *w, GlowCtx *fctx, void *client_data)
{
  curvewidget_sScroll *scroll_data;
  CurveCtx	*ctx;

  ctx = (CurveCtx *) ((CurveWidgetGtk *) w)->curve_ctx;

  if ( ((CurveWidgetGtk *) w)->scroll_h) {
    scroll_data = (curvewidget_sScroll *) malloc( sizeof( curvewidget_sScroll));
    scroll_data->curve = w;
    scroll_data->scroll_h = ((CurveWidgetGtk *) w)->scroll_h;
    scroll_data->scroll_v = ((CurveWidgetGtk *) w)->scroll_v;
    scroll_data->form = ((CurveWidgetGtk *) w)->form;
    scroll_data->scroll_h_managed = 1;
    scroll_data->scroll_v_managed = 1;

    ctx->register_scroll_callback( (void *) scroll_data, scroll_callback);
  }
  return (((CurveWidgetGtk *) w)->init_proc)( ctx, client_data);
}

static gboolean curvewidgetgtk_expose( GtkWidget *glow, GdkEventExpose *event)
{
  if ( !((CurveWidgetGtk *)glow)->curve_ctx)
    // Navigator not yet created
    return TRUE;

  ((GlowDrawGtk *)((CurveCtx *)((CurveWidgetGtk *)glow)->curve_ctx)->gdraw)->event_handler( 
					*(GdkEvent *)event);
  return TRUE;
}

static void curvewidgetgtk_grab_focus( GtkWidget *glow)
{
  if ( !glow->window)
    return;
  GTK_WIDGET_CLASS( curvewidgetgtk_parent_class)->grab_focus( glow);
  gdk_window_focus( glow->window, GDK_CURRENT_TIME);
}

static gboolean curvewidgetgtk_event( GtkWidget *glow, GdkEvent *event)
{
  if ( !((CurveWidgetGtk *)glow)->curve_ctx)
    // Navigator not yet created
    return TRUE;

  if ( event->type == GDK_MOTION_NOTIFY) {
    gdk_display_flush( ((GlowDrawGtk *)((CurveCtx *)((CurveWidgetGtk *)glow)->curve_ctx)->gdraw)->display);
    GdkEvent *next = gdk_event_peek();
    if ( next && next->type == GDK_MOTION_NOTIFY) {
      gdk_event_free( next);
      return TRUE;
    }
    else if ( next)
      gdk_event_free( next);
  }
  else if ( event->type == GDK_CONFIGURE) {
    ((CurveWidgetGtk *)glow)->scroll_configure = 1;
  }

  ((GlowDrawGtk *)((CurveCtx *)((CurveWidgetGtk *)glow)->curve_ctx)->gdraw)->event_handler( *event);
  return TRUE;
}

static void curvewidgetgtk_realize( GtkWidget *widget)
{
  GdkWindowAttr attr;
  gint attr_mask;
  CurveWidgetGtk *curve;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (IS_CURVEWIDGETGTK( widget));

  curve = CURVEWIDGETGTK( widget);
  GTK_WIDGET_SET_FLAGS( widget, GTK_REALIZED);

  attr.x = widget->allocation.x;
  attr.y = widget->allocation.y;
  attr.width = widget->allocation.width;
  attr.height = widget->allocation.height;
  attr.wclass = GDK_INPUT_OUTPUT;
  attr.window_type = GDK_WINDOW_CHILD;
  attr.event_mask = gtk_widget_get_events( widget) |
    GDK_EXPOSURE_MASK | 
    GDK_BUTTON_PRESS_MASK | 
    GDK_BUTTON_RELEASE_MASK | 
    GDK_KEY_PRESS_MASK |
    GDK_POINTER_MOTION_MASK |
    GDK_BUTTON_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK |
    GDK_ENTER_NOTIFY_MASK |
    GDK_LEAVE_NOTIFY_MASK;
  attr.visual = gtk_widget_get_visual( widget);
  attr.colormap = gtk_widget_get_colormap( widget);

  attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new( widget->parent->window, &attr, attr_mask);
  widget->style = gtk_style_attach( widget->style, widget->window);
  gdk_window_set_user_data( widget->window, widget);
  gtk_style_set_background( widget->style, widget->window, GTK_STATE_ACTIVE);

  GTK_WIDGET_SET_FLAGS( widget, GTK_CAN_FOCUS);

  if ( curve->is_navigator) {
    if ( !curve->curve_ctx) {
      CurveWidgetGtk *main_curve = (CurveWidgetGtk *) curve->main_curve_widget;

      if ( !main_curve->is_realized) {
	main_curve->realize_navigator = 1;
	main_curve->navigator_widget = widget;
      }
      else {
	curve->curve_ctx = main_curve->curve_ctx;
	curve->draw_ctx = main_curve->draw_ctx;
	((GlowDrawGtk *)curve->draw_ctx)->init_nav( widget);
      }
    }
  }
  else {
    if ( !curve->curve_ctx) {
      curve->draw_ctx = new GlowDrawGtk( widget, 
					&curve->curve_ctx, 
					curve_init_proc, 
					curve->client_data,
					glow_eCtxType_Curve);
    }
    if ( curve->realize_navigator) {
      CurveWidgetGtk *nav_curve = (CurveWidgetGtk *) curve->navigator_widget;
      nav_curve->curve_ctx = curve->curve_ctx;
      nav_curve->draw_ctx = curve->draw_ctx;
      ((GlowDrawGtk *)nav_curve->draw_ctx)->init_nav( (GtkWidget *)nav_curve);
    }
  }

  curve->is_realized = 1;
}

static void curvewidgetgtk_class_init( CurveWidgetGtkClass *klass)
{
  GtkWidgetClass *widget_class;
  widget_class = GTK_WIDGET_CLASS( klass);
  widget_class->realize = curvewidgetgtk_realize;
  widget_class->expose_event = curvewidgetgtk_expose;
  widget_class->event = curvewidgetgtk_event;
  widget_class->grab_focus = curvewidgetgtk_grab_focus;
}

static void curvewidgetgtk_init( CurveWidgetGtk *glow)
{
}

GtkWidget *curvewidgetgtk_new(
        int (*init_proc)(GlowCtx *ctx, void *client_data),
	void *client_data)
{
  CurveWidgetGtk *w;
  w =  (CurveWidgetGtk *) g_object_new( CURVEWIDGETGTK_TYPE, NULL);
  w->init_proc = init_proc;
  w->curve_ctx = 0;
  w->is_navigator = 0;
  w->client_data = client_data;
  w->scroll_h = 0;
  w->scroll_v = 0;
  return (GtkWidget *) w;  
}

GtkWidget *scrolledcurvewidgetgtk_new(
        int (*init_proc)(GlowCtx *ctx, void *client_data),
	void *client_data, GtkWidget **curvewidget)
{
  CurveWidgetGtk *w;

  GtkWidget *form = gtk_scrolled_window_new( NULL, NULL);

  w =  (CurveWidgetGtk *) g_object_new( CURVEWIDGETGTK_TYPE, NULL);
  w->init_proc = init_proc;
  w->curve_ctx = 0;
  w->is_navigator = 0;
  w->client_data = client_data;
  w->scroll_h = GTK_SCROLLED_WINDOW(form)->hscrollbar;
  w->scroll_v = GTK_SCROLLED_WINDOW(form)->vscrollbar;
  w->scroll_h_ignore = 0;
  w->scroll_v_ignore = 0;
  w->scroll_h_value = 0;
  w->scroll_v_value = 0;
  w->scroll_configure = 0;
  w->form = form;
  *curvewidget = GTK_WIDGET( w);

  g_signal_connect( ((GtkScrollbar *)w->scroll_h)->range.adjustment, 
		    "value-changed", G_CALLBACK(scroll_h_action), w);
  g_signal_connect( ((GtkScrollbar *)w->scroll_v)->range.adjustment, 
		    "value-changed", G_CALLBACK(scroll_v_action), w);

  GtkWidget *viewport = gtk_viewport_new( NULL, NULL);
  gtk_container_add( GTK_CONTAINER(viewport), GTK_WIDGET(w));
  gtk_container_add( GTK_CONTAINER(form), GTK_WIDGET(viewport));

  return (GtkWidget *) form;  
}

GtkWidget *curvenavwidgetgtk_new( GtkWidget *main_curve)
{
  CurveWidgetGtk *w;
  w =  (CurveWidgetGtk *) g_object_new( CURVEWIDGETGTK_TYPE, NULL);
  w->init_proc = 0;
  w->curve_ctx = 0;
  w->is_navigator = 1;
  w->main_curve_widget = main_curve;
  w->client_data = 0;
  w->scroll_h = 0;
  w->scroll_v = 0;
  w->scroll_h_ignore = 0;
  w->scroll_v_ignore = 0;
  w->scroll_h_value = 0;
  w->scroll_v_value = 0;
  w->scroll_configure = 0;
  return (GtkWidget *) w;  
}

#if 0
GType curvewidgetgtk_get_type(void)
{
  static GType curvewidgetgtk_type = 0;

  if ( !curvewidgetgtk_type) {
    static const GTypeInfo curvewidgetgtk_info = {
      sizeof(CurveWidgetGtkClass), NULL, NULL, (GClassInitFunc)curvewidgetgtk_class_init,
      NULL, NULL, sizeof(CurveWidgetGtk), 1, NULL, NULL};
    
    curvewidgetgtk_type = g_type_register_static( G_TYPE_OBJECT, "CurveWidgetGtk", &curvewidgetgtk_info, 
					   (GTypeFlags)0);  
  }
  return curvewidgetgtk_type;
}
#endif
