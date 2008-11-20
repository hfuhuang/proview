/* 
 * Proview   $Id: glow_growbar.cpp,v 1.11 2008-11-20 10:30:44 claes Exp $
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

#include "glow_std.h"


#include <iostream>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include "glow_growbar.h"
#include "glow_grownode.h"
#include "glow_draw.h"
#include "glow_growctx.h"

GrowBar::GrowBar( GrowCtx *glow_ctx, const char *name, double x, double y, 
		double w, double h, glow_eDrawType border_d_type, int line_w, 
		glow_mDisplayLevel display_lev,
		int fill_rect, int display_border, 
		glow_eDrawType fill_d_type, int nodraw) : 
		GrowRect(glow_ctx,name,x,y,w,h,border_d_type,line_w,0,
		display_lev,fill_rect,display_border,0,fill_d_type,nodraw),
		max_value(100),min_value(0),bar_value(35),
		bar_drawtype(glow_eDrawType_Inherit),
		bar_bordercolor(glow_eDrawType_Inherit),
		bar_borderwidth(1), user_data(0)
{

  if ( !nodraw)
    draw( &ctx->mw, (GlowTransform *)NULL, highlight, hot, NULL, NULL);
}

GrowBar::~GrowBar()
{
  if ( ctx->nodraw) return;
  erase( &ctx->mw);
  erase( &ctx->navw);
}

void GrowBar::save( ofstream& fp, glow_eSaveMode mode) 
{ 

  fp << int(glow_eSave_GrowBar) << endl;
  fp << int(glow_eSave_GrowBar_max_value) << FSPACE << max_value << endl;
  fp << int(glow_eSave_GrowBar_min_value) << FSPACE << min_value << endl;
  fp << int(glow_eSave_GrowBar_bar_value) << FSPACE << bar_value << endl;
  fp << int(glow_eSave_GrowBar_bar_drawtype) << FSPACE << int(bar_drawtype) << endl;
  fp << int(glow_eSave_GrowBar_bar_bordercolor) << FSPACE << int(bar_bordercolor) << endl;
  fp << int(glow_eSave_GrowBar_bar_borderwidth) << FSPACE << bar_borderwidth << endl;
  fp << int(glow_eSave_GrowBar_trace_data1) << FSPACE << trace.data[0] << endl;
  fp << int(glow_eSave_GrowBar_trace_data2) << FSPACE << trace.data[1] << endl;
  fp << int(glow_eSave_GrowBar_trace_data3) << FSPACE << trace.data[2] << endl;
  fp << int(glow_eSave_GrowBar_trace_data4) << FSPACE << trace.data[3] << endl;
  fp << int(glow_eSave_GrowBar_trace_data5) << FSPACE << trace.data[4] << endl;
  fp << int(glow_eSave_GrowBar_trace_data6) << FSPACE << trace.data[5] << endl;
  fp << int(glow_eSave_GrowBar_trace_data7) << FSPACE << trace.data[6] << endl;
  fp << int(glow_eSave_GrowBar_trace_data8) << FSPACE << trace.data[7] << endl;
  fp << int(glow_eSave_GrowBar_trace_data9) << FSPACE << trace.data[8] << endl;
  fp << int(glow_eSave_GrowBar_trace_data10) << FSPACE << trace.data[9] << endl;
  fp << int(glow_eSave_GrowBar_trace_attr_type) << FSPACE << int(trace.attr_type) << endl;
  fp << int(glow_eSave_GrowBar_trace_color) << FSPACE << int(trace.color) << endl;
  fp << int(glow_eSave_GrowBar_trace_color) << FSPACE << int(trace.color) << endl;
  fp << int(glow_eSave_GrowBar_access) << FSPACE << (unsigned int)(trace.access) << endl;
  fp << int(glow_eSave_GrowBar_cycle) << FSPACE << int(trace.cycle) << endl;
  fp << int(glow_eSave_GrowBar_ref_object) << FSPACE << trace.ref_object << endl;
  fp << int(glow_eSave_GrowBar_rect_part) << endl;
  GrowRect::save( fp, mode);
  if ( user_data && ctx->userdata_save_callback) {
    fp << int(glow_eSave_GrowBar_userdata_cb) << endl;
    (ctx->userdata_save_callback)(&fp, this, glow_eUserdataCbType_Node);
  }
  fp << int(glow_eSave_End) << endl;
}

void GrowBar::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;
  unsigned int	utmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_GrowBar: break;
      case glow_eSave_GrowBar_max_value: fp >> max_value; break;
      case glow_eSave_GrowBar_min_value: fp >> min_value; break;
      case glow_eSave_GrowBar_bar_value: fp >> bar_value; break;
      case glow_eSave_GrowBar_bar_bordercolor: fp >> 
		tmp; bar_bordercolor = (glow_eDrawType)tmp; break;
      case glow_eSave_GrowBar_bar_borderwidth: fp >> bar_borderwidth; break;
      case glow_eSave_GrowBar_bar_drawtype: fp >> 
		tmp; bar_drawtype = (glow_eDrawType)tmp; break;
      case glow_eSave_GrowBar_rect_part: 
        GrowRect::open( fp);
        break;
      case glow_eSave_GrowBar_trace_data1:
        fp.get();
        fp.getline( trace.data[0], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data2:
        fp.get();
        fp.getline( trace.data[1], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data3:
        fp.get();
        fp.getline( trace.data[2], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data4:
        fp.get();
        fp.getline( trace.data[3], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data5:
        fp.get();
        fp.getline( trace.data[4], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data6:
        fp.get();
        fp.getline( trace.data[5], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data7:
        fp.get();
        fp.getline( trace.data[6], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data8:
        fp.get();
        fp.getline( trace.data[7], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data9:
        fp.get();
        fp.getline( trace.data[8], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_data10:
        fp.get();
        fp.getline( trace.data[9], sizeof(trace.data[0]));
        break;
      case glow_eSave_GrowBar_trace_attr_type: fp >> tmp; trace.attr_type = (glow_eTraceType)tmp; break;
      case glow_eSave_GrowBar_trace_color: fp >> tmp; trace.color = (glow_eDrawType)tmp; break;
      case glow_eSave_GrowBar_access: fp >> utmp; trace.access = (glow_mAccess)utmp; break;
      case glow_eSave_GrowBar_cycle: fp >> tmp; trace.cycle = (glow_eCycle)tmp; break;
      case glow_eSave_GrowBar_ref_object:
        fp.get();
        fp.getline( trace.ref_object, sizeof(trace.ref_object));
        break;
      case glow_eSave_GrowBar_userdata_cb:
	if ( ctx->userdata_open_callback)
	  (ctx->userdata_open_callback)(&fp, this, glow_eUserdataCbType_Node);
	break;
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GrowBar:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

void GrowBar::draw( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y) 
{ 
  int tmp;

  if ( ll_x > ur_x) {
    /* Shift */
    tmp = ll_x;
    ll_x = ur_x;
    ur_x = tmp;
  }
  if ( ll_y > ur_y) {
    /* Shift */
    tmp = ll_y;
    ll_y = ur_y;
    ur_y = tmp;
  }

  if ( x_right * w->zoom_factor_x - w->offset_x + 1 >= ll_x &&
      	x_left * w->zoom_factor_x - w->offset_x <= ur_x &&
       	y_high * w->zoom_factor_y - w->offset_y + 1 >= ll_y &&
       	y_low * w->zoom_factor_y- w->offset_y <= ur_y)
  {
    draw( w, (GlowTransform *)NULL, highlight, hot, NULL, NULL);
  }
}

