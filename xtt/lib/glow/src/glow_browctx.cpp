/* 
 * Proview   $Id: glow_browctx.cpp,v 1.4 2008-10-31 12:51:35 claes Exp $
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
#include <fstream>
#include <math.h>
#include <float.h>
#include "glow_ctx.h"
#include "glow_browctx.h"
#include "glow_node.h"
#include "glow_point.h"
#include "glow_rect.h"
#include "glow_line.h"
#include "glow_arc.h"
#include "glow_text.h"
#include "glow_conpoint.h"
#include "glow_node.h"
#include "glow_nodeclass.h"
#include "glow_con.h"
#include "glow_conclass.h"
#include "glow_draw.h"

int BrowCtx::insert( GlowArrayElem *element, GlowArrayElem *destination, 
		glow_eDest destination_code)
{
  int sts;
  double y;

  sts = a.brow_insert( element, destination, destination_code);
  if ( !sts) return sts;

  if ( destination)
    y = ((GlowNode *)destination)->y_low;
  else
   y = 0;
  configure( y);
  return 1;
}

void BrowCtx::remove( GlowArrayElem *element)
{
  double pos = ((GlowNode *)element)->y_low;

  set_nodraw();
  a.brow_remove( this, element);
  reset_nodraw();
  configure( pos);
}

void BrowCtx::close( GlowArrayElem *element)
{
  set_nodraw();
  a.brow_close( this, element);
  reset_nodraw();
  configure( ((GlowNode *)element)->y_low);
}

void BrowCtx::configure( double y_redraw) 
{
  if ( nodraw) 
    return;
  
  a.configure();
  get_borders();
  frame_x_right = max( x_right, 
	1.0 * (mw.window_width + mw.offset_x) / mw.zoom_factor_x);
  a.zoom();
  redraw( y_redraw);
  change_scrollbar();
}

void BrowCtx::change_scrollbar()
{
  glow_sScroll data;

  if ( !scroll_size)
  {
    // Get scroll size from the width of the first element
    double ll_x, ur_x, ll_y, ur_y;

    if ( a.size() == 0)
      return;
    ((GlowNode *)a[0])->measure( &ll_x, &ll_y, &ur_x, &ur_y);
    scroll_size = ur_y - ll_y + 1.0 / mw.zoom_factor_y;
  }

  data.scroll_data = scroll_data;
  data.total_width = int( (x_right - x_left) / scroll_size);
  data.total_height = int( (y_high - y_low) / scroll_size);
  data.window_width = int( mw.window_width / scroll_size / mw.zoom_factor_x);
  data.window_height = int( mw.window_height / scroll_size / mw.zoom_factor_y + 1);
  data.offset_x = int( mw.offset_x / scroll_size / mw.zoom_factor_x - x_left / 
	scroll_size);
  data.offset_y = int( mw.offset_y /scroll_size / mw.zoom_factor_y - y_low / 
	scroll_size);

  (scroll_callback)( &data);
}


void BrowCtx::redraw( double y_redraw)
{
  if ( y_redraw)
  {
    gdraw->clear_area( &mw, 0, mw.window_width, 
		int(y_redraw * mw.zoom_factor_y - mw.offset_y), mw.window_height);
    draw( &mw, 0, (int)(y_redraw * mw.zoom_factor_y - mw.offset_y), mw.window_width, mw.window_height);
  }
  else
  {
    clear( &mw);
    draw( &mw, 0, 0, mw.window_width, mw.window_height);
  }
  nav_zoom();
}

void BrowCtx::zoom( double factor)
{ 
  if ( fabs(factor) < DBL_EPSILON)
    return;

  mw.zoom_factor_x *= factor;
  mw.zoom_factor_y *= factor;
  if ( mw.offset_x != 0)
    mw.offset_x = int( (mw.offset_x - mw.window_width / 2.0 * ( 1.0/factor - 1)) 
		* factor);
  if ( mw.offset_y != 0)
    mw.offset_y = int( (mw.offset_y  - mw.window_height / 2.0 * ( 1.0/factor - 1)) 
		* factor);
  mw.offset_x = max( mw.offset_x, 0);
  mw.offset_y = max( mw.offset_y, 0);
  if ( (x_right - x_left) * mw.zoom_factor_x <= mw.window_width)
    mw.offset_x = 0;
  if ( (y_high - y_low) * mw.zoom_factor_y <= mw.window_height)
    mw.offset_y = 0;
  a.zoom();
  clear( &mw);
  draw( &mw, 0, 0, mw.window_width, mw.window_height);
  nav_zoom();
  change_scrollbar();
}

void BrowCtx::print( char *filename)
{
  int		i;
  double ll_x, ll_y, ur_x, ur_y;
  double width, height;

  if ( a.size() == 0)
    return;
  ((GlowNode *)a[0])->measure( &ll_x, &ll_y, &ur_x, &ur_y);
  height = 60 * (ur_y - ll_y);
  width = 0.70 * height;

  print_ps = new GlowPscript( filename, this, 1);

  for ( i = 0;; i++)
  {
    ll_y = i * height;
    ur_y = ll_y + height;
    ll_x = 0;
    ur_x = width;
    if (  ll_y > y_high)
      break;

    print_ps->print_page( ll_x, ll_y, ur_x, ur_y);
  }
  delete print_ps;
}


int BrowCtx::is_visible( GlowArrayElem *element)
{
  double ll_x, ll_y, ur_x, ur_y;
  double window_low, window_high;

  ((GlowNode *)element)->measure( &ll_x, &ll_y, &ur_x, &ur_y);
  window_low = double(mw.offset_y) / mw.zoom_factor_y;
  window_high = double(mw.offset_y + mw.window_height) / mw.zoom_factor_y;
  if ( ll_y >= window_low && ur_y <= window_high)
    return 1;
  else
    return 0;
}

void BrowCtx::center_object( GlowArrayElem *object, double factor)
{
  double ll_x, ll_y, ur_x, ur_y;
  int new_offset_y;
  int y_pix;

  ((GlowNode *)object)->measure( &ll_x, &ll_y, &ur_x, &ur_y);
  new_offset_y = int(ll_y * mw.zoom_factor_y - factor * mw.window_height);
  if ( new_offset_y <= 0)
    y_pix = mw.offset_y;
  else
    y_pix = - (new_offset_y - mw.offset_y);
  scroll( 0, y_pix);
  change_scrollbar();
}

void brow_scroll_horizontal( BrowCtx *ctx, int value, int bottom)
{
  int x_pix;

  x_pix = int( - value * ctx->scroll_size * ctx->mw.zoom_factor_x + 
	(ctx->mw.offset_x - ctx->x_left * ctx->mw.zoom_factor_x));
  ctx->scroll( x_pix, 0);
}

void brow_scroll_vertical( BrowCtx *ctx, int value, int bottom)
{
  int y_pix;

  y_pix = int( - value * ctx->scroll_size * ctx->mw.zoom_factor_y + 
	(ctx->mw.offset_y - ctx->y_low * ctx->mw.zoom_factor_y));
  // Correction for the bottom position
  if ( bottom && ( y_pix >= 0 ||
	ctx->mw.window_height + y_pix < ctx->y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y))
//        mw.window_height >= (y_high - y_low) * mw.zoom_factor_y)
    y_pix = int( ctx->mw.window_height + ctx->mw.offset_y - 
		ctx->y_high * ctx->mw.zoom_factor_y);
  ctx->scroll( 0, y_pix);
}
