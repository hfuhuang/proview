/* 
 * Proview   $Id: glow_arrow.cpp,v 1.3 2007-01-04 07:57:38 claes Exp $
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


#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "glow.h"
#include "glow_arrow.h"
#include "glow_draw.h"

#define max(Dragon,Eagle) ((Dragon) > (Eagle) ? (Dragon) : (Eagle))
#define min(Dragon,Eagle) ((Dragon) < (Eagle) ? (Dragon) : (Eagle))


GlowArrow::GlowArrow( GrowCtx *glow_ctx, double x1, double y1, double x2, 
	double y2, double w, double l, glow_eDrawType d_type) : 
	ctx(glow_ctx), p_dest(glow_ctx,x2,y2),
	arrow_width(w), arrow_length(l), draw_type(d_type), line_width(1)
{
  double p1_x, p1_y, p2_x, p2_y;

  if ( fabs( x2 - x1) < DBL_EPSILON)
  {
    if ( y1 > y2)
    {
      p1_x = x2 + w/2;
      p1_y = y2 + l;
      p2_x = x2 - w/2;
      p2_y = y2 + l;
    }
    else
    {
      p1_x = x2 + w/2;
      p1_y = y2 - l;
      p2_x = x2 - w/2;
      p2_y = y2 - l;
    }
  }
  else if ( fabs( y2 - y1) < DBL_EPSILON)
  {
    if ( x1 > x2)
    {
      p1_x = x2 + l;
      p1_y = y2 + w/2;
      p2_x = x2 + l;
      p2_y = y2 - w/2;
    }
    else
    {
      p1_x = x2 - l;
      p1_y = y2 - w/2;
      p2_x = x2 - l;
      p2_y = y2 + w/2;
    }
  }
  else
  {
    double d = sqrt( (y1-y2)*(y1-y2) + (x1-x2)*(x1-x2));
    p1_x = x2 + (x1-x2)*l/d + (y1-y2)*w/d/2;
    p1_y = y2 + (y1-y2)*l/d - (x1-x2)*w/d/2;
    p2_x = x2 + (x1-x2)*l/d - (y1-y2)*w/d/2;
    p2_y = y2 + (y1-y2)*l/d + (x1-x2)*w/d/2;
  }
  p1 = GlowPoint(glow_ctx, p1_x, p1_y);
  p2 = GlowPoint(glow_ctx, p2_x, p2_y);
}

void GlowArrow::zoom()
{
  p_dest.zoom();
  p1.zoom();
  p2.zoom();
}

void GlowArrow::nav_zoom()
{
  p_dest.nav_zoom();
  p1.nav_zoom();
  p2.nav_zoom();
}

void GlowArrow::save( ofstream& fp, glow_eSaveMode mode)
{
  fp << int(glow_eSave_Arrow) << endl;
  fp << int(glow_eSave_Arrow_arrow_width) << FSPACE << arrow_width << endl;
  fp << int(glow_eSave_Arrow_arrow_length) << FSPACE << arrow_length << endl;
  fp << int(glow_eSave_Arrow_draw_type) << FSPACE << int(draw_type) << endl;
  fp << int(glow_eSave_Arrow_line_width) << FSPACE << line_width << endl;
  fp << int(glow_eSave_Arrow_p_dest) << endl;
  p_dest.save( fp, mode);
  fp << int(glow_eSave_Arrow_p1) << endl;
  p1.save( fp, mode);
  fp << int(glow_eSave_Arrow_p2) << endl;
  p2.save( fp, mode);
  fp << int(glow_eSave_End) << endl;
}

void GlowArrow::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_Arrow: break;
      case glow_eSave_Arrow_arrow_width: fp >> arrow_width; break;
      case glow_eSave_Arrow_arrow_length: fp >> arrow_length; break;
      case glow_eSave_Arrow_draw_type: fp >> tmp; draw_type = (glow_eDrawType)tmp; break;
      case glow_eSave_Arrow_line_width: fp >> line_width; break;
      case glow_eSave_Arrow_p_dest: p_dest.open( fp); break;
      case glow_eSave_Arrow_p1: p1.open( fp); break;
      case glow_eSave_Arrow_p2: p2.open( fp); break;
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GlowArrow:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
}

void GlowArrow::draw( GlowWind *w, void *pos, int highlight, int hot, void *node)
{
  int p1_x, p1_y, p2_x, p2_y, p_dest_x, p_dest_y;
  if ( w == &ctx->navw) {
    if ( ctx->no_nav)
      return;
    hot = 0;
    p1_x = p1.nav_z_x;
    p1_y = p1.nav_z_y;
    p2_x = p2.nav_z_x;
    p2_y = p2.nav_z_y;
    p_dest_x = p_dest.nav_z_x;
    p_dest_y = p_dest.nav_z_y;
  }
  else {
    p1_x = p1.z_x;
    p1_y = p1.z_y;
    p2_x = p2.z_x;
    p2_y = p2.z_y;
    p_dest_x = p_dest.z_x;
    p_dest_y = p_dest.z_y;
  }
  int idx = int( w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
  idx += hot;
  idx = max( 0, idx);
  idx = min( idx, DRAW_TYPE_SIZE-1);
  ctx->gdraw->arrow( w,
	p_dest_x + ((GlowPoint *)pos)->z_x - w->offset_x, 
	p_dest_y + ((GlowPoint *)pos)->z_y - w->offset_y, 
	p1_x + ((GlowPoint *)pos)->z_x - w->offset_x, 
	p1_y + ((GlowPoint *)pos)->z_y - w->offset_y, 
	p2_x + ((GlowPoint *)pos)->z_x - w->offset_x, 
	p2_y + ((GlowPoint *)pos)->z_y - w->offset_y, 
	draw_type, idx, highlight);
}

void GlowArrow::erase( GlowWind *w, void *pos, int hot, void *node)
{
  int p1_x, p1_y, p2_x, p2_y, p_dest_x, p_dest_y;
  if ( w == &ctx->navw) {
    if ( ctx->no_nav)
      return;
    hot = 0;
    p1_x = p1.nav_z_x;
    p1_y = p1.nav_z_y;
    p2_x = p2.nav_z_x;
    p2_y = p2.nav_z_y;
    p_dest_x = p_dest.nav_z_x;
    p_dest_y = p_dest.nav_z_y;
  }
  else {
    p1_x = p1.z_x;
    p1_y = p1.z_y;
    p2_x = p2.z_x;
    p2_y = p2.z_y;
    p_dest_x = p_dest.z_x;
    p_dest_y = p_dest.z_y;
  }
  int idx = int( w->zoom_factor_y / w->base_zoom_factor * line_width - 1);
  idx += hot;
  idx = max( 0, idx);
  idx = min( idx, DRAW_TYPE_SIZE-1);
  ctx->gdraw->arrow_erase( w,
	p_dest_x + ((GlowPoint *)pos)->z_x - w->offset_x, 
	p_dest_y + ((GlowPoint *)pos)->z_y - w->offset_y, 
	p1_x + ((GlowPoint *)pos)->z_x - w->offset_x, 
	p1_y + ((GlowPoint *)pos)->z_y - w->offset_y, 
	p2_x + ((GlowPoint *)pos)->z_x - w->offset_x, 
	p2_y + ((GlowPoint *)pos)->z_y - w->offset_y, 
	idx);
}


void GlowArrow::move( void *pos, double x1, double y1, double x2, double y2,
	int highlight, int hot)
{
  erase( &ctx->mw, pos, hot, NULL);
  erase( &ctx->navw, pos, 0, NULL);

  if ( fabs( x2 - x1) < DBL_EPSILON) {
    if ( y1 > y2) {
      p1.x = x2 + arrow_width/2;
      p1.y = y2 + arrow_length;
      p2.x = x2 - arrow_width/2;
      p2.y = y2 + arrow_length;
    }
    else {
      p1.x = x2 + arrow_width/2;
      p1.y = y2 - arrow_length;
      p2.x = x2 - arrow_width/2;
      p2.y = y2 - arrow_length;
    }
  }
  else if ( fabs( y2 - y1) < DBL_EPSILON) {
    if ( x1 > x2) {
      p1.x = x2 + arrow_length;
      p1.y = y2 + arrow_width/2;
      p2.x = x2 + arrow_length;
      p2.y = y2 - arrow_width/2;
    }
    else {
      p1.x = x2 - arrow_length;
      p1.y = y2 - arrow_width/2;
      p2.x = x2 - arrow_length;
      p2.y = y2 + arrow_width/2;
    }
  }
  else {
    double d = sqrt( (y1-y2)*(y1-y2) + (x1-x2)*(x1-x2));
    p1.x = x2 + (x1-x2)*arrow_length/d + (y1-y2)*arrow_width/d/2;
    p1.y = y2 + (y1-y2)*arrow_length/d - (x1-x2)*arrow_width/d/2;
    p2.x = x2 + (x1-x2)*arrow_length/d - (y1-y2)*arrow_width/d/2;
    p2.y = y2 + (y1-y2)*arrow_length/d + (x1-x2)*arrow_width/d/2;
  }
  p_dest.x = x2;
  p_dest.y = y2;
  zoom();
  nav_zoom();
  draw( &ctx->mw, pos, highlight, hot, NULL);
  draw( &ctx->navw, pos, highlight, 0, NULL);
}

void GlowArrow::shift( void *pos, double delta_x, double delta_y, 
	int highlight, int hot)
{
  erase( &ctx->mw, pos, hot, NULL);
  erase( &ctx->navw, pos, 0, NULL);
  p_dest.x += delta_x;
  p_dest.y += delta_y;
  p1.x += delta_x;
  p1.y += delta_y;
  p2.x += delta_x;
  p2.y += delta_y;
  zoom();
  nav_zoom();

  draw( &ctx->mw, pos, highlight, hot, NULL);
  draw( &ctx->navw, pos, highlight, 0, NULL);
}

int GlowArrow::event_handler( GlowWind *w, void *pos, glow_eEvent event, int x, int y,
	void *node)
{
    return 0;
}