void GrowBar::draw( GlowWind *w, int *ll_x, int *ll_y, int *ur_x, int *ur_y) 
{ 
  int 	tmp;
  int 	obj_ur_x = int( x_right * w->zoom_factor_x) - w->offset_x;
  int	obj_ll_x = int( x_left * w->zoom_factor_x) - w->offset_x;
  int	obj_ur_y = int( y_high * w->zoom_factor_y) - w->offset_y;
  int   obj_ll_y = int( y_low * w->zoom_factor_y) - w->offset_y;


  if ( *ll_x > *ur_x) {
    /* Shift */
    tmp = *ll_x;
    *ll_x = *ur_x;
    *ur_x = tmp;
  }
  if ( *ll_y > *ur_y) {
    /* Shift */
    tmp = *ll_y;
    *ll_y = *ur_y;
    *ur_y = tmp;
  }

  if (  obj_ur_x >= *ll_x &&
      	obj_ll_x <= *ur_x &&
       	obj_ur_y >= *ll_y &&
       	obj_ll_y <= *ur_y) {
    draw( w, (GlowTransform *)NULL, highlight, hot, NULL, NULL);

    // Increase the redraw area
    if ( obj_ur_x > *ur_x)
      *ur_x = obj_ur_x;
    if ( obj_ur_y > *ur_y)
      *ur_y = obj_ur_y;
    if ( obj_ll_x < *ll_x)
      *ll_x = obj_ll_x;
    if ( obj_ll_y < *ll_y)
      *ll_y = obj_ll_y;
  }
}

