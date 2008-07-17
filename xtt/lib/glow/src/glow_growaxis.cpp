/* 
 * Proview   $Id: glow_growaxis.cpp,v 1.10 2008-07-17 11:23:53 claes Exp $
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


#include <iostream.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include "glow_growaxis.h"
#include "glow_grownode.h"
#include "glow_draw.h"
#include "glow_growctx.h"
#include "pwr.h"
#include "co_time.h"

GrowAxis::GrowAxis( GrowCtx *glow_ctx, char *name, double x1, double y1, 
		double x2, double y2, glow_eDrawType border_d_type, 
		int line_w, int t_size, glow_eDrawType t_drawtype, 
	        int nodraw) : 
		GrowRect(glow_ctx,name,x1,y1,x2-x1,y2-y1,border_d_type,line_w,
		0,glow_mDisplayLevel_1,0,1,0, glow_eDrawType_Line,1),
		text_size(t_size), text_drawtype(t_drawtype), text_color_drawtype(glow_eDrawType_Line),
                max_value(100), min_value(0), lines(11),
		longquotient(1),
                valuequotient(1), increment(0)
{
  strcpy( format, "%3.0f");

  configure();
  if ( !nodraw)
    draw( &ctx->mw, (GlowTransform *)NULL, highlight, hot, NULL, NULL);

}

GrowAxis::~GrowAxis()
{
  if ( ctx->nodraw) return;
  erase( &ctx->mw);
  erase( &ctx->navw);
}

void GrowAxis::configure() 
{
  if ( lines <= 1)
    lines = 2;
  if ( longquotient <= 0)
    longquotient = 1;
  if ( valuequotient <= 0)
    valuequotient = 1;
  increment = (max_value - min_value) / (lines - 1);
}

void GrowAxis::save( ofstream& fp, glow_eSaveMode mode) 
{ 

  fp << int(glow_eSave_GrowAxis) << endl;
  fp << int(glow_eSave_GrowAxis_max_value) << FSPACE << max_value << endl;
  fp << int(glow_eSave_GrowAxis_min_value) << FSPACE << min_value << endl;
  fp << int(glow_eSave_GrowAxis_rect_part) << endl;
  GrowRect::save( fp, mode);
  fp << int(glow_eSave_GrowAxis_lines) << FSPACE << lines << endl;
  fp << int(glow_eSave_GrowAxis_longquotient) << FSPACE << longquotient << endl;
  fp << int(glow_eSave_GrowAxis_valuequotient) << FSPACE << valuequotient << endl;
  fp << int(glow_eSave_GrowAxis_format) << FSPACE << format << endl;
  fp << int(glow_eSave_GrowAxis_text_size) << FSPACE << text_size << endl;
  fp << int(glow_eSave_GrowAxis_text_drawtype) << FSPACE << int(text_drawtype) << endl;
  fp << int(glow_eSave_GrowAxis_text_color_drawtype) << FSPACE << int(text_color_drawtype) << endl;
  fp << int(glow_eSave_End) << endl;
}

void GrowAxis::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int           tmp;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_GrowAxis: break;
      case glow_eSave_GrowAxis_max_value: fp >> max_value; break;
      case glow_eSave_GrowAxis_min_value: fp >> min_value; break;
      case glow_eSave_GrowAxis_rect_part: 
        GrowRect::open( fp);
        break;
      case glow_eSave_GrowAxis_lines: fp >> lines; break;
      case glow_eSave_GrowAxis_longquotient: fp >> longquotient; break;
      case glow_eSave_GrowAxis_valuequotient: fp >> valuequotient; break;
      case glow_eSave_GrowAxis_format:
        fp.get();
        fp.getline( format, sizeof(format));
        break;
      case glow_eSave_GrowAxis_text_size: fp >> text_size; break;
      case glow_eSave_GrowAxis_text_drawtype: fp >> tmp; text_drawtype = (glow_eDrawType)tmp; break;
      case glow_eSave_GrowAxis_text_color_drawtype: fp >> tmp; text_color_drawtype = (glow_eDrawType)tmp; break;
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GrowAxis:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }
  configure();
}

void GrowAxis::draw( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y) 
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

  if ( x_right * w->zoom_factor_x - w->offset_x >= ll_x &&
      	x_left * w->zoom_factor_x - w->offset_x <= ur_x &&
       	y_high * w->zoom_factor_y - w->offset_y >= ll_y &&
       	y_low * w->zoom_factor_y - w->offset_y <= ur_y) {
    draw( w, (GlowTransform *)NULL, highlight, hot, NULL, NULL);
  }
}

void GrowAxis::draw( GlowWind *w, int *ll_x, int *ll_y, int *ur_x, int *ur_y) 
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

void GrowAxis::set_highlight( int on)
{
  highlight = on;
  draw();
}

void GrowAxis::draw( GlowWind *w, GlowTransform *t, int highlight, int hot, void *node, 
		     void *colornode)
{
  if ( ctx->nodraw)
    return;
  if ( w == &ctx->navw) {
    if ( ctx->no_nav)
      return;
    hot = 0;
  }
  int i;
  int draw_text = (fabs(increment) > DBL_EPSILON);
  int idx;
  int x, y;
  char text[20];
  int line_length;
  int x_text, y_text;
  int z_height, z_width, z_descent;
  int max_z_width = 0;
  double rotation;
  glow_eDrawType drawtype;
  int text_idx = int( w->zoom_factor_y / w->base_zoom_factor * (text_size +4) - 4);
  text_idx = min( text_idx, DRAW_TYPE_SIZE-1);

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
    rotation = (trf.rot() / 360 - floor( trf.rot() / 360)) * 360;
  }
  else {
    x1 = int( trf.x( t, ll.x, ll.y) * w->zoom_factor_x) - w->offset_x;
    y1 = int( trf.y( t, ll.x, ll.y) * w->zoom_factor_y) - w->offset_y;
    x2 = int( trf.x( t, ur.x, ur.y) * w->zoom_factor_x) - w->offset_x;
    y2 = int( trf.y( t, ur.x, ur.y) * w->zoom_factor_y) - w->offset_y;
    rotation = (trf.rot( t) / 360 - floor( trf.rot( t) / 360)) * 360;
  }

  ll_x = min( x1, x2);
  ur_x = max( x1, x2);
  ll_y = min( y1, y2);
  ur_y = max( y1, y2);
  drawtype = ctx->get_drawtype( draw_type, glow_eDrawType_LineHighlight,
				highlight, (GrowNode *)colornode, 0);

  if ( 45 >= rotation || rotation > 315) {
    // Vertical line to the right and values to the left

    ctx->gdraw->line( w, ur_x, ll_y, ur_x, ur_y, drawtype, idx, 0);

    // Calculate max value text width
    if ( draw_text) {
      for ( i = 0; i < lines; i++) {
	if ( i % valuequotient == 0) {
	  format_text( text, format, max_value - i * increment);
	  ctx->gdraw->get_text_extent( text, strlen(text), text_drawtype,
				       max( 0, text_idx), glow_eFont_Helvetica, 
				       &z_width, &z_height, &z_descent);
	  if ( max_z_width < z_width)
	    max_z_width = z_width;
	}
      }
      x_text = ll_x + max_z_width;
      line_length = ur_x - ll_x - max_z_width;
      if ( line_length < 3)
	line_length = 3;
    }
    else {
      x_text = ll_x;
      line_length = ur_x - ll_x;
    }

    for ( i = 0; i < lines; i++) {
      y = int( ll_y + double(ur_y - ll_y) / (lines - 1) * i);
      if ( i % longquotient == 0)
        ctx->gdraw->line( w, ur_x - line_length, y, 
          ur_x, y, drawtype, idx, 0);
      else
        ctx->gdraw->line( w, ur_x -  int( 2.0 / 3 * line_length), y, 
          ur_x, y, drawtype, idx, 0);
      if ( draw_text) {
	format_text( text, format, max_value - i * increment);

	if ( text_idx >= 0 && max_z_width < ur_x - ll_x &&
	     i % valuequotient == 0) {
	  if ( i == lines - 1)
	    y_text = y;
	  else if ( i == 0)
	    y_text = y + z_height - z_descent - 3;
	  else
	    y_text = y + (z_height-z_descent)/2;
	  ctx->gdraw->text( w, ll_x, y_text,
			    text, strlen(text), text_drawtype, text_color_drawtype, 
			    text_idx, highlight, 0, glow_eFont_Helvetica);
	}
      }
    }
  }
  else if ( 45 < rotation && rotation <= 135)   {
    // Horizontal line at bottom and values to the top

    ctx->gdraw->line( w, ll_x, ur_y, ur_x, ur_y, drawtype, idx, 0);

    // Calculate max value text height
    if ( draw_text) {
      ctx->gdraw->get_text_extent( "0", 1, text_drawtype, 
				   max( 0, text_idx), glow_eFont_Helvetica, 
				   &z_width, &z_height, &z_descent);

      line_length = ur_y - ll_y - z_height;
      if ( line_length < 3)
	line_length = 3;
    }
    else {
      line_length = ur_y - ll_y;
    }

    for ( i = 0; i < lines; i++) {
      x = int( ll_x + double(ur_x - ll_x) / (lines - 1) * (lines - 1- i));
      if ( i % longquotient == 0)
        ctx->gdraw->line( w, x, ur_y - line_length, x, 
          ur_y, drawtype, idx, 0);
      else
        ctx->gdraw->line( w, x, ur_y -  int( 2.0 / 3 * line_length), x, 
          ur_y, drawtype, idx, 0);

      if ( draw_text && i % valuequotient == 0) {
        format_text( text, format, max_value - i * increment);
        ctx->gdraw->get_text_extent( text, strlen(text), text_drawtype, 
				     max( 0, text_idx), glow_eFont_Helvetica, 
				     &z_width, &z_height, &z_descent);

        if ( text_idx >= 0 && z_height < ur_y - ll_y ) {
          if ( i == lines - 1)
            x_text = x;
          else if ( i == 0)
            x_text = x - z_width;
          else
            x_text = x - (z_width)/2;
          ctx->gdraw->text( w, x_text, ll_y + z_height - z_descent,
			    text, strlen(text), text_drawtype, text_color_drawtype, 
			    text_idx, highlight, 0, glow_eFont_Helvetica);
        }
      }
    }
  }
  else if ( 135 < rotation && rotation <= 225) {
    // Vertical line to the left and values to the right

    ctx->gdraw->line( w, ll_x, ll_y, ll_x, ur_y, drawtype, idx, 0);

    // Calculate max value text width
    if ( draw_text) {
      for ( i = 0; i < lines; i++) {
	if ( i % valuequotient == 0) {
	  format_text( text, format, max_value - i * increment);
	  ctx->gdraw->get_text_extent( text, strlen(text), text_drawtype, 
				       max( 0, text_idx), glow_eFont_Helvetica, 
				       &z_width, &z_height, &z_descent);
	  if ( max_z_width < z_width)
	    max_z_width = z_width;
	}
      }
      x_text = ur_x - max_z_width;
      line_length = ur_x - ll_x - max_z_width;
      if ( line_length < 3)
	line_length = 3;
    }
    else {
      x_text = ur_x;
      line_length = ur_x - ll_x;
    }

    for ( i = 0; i < lines; i++) {
      y = int( ll_y + double(ur_y - ll_y) / (lines - 1) * ( lines - 1 - i));
      if ( i % longquotient == 0)
        ctx->gdraw->line( w, ll_x, y, 
          ll_x + line_length, y, drawtype, idx, 0);
      else
        ctx->gdraw->line( w, ll_x, y, 
          ll_x + int( 2.0 / 3 * line_length), y, drawtype, idx, 0);
      format_text( text, format, max_value - i * increment);

      if ( draw_text && 
	   text_idx >= 0 && max_z_width < ur_x - ll_x &&
           i % valuequotient == 0) {
        if ( i == lines - 1)
          y_text = y + z_height - z_descent - 3;
        else if ( i == 0)
          y_text = y;
        else
          y_text = y + (z_height-z_descent)/2;
        ctx->gdraw->text( w, x_text, y_text,
			  text, strlen(text), text_drawtype, text_color_drawtype, 
			  text_idx, highlight, 0, glow_eFont_Helvetica);
      }
    }
  }
  else { // if ( 225 < rotation && rotation <= 315)
    // Horizontal line at top and values at the bottom

    ctx->gdraw->line( w, ll_x, ll_y, ur_x, ll_y, drawtype, idx, 0);

    // Calculate max value text height
    if ( draw_text) {
      ctx->gdraw->get_text_extent( "0", 1, text_drawtype, 
				   max( 0, text_idx), glow_eFont_Helvetica, 
				   &z_width, &z_height, &z_descent);

      line_length = ur_y - ll_y - (z_height - z_descent);
      if ( line_length < 3)
	line_length = 3;
    }
    else {
      line_length = ur_y - ll_y;
    }

    for ( i = 0; i < lines; i++) {
      x = int( ll_x + double(ur_x - ll_x) / (lines - 1) * i);
      if ( i % longquotient == 0)
        ctx->gdraw->line( w, x, ll_y, x, 
          ll_y + line_length, drawtype, idx, 0);
      else
        ctx->gdraw->line( w, x, ll_y, x, 
          ll_y  +  int( 2.0 / 3 * line_length), drawtype, idx, 0);
      if ( draw_text && i % valuequotient == 0) {
        format_text( text, format, max_value - i * increment);
        ctx->gdraw->get_text_extent( text, strlen(text), text_drawtype, 
				     max( 0, text_idx), glow_eFont_Helvetica, 
				     &z_width, &z_height, &z_descent);

        if ( text_idx >= 0 && z_height - z_descent < ur_y - ll_y) {
          if ( i == lines - 1)
            x_text = x - z_width;
          else if ( i == 0)
            x_text = x;
          else
            x_text = x - (z_width)/2;
          ctx->gdraw->text( w, x_text, ur_y,
			    text, strlen(text), text_drawtype, text_color_drawtype, 
			    text_idx, highlight, 0, glow_eFont_Helvetica);
        }
      }
    }
  }

}

void GrowAxis::erase( GlowWind *w, GlowTransform *t, int hot, void *node)
{
  if ( ctx->nodraw)
    return;
  if ( w == &ctx->navw) {
    if ( ctx->no_nav)
      return;
    hot = 0;
  }
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
  ctx->gdraw->fill_rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, glow_eDrawType_LineErase);
  w->reset_draw_buffer_only();
}

void GrowAxis::draw()
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

void GrowAxis::align( double x, double y, glow_eAlignDirection direction)
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

void GrowAxis::set_textsize( int size) 
{ 
  erase( &ctx->mw);
  erase( &ctx->navw);
  text_size = size;
  get_node_borders();
  draw();
}

void GrowAxis::set_textbold( int bold) 
{ 
  if ( ( bold && text_drawtype == glow_eDrawType_TextHelveticaBold) ||
       ( !bold && text_drawtype == glow_eDrawType_TextHelvetica))
    return;

  erase( &ctx->mw);
  erase( &ctx->navw);
  if ( bold)
    text_drawtype = glow_eDrawType_TextHelveticaBold;
  else
    text_drawtype = glow_eDrawType_TextHelvetica;
  get_node_borders();
  draw();
}

void GrowAxis::set_range( double min, double max)
{
  max_value = max;
  min_value = min;
  configure();
}

void GrowAxis::export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp)
{
  int i;
  int draw_text = (fabs(increment) > DBL_EPSILON);
  double x1, y1, x2, y2, ll_x, ll_y, ur_x, ur_y;
  double rotation;
  int bold;
  char text[20];
  int line_length;
  int z_height, z_width, z_descent;
  int max_z_width = 0;
  int idx = int( ctx->mw.zoom_factor_y / ctx->mw.base_zoom_factor * (text_size +4) - 4);
  idx = min( idx, DRAW_TYPE_SIZE-1);

  bold = (text_drawtype == glow_eDrawType_TextHelveticaBold);

  if (!t)
  {
    x1 = trf.x( ll.x, ll.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y1 = trf.y( ll.x, ll.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
    x2 = trf.x( ur.x, ur.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y2 = trf.y( ur.x, ur.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
  }
  else
  {
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

  // Calculate max value line width
  if ( 45 >= rotation || rotation > 315)
  {
    if ( draw_text) {
      for ( i = 0; i < lines; i++) {
	if ( i % valuequotient == 0) {
	  format_text( text, format, max_value - i * increment);
	  ctx->gdraw->get_text_extent( text, strlen(text), text_drawtype, 
				       max( 0, idx), glow_eFont_Helvetica, 
				       &z_width, &z_height, &z_descent);
	  if ( max_z_width < z_width)
	    max_z_width = z_width;
	}
      }
      line_length = int(ur_x - ll_x) - max_z_width;
    }
    else
      line_length = int(ur_x - ll_x);
  }
  else if ( 45 < rotation && rotation <= 135)  
  {
    if ( draw_text) {
      ctx->gdraw->get_text_extent( "0", 1, text_drawtype, 
				   max( 0, idx), glow_eFont_Helvetica, 
				   &z_width, &z_height, &z_descent);

      line_length = int(ur_y - ll_y) - (z_height - z_descent);
    }
    else
      line_length = int(ur_y - ll_y);
  }
  else if ( 135 < rotation && rotation <= 225)
  {
    if ( draw_text) {
      for ( i = 0; i < lines; i++) {
	if ( i % valuequotient == 0) {
	  format_text( text, format, max_value - i * increment);
	  ctx->gdraw->get_text_extent( text, strlen(text), text_drawtype, 
				       max( 0, idx), glow_eFont_Helvetica, 
				       &z_width, &z_height, &z_descent);
	  if ( max_z_width < z_width)
	    max_z_width = z_width;
	}
      }
      line_length = int(ur_x - ll_x) - max_z_width;
    }
    else
      line_length = int(ur_x - ll_x);
  }
  else // if ( 225 < rotation && rotation <= 315)
  {
    if ( draw_text) {
      ctx->gdraw->get_text_extent( "0", 1, text_drawtype, 
				   max( 0, idx), glow_eFont_Helvetica, 
				   &z_width, &z_height, &z_descent);

      line_length = int(ur_y - ll_y) - (z_height - z_descent);
    }
    else
      line_length = int(ur_y - ll_y);
  }
  if ( line_length < 3)
    line_length = 3;


  ((GrowCtx *)ctx)->export_jbean->axis( ll_x, ll_y, ur_x, ur_y,
     draw_type, text_color_drawtype, min_value, max_value, lines, longquotient, valuequotient,
     line_length, line_width, rotation, bold, idx, format,
     pass, shape_cnt, node_cnt, fp);
}

void GrowAxis::set_conf( double max_val, double min_val, int no_of_lines, 
     int long_quot, int value_quot, double rot, char *value_format)
{
  erase( &ctx->mw);
  max_value = max_val;
  min_value = min_val;
  lines = no_of_lines;
  longquotient = long_quot;
  valuequotient = value_quot;
  trf.rotation = rot;
  if ( format)
    strcpy( format, value_format);

  configure();
  draw();
}

void GrowAxis::set_axis_info( glow_sAxisInfo *info)
{
  max_value = info->max_value;
  min_value = info->min_value;
  lines = info->lines;
  longquotient = info->longquotient;
  valuequotient = info->valuequotient;
  strcpy( format, info->format);
}

void GrowAxis::format_text( char *text, char *fmt, double value) 
{
  if ( strcmp( fmt, "%1t") == 0) {
    // Hours, minutes and seconds, value in seconds
    int val = (int) nearbyint(value);
    int hours = val / 3600;
    int minutes = (val - hours * 3600) / 60; 
    int seconds = (val - hours * 3600 - minutes * 60); 
    sprintf( text, "%d:%02d:%02d", hours, minutes, seconds);
  }
  else if ( strcmp( fmt, "%2t") == 0) {
    // Hours and minutes, value in seconds
    int val = (int) nearbyint(value);
    int hours = val / 3600;
    int minutes = (val - hours * 3600) / 60; 
    sprintf( text, "%d:%02d", hours, minutes);
  }
  else if ( strcmp( fmt, "%3t") == 0) {
    // Days, hours and minues, value in seconds
    int val = (int) nearbyint(value);
    int days = val / (24 * 3600);
    int hours = (val - days * 24 * 3600) / 3600; 
    int minutes = (val - days * 24 * 3600 - hours * 3600) / 60; 
    sprintf( text, "%d %02d:%02d", days, hours, minutes);
  }
  else if ( strcmp( fmt, "%10t") == 0) {
    // Date
    char timstr[40];
    pwr_tTime t;
    t.tv_sec = (int) nearbyint(value);
    t.tv_nsec = 0;
    
    time_AtoAscii( &t, time_eFormat_NumDateAndTime, timstr, sizeof(timstr));
    timstr[19] = 0;
    strcpy( text, timstr);
  }
  else if ( strcmp( fmt, "%11t") == 0) {
    // Date, no seconds
    char timstr[40];
    pwr_tTime t;
    t.tv_sec = (int) nearbyint(value);
    t.tv_nsec = 0;
    
    time_AtoAscii( &t, time_eFormat_NumDateAndTime, timstr, sizeof(timstr));
    timstr[16] = 0;
    strcpy( text, timstr);
  }
  else
    sprintf( text, fmt, value);
}

void GrowAxis::convert( glow_eConvert version) 
{
  switch ( version) {
  case glow_eConvert_V34: {
    // Conversion of colors
    GrowRect::convert( version);
    text_drawtype = GlowColor::convert( version, text_drawtype);

    break;
  }
  }  
}