void GrowBar::set_highlight( int on)
{
  highlight = on;
  draw();
}

void GrowBar::draw( GlowWind *w, GlowTransform *t, int highlight, int hot, void *node, void *colornode)
{
  if ( !(display_level & ctx->display_level))
    return;
  if ( w == &ctx->navw) {
    if ( ctx->no_nav)
      return;
    hot = 0;
  }
  int idx;
  glow_eDrawType drawtype;
  glow_eGradient grad = gradient;
  if ( gradient == glow_eGradient_No && 
       (node && ((GrowNode *)node)->gradient != glow_eGradient_No) && !disable_gradient)
    grad = ((GrowNode *)node)->gradient;

  int bar_border_idx = int( w->zoom_factor_y / w->base_zoom_factor * 
	bar_borderwidth - 1);
  bar_border_idx = min( DRAW_TYPE_SIZE - 1, max( 0, bar_border_idx));

  if ( fix_line_width) {
    idx = line_width;
    idx += hot;
    if ( idx < 0) {
      erase( w, t, hot, node);
      return;
    }
  }
  else {
    if ( node && ((GrowNode *)node)->line_width)
      idx = int( w->zoom_factor_y / w->base_zoom_factor * 
		((GrowNode *)node)->line_width - 1);
    else
      idx = int( w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
    idx += hot;
  }
  idx = max( 0, idx);
  idx = min( idx, DRAW_TYPE_SIZE-1);
  int x1, y1, x2, y2, ll_x, ll_y, ur_x, ur_y;

  if (!t) {
    x1 = int( trf.x( ll.x, ll.y) * w->zoom_factor_x) - w->offset_x;
    y1 = int( trf.y( ll.x, ll.y) * w->zoom_factor_y) - w->offset_y;
    x2 = int( trf.x( ur.x, ur.y) * w->zoom_factor_x) - w->offset_x;
    y2 = int( trf.y( ur.x, ur.y) * w->zoom_factor_y) - w->offset_y;
  }
  else {
    x1 = int( trf.x( t, ll.x, ll.y) * w->zoom_factor_x) - w->offset_x;
    y1 = int( trf.y( t, ll.x, ll.y) * w->zoom_factor_y) - w->offset_y;
    x2 = int( trf.x( t, ur.x, ur.y) * w->zoom_factor_x) - w->offset_x;
    y2 = int( trf.y( t, ur.x, ur.y) * w->zoom_factor_y) - w->offset_y;
  }

  ll_x = min( x1, x2);
  ur_x = max( x1, x2);
  ll_y = min( y1, y2);
  ur_y = max( y1, y2);
  if ( fill) {
    drawtype = ctx->get_drawtype( fill_drawtype, glow_eDrawType_FillHighlight,
		 highlight, (GrowNode *)colornode, 1);
    
    if ( grad == glow_eGradient_No)
      ctx->gdraw->fill_rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, drawtype);
    else {
      glow_eDrawType f1, f2;
      double rotation;

      if ( t)
	rotation = trf.rot( t);
      else
	rotation = trf.rot();
      if ( gradient_contrast >= 0) {
	f2 = GlowColor::shift_drawtype( drawtype, -gradient_contrast/2, 0);
	f1 = GlowColor::shift_drawtype( drawtype, int(float(gradient_contrast)/2+0.6), 0);
      }
      else {
	f2 = GlowColor::shift_drawtype( drawtype, -int(float(gradient_contrast)/2-0.6), 0);
	f1 = GlowColor::shift_drawtype( drawtype, gradient_contrast/2, 0);
      }
      ctx->gdraw->gradient_fill_rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, drawtype, f1, f2, ctx->gdraw->gradient_rotate( rotation, grad));
    }
  }
  drawtype = ctx->get_drawtype( draw_type, glow_eDrawType_LineHighlight,
		 highlight, (GrowNode *)colornode, 0);

  if ( max_value != min_value) {
    double rotation;
    int x0, y0, width, height, l_x0, l_y0, l_x1, l_y1;

    if ( t)
      rotation = (trf.rot( t) / 360 - floor( trf.rot( t) / 360)) * 360;
    else
      rotation = (trf.rot() / 360 - floor( trf.rot() / 360)) * 360;
    if ( 45 >= rotation || rotation > 315) {
      height = int( (bar_value - min_value) / (max_value - min_value) 
		* (ur_y - ll_y));
      height = max( 0, min( height, ur_y - ll_y));
      width = ur_x - ll_x;
      x0 = ll_x;
      y0 = ur_y - height;
      l_x0 = ll_x;
      l_y0 = ur_y - height;
      l_x1 = ur_x;
      l_y1 = ur_y - height;
    }
    else if ( 45 < rotation && rotation <= 135) {
      width = int( (bar_value - min_value) / (max_value - min_value) * 
		(ur_x - ll_x));
      width = max( 0, min( width, ur_x - ll_x));
      height = ur_y - ll_y;
      x0 = ll_x;
      y0 = ll_y;
      l_x0 = ll_x + width;
      l_y0 = ll_y;
      l_x1 = ll_x + width;
      l_y1 = ur_y;
    }
    else if ( 135 < rotation && rotation <= 225) {
      height = int( (bar_value - min_value) / (max_value - min_value) * 
		(ur_y - ll_y));
      height = max( 0, min( height, ur_y - ll_y));
      width = ur_x - ll_x;
      x0 = ll_x;
      y0 = ll_y;
      l_x0 = ll_x;
      l_y0 = ll_y + height;
      l_x1 = ur_x;
      l_y1 = ll_y + height;
    }
    else { // if ( 225 < rotation && rotation <= 315)
      width = int( (bar_value - min_value) / (max_value - min_value) * 
		(ur_x - ll_x));
      width = max( 0, min( width, ur_x - ll_x));
      height = ur_y - ll_y;
      x0 = ur_x - width;
      y0 = ll_y;
      l_x0 = ur_x - width;
      l_y0 = ll_y;
      l_x1 = ur_x - width;
      l_y1 = ur_y;
    }

    glow_eDrawType dt = drawtype;
    if ( bar_drawtype != glow_eDrawType_Inherit)
      dt = bar_drawtype;

    if ( grad == glow_eGradient_No)
      ctx->gdraw->fill_rect( w, x0, y0, width, height, dt);
    else {
      glow_eDrawType f1, f2;
      double rotation;

      if ( t)
	rotation = trf.rot( t);
      else
	rotation = trf.rot();
      if ( gradient_contrast >= 0) {
	f2 = GlowColor::shift_drawtype( dt, -gradient_contrast/2, 0);
	f1 = GlowColor::shift_drawtype( dt, int(float(gradient_contrast)/2+0.6), 0);
      }
      else {
	f2 = GlowColor::shift_drawtype( dt, -int(float(gradient_contrast)/2-0.6), 0);
	f1 = GlowColor::shift_drawtype( dt, gradient_contrast/2, 0);
      }
      ctx->gdraw->gradient_fill_rect( w, x0, y0, width, height, dt, f1, f2, ctx->gdraw->gradient_rotate( rotation, grad));
    }

    dt = drawtype;
    if ( bar_bordercolor != glow_eDrawType_Inherit)
      dt = bar_bordercolor;
    ctx->gdraw->line( w, l_x0, l_y0, l_x1, l_y1, dt, bar_border_idx, 0);
  }

  if ( border) {
    ctx->gdraw->rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, drawtype, idx, 0);
  }
}

void GrowBar::erase( GlowWind *w, GlowTransform *t, int hot, void *node)
{
  if ( !(display_level & ctx->display_level))
    return;
  if ( w == &ctx->navw) {
    if ( ctx->no_nav)
      return;
    hot = 0;
  }
  int idx;
  if ( fix_line_width) {
    idx = line_width;
    idx += hot;
    if ( idx < 0) return;
  }
  else {
    if ( node && ((GrowNode *)node)->line_width)
      idx = int( w->zoom_factor_y / w->base_zoom_factor * 
		((GrowNode *)node)->line_width - 1);
    else
      idx = int( w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
    idx += hot;
  }
  idx = max( 0, idx);
  idx = min( idx, DRAW_TYPE_SIZE-1);
  int x1, y1, x2, y2, ll_x, ll_y, ur_x, ur_y;

  if (!t) {
    x1 = int( trf.x( ll.x, ll.y) * w->zoom_factor_x) - w->offset_x;
    y1 = int( trf.y( ll.x, ll.y) * w->zoom_factor_y) - w->offset_y;
    x2 = int( trf.x( ur.x, ur.y) * w->zoom_factor_x) - w->offset_x;
    y2 = int( trf.y( ur.x, ur.y) * w->zoom_factor_y) - w->offset_y;
  }
  else {
    x1 = int( trf.x( t, ll.x, ll.y) * w->zoom_factor_x) - w->offset_x;
    y1 = int( trf.y( t, ll.x, ll.y) * w->zoom_factor_y) - w->offset_y;
    x2 = int( trf.x( t, ur.x, ur.y) * w->zoom_factor_x) - w->offset_x;
    y2 = int( trf.y( t, ur.x, ur.y) * w->zoom_factor_y) - w->offset_y;
  }
  ll_x = min( x1, x2);
  ur_x = max( x1, x2);
  ll_y = min( y1, y2);
  ur_y = max( y1, y2);

  w->set_draw_buffer_only();
  if ( border)
    ctx->gdraw->rect_erase( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, idx);
  ctx->gdraw->fill_rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, glow_eDrawType_LineErase);
  w->reset_draw_buffer_only();
}

void GrowBar::trace_scan()
{
  if ( !trace.p)
    return;

  if ( ctx->trace_scan_func)
  {
    ctx->trace_scan_func( (void *) this, trace.p);
    return;
  }    
}

int GrowBar::trace_init()
{
  int sts;

  sts = ctx->trace_connect_func( (void *) this, &trace);
  return sts;
}

void GrowBar::trace_close()
{
  if ( trace.p)
    ctx->trace_disconnect_func( (void *) this);
}

void GrowBar::draw()
{
  ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->navw, x_left * ctx->navw.zoom_factor_x - ctx->navw.offset_x - 1,
	     y_low * ctx->navw.zoom_factor_y - ctx->navw.offset_y - 1,
  	     x_right * ctx->navw.zoom_factor_x - ctx->navw.offset_x + 1,
	     y_high * ctx->navw.zoom_factor_y - ctx->navw.offset_y + 1);
}

void GrowBar::align( double x, double y, glow_eAlignDirection direction)
{
    double dx, dy;

    erase( &ctx->mw);
    erase( &ctx->navw);
    ctx->set_defered_redraw();
    draw();
    switch ( direction) {
      case glow_eAlignDirection_CenterVert:
        dx = x - (x_right + x_left) / 2;
        dy = 0;
        break;        
      case glow_eAlignDirection_CenterHoriz:
        dx = 0;
        dy = y - (y_high + y_low) / 2;
        break;        
      case glow_eAlignDirection_CenterCenter:
        dx = x - (x_right + x_left) / 2;
        dy = y - (y_high + y_low) / 2;
        break;        
      case glow_eAlignDirection_Right:
        dx = x - x_right;
        dy = 0;
        break;        
      case glow_eAlignDirection_Left:
        dx = x - x_left;
        dy = 0;
        break;        
      case glow_eAlignDirection_Up:
        dx = 0;
        dy = y - y_high;
        break;        
      case glow_eAlignDirection_Down:
        dx = 0;
        dy = y - y_low;
        break;        
    }
    trf.move( dx, dy);
    x_right += dx;
    x_left += dx;
    y_high += dy;
    y_low += dy;

    draw();
    ctx->redraw_defered();
}

void GrowBar::set_trace_attr( GlowTraceData *attr)
{
  memcpy( &trace, attr, sizeof(trace));
}

void GrowBar::get_trace_attr( GlowTraceData **attr)
{
  *attr = &trace;
}

void GrowBar::set_range( double min, double max)
{ 
  max_value = max;
  min_value = min;
  draw();
}

void GrowBar::export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp)
{
  double x1, y1, x2, y2, ll_x, ll_y, ur_x, ur_y;
  double rotation;

  if (!t) {
    x1 = trf.x( ll.x, ll.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y1 = trf.y( ll.x, ll.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
    x2 = trf.x( ur.x, ur.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y2 = trf.y( ur.x, ur.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
  }
  else {
    x1 = trf.x( t, ll.x, ll.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y1 = trf.y( t, ll.x, ll.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
    x2 = trf.x( t, ur.x, ur.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y2 = trf.y( t, ur.x, ur.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
  }

  ll_x = min( x1, x2);
  ur_x = max( x1, x2);
  ll_y = min( y1, y2);
  ur_y = max( y1, y2);

  if ( t)
    rotation = (trf.rot( t) / 360 - floor( trf.rot( t) / 360)) * 360;
  else
    rotation = (trf.rot() / 360 - floor( trf.rot() / 360)) * 360;

  ctx->export_jbean->bar( ll_x, ll_y, ur_x, ur_y,
    	draw_type, fill_drawtype, bar_drawtype, bar_bordercolor, fill,
	border, min_value, max_value, bar_borderwidth,
    	line_width, rotation,
    	pass, shape_cnt, node_cnt, fp);
}

void GrowBar::set_bar_info( glow_sBarInfo *info)
{
  max_value = info->max_value;
  min_value = info->min_value;
  bar_drawtype = info->bar_drawtype;
  bar_bordercolor = info->bar_bordercolor;
  bar_borderwidth = info->bar_borderwidth;
}

void GrowBar::get_bar_info( glow_sBarInfo *info)
{
  info->max_value = max_value;
  info->min_value = min_value;
  info->bar_drawtype = bar_drawtype;
  info->bar_bordercolor = bar_bordercolor;
  info->bar_borderwidth = bar_borderwidth;
}

void GrowBar::convert( glow_eConvert version) 
{
  switch ( version) {
  case glow_eConvert_V34: {
    // Conversion of colors
    GrowRect::convert( version);
    bar_drawtype = GlowColor::convert( version, bar_drawtype);
    bar_bordercolor = GlowColor::convert( version, bar_bordercolor);

    break;
  }
  }  
}
