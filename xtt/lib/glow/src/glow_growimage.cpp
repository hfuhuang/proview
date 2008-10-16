/* 
 * Proview   $Id: glow_growimage.cpp,v 1.8 2008-10-16 08:58:11 claes Exp $
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
#if defined OS_LINUX || defined OS_LYNX
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include "glow_transform.h"
#include "glow_growimage.h"
#include "glow_grownode.h"
#include "glow_growctx.h"
#include "glow_draw.h"


#if defined IMLIB
static int rgb_tone( unsigned char *x0, unsigned char *y0, unsigned char *z0, int tone);
static int rgb_shift( unsigned char *x0, unsigned char *y0, unsigned char *z0, int shift);
#endif

extern "C" {
#include "co_dcli.h"
}
 
GrowImage::GrowImage( GrowCtx *glow_ctx, char *name, double x, double y, 
	        char *imagefile,
		glow_mDisplayLevel display_lev) : 
                ll(glow_ctx,x,y), ur(glow_ctx,x+1,y+1),
    		hot(0), pzero(glow_ctx), stored_pos(glow_ctx), 
                highlight(0), inverse(0), user_data(NULL),
		dynamic(0), dynamicsize(0), image(0), original_image(0), 
                pixmap(0), nav_pixmap(0), clip_mask(0), nav_clip_mask(0),
                ctx(glow_ctx), display_level(display_lev), 
		color_tone(glow_eDrawTone_No), color_lightness(0),
		color_intensity(0), color_shift(0), color_inverse(0),
		current_color_tone(glow_eDrawTone_No), current_color_lightness(0),
		current_color_intensity(0), current_color_shift(0), 
		current_color_inverse(0), current_direction(0),
		current_nav_color_tone(glow_eDrawTone_No), current_nav_color_lightness(0),
		current_nav_color_intensity(0), current_nav_color_shift(0), 
		current_nav_color_inverse(0), current_nav_direction(0), 
		flip_vertical(false), flip_horizontal(false),
		current_flip_vertical(false), current_flip_horizontal(false), 
		rotation(0), current_rotation(0)
{
  strcpy( n_name, name);
  strcpy( image_filename, "");
  strcpy( filename, "");
  strcpy( last_group, "");
  imlib = ((GlowDraw *)ctx->gdraw)->imlib;
  if ( imagefile)
    insert_image( imagefile);

  pzero.nav_zoom();

  if ( ctx->grid_on) {
    double x_grid, y_grid;

    ctx->find_grid( ll.x, ll.y, &x_grid, &y_grid);
    ll.posit( x_grid, y_grid);
//    ctx->find_grid( ur.x, ur.y, &x_grid, &y_grid);
    if ( !pixmap)
      ur.posit( ll.x + 1, ll.y + 1);
    else
      ur.posit( ll.x + double( current_width) / ctx->mw.zoom_factor_x,
	    ll.y + double( current_height) / ctx->mw.zoom_factor_y);
  }
  draw( &ctx->mw, (GlowTransform *)NULL, highlight, hot, NULL, NULL);
  get_node_borders();
}

void GrowImage::copy_from( const GrowImage& im)
{
  memcpy( this, &im, sizeof(im));
  image = 0;
  pixmap = 0;
  nav_pixmap = 0;
  clip_mask = 0;
  nav_clip_mask = 0;
  if ( strcmp( image_filename, "") != 0) 
    insert_image( image_filename);
}

GrowImage::~GrowImage()
{
  ctx->object_deleted( this);

  if ( !ctx->nodraw) {
    erase( &ctx->mw);
    erase( &ctx->navw);

    ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	       y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
	       x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	       y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
    ctx->draw( &ctx->navw, x_left * ctx->navw.zoom_factor_x - ctx->navw.offset_x - 1,
	       y_low * ctx->navw.zoom_factor_y - ctx->navw.offset_y - 1,
	       x_right * ctx->navw.zoom_factor_x - ctx->navw.offset_x + 1,
	       y_high * ctx->navw.zoom_factor_y - ctx->navw.offset_y + 1);
    if ( hot)
      ctx->gdraw->set_cursor( &ctx->mw, glow_eDrawCursor_Normal);
  }
#if defined IMLIB
  if ( original_image)
    ctx->gdraw->image_free( original_image);
  if ( image)
    ctx->gdraw->image_free( image);
  if ( pixmap)
    ctx->gdraw->pixmap_free( pixmap);
#endif
}

int GrowImage::insert_image( char *imagefile)
{
  int found = 0;
  char imagename[80];
  char *s;
  struct stat info;
  int sts;

  strcpy( image_filename, imagefile);

  // Find file
  if ( strncmp( image_filename, "jpwr/", 5) == 0) {
    if ( (s = strchr( &image_filename[5], '/')))
      strcpy( imagename, s+1);
    else
      strcpy( imagename, image_filename); 
  }
  else
    strcpy( imagename, image_filename);

  strcpy( filename, imagename);
  if ( check_file( filename))
    found = 1;

  if ( !found) {
    // Add some search path
    for ( int i = 0; i < ((GrowCtx *)ctx)->path_cnt; i++) {
      strcpy( filename, ((GrowCtx *)ctx)->path[i]);
      strcat( filename, imagename);
      dcli_translate_filename( filename, filename);
      if ( check_file( filename)) {
        found = 1;
        break;
      }
    }
    if ( !found)
      return 0;
  }

  sts = stat( filename, &info);
  if ( sts == -1)
    return 0;

  date = info.st_ctime;

  ctx->gdraw->image_load( filename, &original_image, &image);
  if ( !original_image) 
    return 0;
  current_width = int( ctx->mw.zoom_factor_x / ctx->mw.base_zoom_factor *
		       ctx->gdraw->image_get_width( image));
  current_height = int( ctx->mw.zoom_factor_y / ctx->mw.base_zoom_factor *
			ctx->gdraw->image_get_height( image));
  current_color_tone = color_tone;
  current_color_lightness = color_lightness;
  current_color_intensity = color_intensity;
  current_color_shift = color_shift;
  current_color_inverse = color_inverse;

  set_image_color( image, NULL);

  ctx->gdraw->image_scale( current_width, current_height,
			   original_image, &image, &pixmap, &clip_mask);
  ctx->gdraw->image_render( current_width, current_height,
			    original_image, &image, &pixmap, &clip_mask);

  ur.posit( ll.x + double( current_width) / ctx->mw.zoom_factor_x,
	    ll.y + double( current_height) / ctx->mw.zoom_factor_y);
  get_node_borders();
  return 1;
}

int GrowImage::update()
{
  struct stat info;
  int sts;

  sts = stat( filename, &info);
  if ( sts == -1)
    return 0;

  if ( date == info.st_ctime)
    return 0;
  date = info.st_ctime;

  ctx->gdraw->image_load( filename, &original_image, &image);
  if ( !original_image) 
    return 0;

  set_image_color( image, NULL);
  ctx->gdraw->image_scale( current_width, current_height,
			   original_image, &image, &pixmap, &clip_mask);
  ctx->gdraw->image_render( current_width, current_height,
			    original_image, &image, &pixmap, &clip_mask);

  draw();
  return 1;
}

void GrowImage::zoom()
{
  ll.zoom();
  ur.zoom();
}

void GrowImage::nav_zoom()
{
  ll.nav_zoom();
  ur.nav_zoom();
}

void GrowImage::traverse( int x, int y)
{
  ll.traverse( x, y);
  ur.traverse( x, y);
}

void GrowImage::move( double delta_x, double delta_y, int grid)
{
  ctx->set_defered_redraw();
  ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  if ( grid) {
    double x_grid, y_grid;

    /* Move to closest grid point */
    erase( &ctx->mw);
    erase( &ctx->navw);
    ctx->find_grid( x_left + delta_x / ctx->mw.zoom_factor_x,
	y_low + delta_y / ctx->mw.zoom_factor_y, &x_grid, &y_grid);
    trf.move( x_grid - x_left, y_grid - y_low);
    get_node_borders();
  }
  else {
    double dx, dy;

    erase( &ctx->mw);
    erase( &ctx->navw);
    dx = delta_x / ctx->mw.zoom_factor_x;
    dy = delta_y / ctx->mw.zoom_factor_y;
    trf.move( dx, dy);
    x_right += dx;
    x_left += dx;
    y_high += dy;
    y_low += dy;
  }
  ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->navw, x_left * ctx->navw.zoom_factor_x - ctx->navw.offset_x - 1,
	     y_low * ctx->navw.zoom_factor_y - ctx->navw.offset_y - 1,
  	     x_right * ctx->navw.zoom_factor_x - ctx->navw.offset_x + 1,
	     y_high * ctx->navw.zoom_factor_y - ctx->navw.offset_y + 1);
  ctx->redraw_defered();

}

void GrowImage::move_noerase( int delta_x, int delta_y, int grid)
{
  if ( grid) {
    double x_grid, y_grid;

    /* Move to closest grid point */
    ctx->find_grid( x_left + double( delta_x) / ctx->mw.zoom_factor_x,
	y_low + double( delta_y) / ctx->mw.zoom_factor_y, &x_grid, &y_grid);
    trf.move( x_grid - x_left, y_grid - y_low);
    get_node_borders();
  }
  else {
    double dx, dy;

    dx = double( delta_x) / ctx->mw.zoom_factor_x;
    dy = double( delta_y) / ctx->mw.zoom_factor_y;
    trf.move( dx, dy);
    x_right += dx;
    x_left += dx;
    y_high += dy;
    y_low += dy;
  }
  ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->navw, x_left * ctx->navw.zoom_factor_x - ctx->navw.offset_x - 1,
	     y_low * ctx->navw.zoom_factor_y - ctx->navw.offset_y - 1,
  	     x_right * ctx->navw.zoom_factor_x - ctx->navw.offset_x + 1,
	     y_high * ctx->navw.zoom_factor_y - ctx->navw.offset_y + 1);

}

int GrowImage::local_event_handler( glow_eEvent event, double x, double y)
{
  double ll_x, ur_x, ll_y, ur_y;

  ll_x = min( ll.x, ur.x);
  ur_x = max( ll.x, ur.x);
  ll_y = min( ll.y, ur.y);
  ur_y = max( ll.y, ur.y);

  if ( ll_x <= x && x <= ur_x &&
       ll_y <= y && y <= ur_y) {
//    cout << "Event handler: Hit in image" << endl;
    return 1;
  }  
  else
    return 0;
}

int GrowImage::event_handler( GlowWind *w, glow_eEvent event, double fx, double fy)
{
  double x, y;

  trf.reverse( fx, fy, &x, &y);
  return local_event_handler( event, x, y);
}

int GrowImage::event_handler( GlowWind *w, glow_eEvent event, int x, int y, double fx,
	double fy)
{
  int sts;

  double rx, ry;

  // Convert koordinates to local koordinates
  trf.reverse( fx, fy, &rx, &ry);

  sts = 0;
  if ( event == ctx->event_move_node)
  {
    sts = local_event_handler( event, rx, ry);
    if ( sts) {
      /* Register node for potential movement */
      ctx->move_insert( this);
    }
    return sts;
  }
  switch ( event) {
    case glow_eEvent_CursorMotion: {
      int redraw = 0;

      if ( ctx->hot_mode == glow_eHotMode_TraceAction)
        sts = 0;
      else if ( ctx->hot_found)
        sts = 0;
      else
      {
        sts = local_event_handler( event, rx, ry);
        if ( sts)
          ctx->hot_found = 1;
      }
      if ( sts && !hot  &&
	   !(ctx->node_movement_active || ctx->node_movement_paste_active)) {
        ctx->gdraw->set_cursor( w, glow_eDrawCursor_CrossHair);
        hot = 1;
        redraw = 1;
      }
      if ( !sts && hot) {
        if ( !ctx->hot_found)
          ctx->gdraw->set_cursor( w, glow_eDrawCursor_Normal);
        erase( w);
        redraw = 1;
        hot = 0;
      }
      if ( redraw) {
	ctx->draw( w, x_left * w->zoom_factor_x - w->offset_x - DRAW_MP,
	     y_low * w->zoom_factor_y - w->offset_y - DRAW_MP,
  	     x_right * w->zoom_factor_x - w->offset_x + DRAW_MP,
	     y_high * w->zoom_factor_y - w->offset_y + DRAW_MP);
//          ((GlowImage *)this)->draw( (void *)&pzero, highlight, hot, NULL);
      }
      break;
    }
    default:
      sts = local_event_handler( event, rx, ry);
  }
  if ( sts)
    ctx->register_callback_object( glow_eObjectType_Node, this);
  return sts;
}


void GrowImage::save( ofstream& fp, glow_eSaveMode mode) 
{ 
  char *s;

  fp << int(glow_eSave_GrowImage) << endl;
  fp << int(glow_eSave_GrowImage_n_name) << FSPACE << n_name << endl;
  fp << int(glow_eSave_GrowImage_image_filename) << FSPACE << image_filename << endl;
  fp << int(glow_eSave_GrowImage_x_right) << FSPACE << x_right << endl;
  fp << int(glow_eSave_GrowImage_x_left) << FSPACE << x_left << endl;
  fp << int(glow_eSave_GrowImage_y_high) << FSPACE << y_high << endl;
  fp << int(glow_eSave_GrowImage_y_low) << FSPACE << y_low << endl;
  fp << int(glow_eSave_GrowImage_dynamicsize) << FSPACE << dynamicsize << endl;
  fp << int(glow_eSave_GrowImage_dynamic) << endl;
  if( dynamic)
  {
    fp << "\"";
    for ( s  = dynamic; *s; s++)
    {
      if ( *s == '"')
        fp << "\\";
      fp << *s;
    }
    fp << "\"" << endl;
  }
  fp << int(glow_eSave_GrowImage_trf) << endl;
  trf.save( fp, mode);
  fp << int(glow_eSave_GrowImage_display_level) << FSPACE << int(display_level) << endl;
  fp << int(glow_eSave_GrowImage_ll) << endl;
  ll.save( fp, mode);
  fp << int(glow_eSave_GrowImage_ur) << endl;
  ur.save( fp, mode);
  fp << int(glow_eSave_GrowImage_color_tone) << FSPACE << int(color_tone) << endl;
  fp << int(glow_eSave_GrowImage_color_lightness) << FSPACE << color_lightness << endl;
  fp << int(glow_eSave_GrowImage_color_intensity) << FSPACE << color_intensity << endl;
  fp << int(glow_eSave_GrowImage_color_shift) << FSPACE << color_shift << endl;
  fp << int(glow_eSave_End) << endl;
}

void GrowImage::open( ifstream& fp)
{
  int		type;
  int 		end_found = 0;
  char		dummy[40];
  int		tmp;
  int		j;
  char		c;

  for (;;)
  {
    fp >> type;
    switch( type) {
      case glow_eSave_GrowImage: break;
      case glow_eSave_GrowImage_n_name:
        fp.get();
        fp.getline( n_name, sizeof(n_name));
        break;
      case glow_eSave_GrowImage_image_filename:
        fp.get();
        fp.getline( image_filename, sizeof(image_filename));
        break;
      case glow_eSave_GrowImage_x_right: fp >> x_right; break;
      case glow_eSave_GrowImage_x_left: fp >> x_left; break;
      case glow_eSave_GrowImage_y_high: fp >> y_high; break;
      case glow_eSave_GrowImage_y_low: fp >> y_low; break;
      case glow_eSave_GrowImage_dynamicsize: fp >> dynamicsize; break;
      case glow_eSave_GrowImage_dynamic:
        fp.getline( dummy, sizeof(dummy));
        if ( dynamicsize)
        {
          dynamic = (char *) calloc( 1, dynamicsize);
	  fp.get();
          for ( j = 0; j < dynamicsize; j++)
	  {
	    if ((c = fp.get()) == '"')
	    {
	      if ( dynamic[j-1] == '\\')
	        j--;
	      else
              {
	        dynamic[j] = 0;
                break;
              }
	    }
            dynamic[j] = c;
	  }
          fp.getline( dummy, sizeof(dummy));
        }
        break;
      case glow_eSave_GrowImage_trf: trf.open( fp); break;
      case glow_eSave_GrowImage_display_level: fp >> tmp; display_level = (glow_mDisplayLevel)tmp; break;
      case glow_eSave_GrowImage_ll: ll.open( fp); break;
      case glow_eSave_GrowImage_ur: ur.open( fp); break;
      case glow_eSave_GrowImage_color_tone: fp >> tmp; 
		color_tone = (glow_eDrawTone)tmp; break;
      case glow_eSave_GrowImage_color_lightness: fp >> color_lightness; break;
      case glow_eSave_GrowImage_color_intensity: fp >> color_intensity; break;
      case glow_eSave_GrowImage_color_shift: fp >> color_shift; break;
      case glow_eSave_End: end_found = 1; break;
      default:
        cout << "GrowImage:open syntax error" << endl;
        fp.getline( dummy, sizeof(dummy));
    }
    if ( end_found)
      break;
  }

  if ( strcmp( image_filename, "") != 0)
    insert_image( image_filename);
}

void GrowImage::draw( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y) 
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
       	y_low * w->zoom_factor_y - w->offset_y <= ur_y) {
    draw( w, (GlowTransform *)NULL, highlight, hot, NULL, NULL);
  }
}

void GrowImage::draw( GlowWind *w, int *ll_x, int *ll_y, int *ur_x, int *ur_y) 
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

void GrowImage::set_highlight( int on)
{
  erase( &ctx->mw);
  erase( &ctx->navw);

  highlight = on;
  draw();
}

void GrowImage::select_region_insert( double ll_x, double ll_y, double ur_x, 
		double ur_y, glow_eSelectPolicy select_policy)
{
  if ( select_policy == glow_eSelectPolicy_Surround ) {
    if ( x_left > ll_x && x_right < ur_x && y_high < ur_y && y_low > ll_y)
      ctx->select_insert( this);
  }
  else {
    if ( x_right > ll_x && x_left < ur_x && y_low < ur_y && y_high > ll_y)
      ctx->select_insert( this);
  }
}

void GrowImage::set_dynamic( char *code, int size)
{
  if ( !dynamic)
  {
    dynamic = (char *) calloc( 1, size+1);
    dynamicsize = size+1;
  }
  else if ( dynamicsize < size+1)
  {
    free( dynamic);
    dynamic = (char *) calloc( 1, size+1);
    dynamicsize = size+1;
  }
  strncpy( dynamic, code, size+1);
}

void GrowImage::exec_dynamic()
{
  if ( dynamicsize && strcmp( dynamic, "") != 0)
    ((GrowCtx *)ctx)->dynamic_cb( this, dynamic, glow_eDynamicType_Object);
}

void GrowImage::set_position( double x, double y)
{
  double old_x_left, old_x_right, old_y_low, old_y_high;

  if ( trf.a13 == x && trf.a23 == y)
     return;
  old_x_left = x_left;
  old_x_right = x_right;
  old_y_low = y_low;
  old_y_high = y_high;
  erase( &ctx->mw);
  erase( &ctx->navw);
  trf.posit( x, y);
  get_node_borders();
  ctx->draw( &ctx->mw, old_x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     old_y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     old_x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     old_y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->navw, x_left * ctx->navw.zoom_factor_x - ctx->navw.offset_x - 1,
	     y_low * ctx->navw.zoom_factor_y - ctx->navw.offset_y - 1,
  	     x_right * ctx->navw.zoom_factor_x - ctx->navw.offset_x + 1,
	     y_high * ctx->navw.zoom_factor_y - ctx->navw.offset_y + 1);

}

void GrowImage::set_scale( double scale_x, double scale_y, 
	double x0, double y0, glow_eScaleType type)
{
  double old_x_left, old_x_right, old_y_low, old_y_high;

  if ( trf.s_a11 && trf.s_a22 &&
       fabs( scale_x - trf.a11 / trf.s_a11) < FLT_EPSILON &&
       fabs( scale_y - trf.a22 / trf.s_a22) < FLT_EPSILON)
    return;

  switch( type) {
    case glow_eScaleType_LowerLeft:
      x0 = x_left;
      y0 = y_low;
      break;
    case glow_eScaleType_LowerRight:
      x0 = x_right;
      y0 = y_low;
      break;
    case glow_eScaleType_UpperRight:
      x0 = x_right;
      y0 = y_high;
      break;
    case glow_eScaleType_UpperLeft:
      x0 = x_left;
      y0 = y_high;
      break;
    case glow_eScaleType_FixPoint:
      break;
    case glow_eScaleType_Center:
      x0 = (x_left + x_right) / 2;
      y0 = (y_low + y_high) /2;
      break;
    default:
      ;
  }

  old_x_left = x_left;
  old_x_right = x_right;
  old_y_low = y_low;
  old_y_high = y_high;
  erase( &ctx->mw);
  erase( &ctx->navw);
  trf.scale_from_stored( scale_x, scale_y, x0, y0);
  get_node_borders();

  switch( type) {
    case glow_eScaleType_LowerLeft:
      x_left = old_x_left;
      y_low = old_y_low;
      break;
    case glow_eScaleType_LowerRight:
      x_right = old_x_right;
      y_low = old_y_low;
      break;
    case glow_eScaleType_UpperRight:
      x_right = old_x_right;
      y_high = old_y_high;
      break;
    case glow_eScaleType_UpperLeft:
      x_left = old_x_left;
      y_high = old_y_high;
      break;
    case glow_eScaleType_FixPoint:
      break;
    case glow_eScaleType_Center:
      x0 = (x_left + x_right) / 2;
      y0 = (y_low + y_high) /2;
      break;
    default:
      ;
  }
  ctx->draw( &ctx->mw, old_x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     old_y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     old_x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     old_y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->navw, x_left * ctx->navw.zoom_factor_x - ctx->navw.offset_x - 1,
	     y_low * ctx->navw.zoom_factor_y - ctx->navw.offset_y - 1,
  	     x_right * ctx->navw.zoom_factor_x - ctx->navw.offset_x + 1,
	     y_high * ctx->navw.zoom_factor_y - ctx->navw.offset_y + 1);
}

void GrowImage::set_rotation( double angle, 
	double x0, double y0, glow_eRotationPoint type)
{
  double old_x_left, old_x_right, old_y_low, old_y_high;

  if ( fabs( angle - trf.rotation + trf.s_rotation) < FLT_EPSILON)
    return;

  switch( type)
  {
    case glow_eRotationPoint_LowerLeft:
      x0 = x_left;
      y0 = y_low;
      break;
    case glow_eRotationPoint_LowerRight:
      x0 = x_right;
      y0 = y_low;
      break;
    case glow_eRotationPoint_UpperRight:
      x0 = x_right;
      y0 = y_high;
      break;
    case glow_eRotationPoint_UpperLeft:
      x0 = x_left;
      y0 = y_high;
      break;
    case glow_eRotationPoint_Center:
      x0 = (x_left + x_right) / 2;
      y0 = (y_high + y_low) / 2;
      break;
    default:
      ;
  }

  old_x_left = x_left;
  old_x_right = x_right;
  old_y_low = y_low;
  old_y_high = y_high;
  erase( &ctx->mw);
  erase( &ctx->navw);
  trf.rotate_from_stored( angle, x0, y0);
  get_node_borders();
  ctx->draw( &ctx->mw, old_x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     old_y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     old_x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     old_y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->mw, x_left * ctx->mw.zoom_factor_x - ctx->mw.offset_x - DRAW_MP,
	     y_low * ctx->mw.zoom_factor_y - ctx->mw.offset_y - DRAW_MP,
  	     x_right * ctx->mw.zoom_factor_x - ctx->mw.offset_x + DRAW_MP,
	     y_high * ctx->mw.zoom_factor_y - ctx->mw.offset_y + DRAW_MP);
  ctx->draw( &ctx->navw, x_left * ctx->navw.zoom_factor_x - ctx->navw.offset_x - 1,
	     y_low * ctx->navw.zoom_factor_y - ctx->navw.offset_y - 1,
  	     x_right * ctx->navw.zoom_factor_x - ctx->navw.offset_x + 1,
	     y_high * ctx->navw.zoom_factor_y - ctx->navw.offset_y + 1);
}

void GrowImage::draw( GlowWind *w, GlowTransform *t, int highlight, int hot, void *node,
		      void *colornode)
{
  if ( !(display_level & ctx->display_level))
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

  if ( ll_x == ur_x || ll_y == ur_y)
    return;

  double rot;

  if ( t)
    rot = (trf.rot( t) / 360 - floor( trf.rot( t) / 360)) * 360;
  else
    rot = (trf.rot() / 360 - floor( trf.rot() / 360)) * 360;

  rotation = int((rot + 45) / 90) * 90;

  if ( w == &ctx->navw) {
    ctx->gdraw->fill_rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, 
			   glow_eDrawType_LineGray);
  }
  else {
    if ( pixmap || image) {
      int sts = 0;
      int flip_vert, flip_horiz;
      glow_tImImage om = original_image;
      glow_tImImage old_image = image;

      if ( colornode) {
	flip_vert = (( ((GrowNode *)node)->flip_vertical && !flip_vertical) ||
		     ( !((GrowNode *)node)->flip_vertical && flip_vertical));
	flip_horiz = (( ((GrowNode *)node)->flip_horizontal && !flip_horizontal) ||
		     ( !((GrowNode *)node)->flip_horizontal && flip_horizontal));
      }
      else {
	flip_vert = flip_vertical;
	flip_horiz = flip_horizontal;
      }

      if ( ur_x - ll_x != current_width || ur_y - ll_y != current_height) {
	ctx->gdraw->image_scale( ur_x - ll_x, ur_y - ll_y, om,
				 &image, &pixmap, &clip_mask);
	current_width = ctx->gdraw->image_get_width( image);
	current_height = ctx->gdraw->image_get_height( image);
	sts = 1;
	om = 0;
	if ( rotation != current_rotation)
	  current_rotation = 0;
      }	

      if ( rotation != current_rotation) {
	ctx->gdraw->image_rotate( &image, rotation, current_rotation);
	current_rotation = rotation;
	om = 0;
	sts = 1;
      }

      if ( (colornode && !(current_color_tone == ((GrowNode *)node)->color_tone &&
			   current_color_lightness == ((GrowNode *)node)->color_lightness &&
			   current_color_intensity == ((GrowNode *)node)->color_intensity &&
			   current_color_shift == ((GrowNode *)node)->color_shift &&
			   current_color_inverse == ((GrowNode *)node)->color_inverse)) ||  
	   ( !colornode && !(current_color_tone == color_tone &&
			     current_color_lightness == color_lightness &&
			     current_color_intensity == color_intensity &&
			     current_color_shift == color_shift &&
			     current_color_inverse == color_inverse)) ||
	   ( image != old_image &&
	     ((colornode && (glow_eDrawTone_No != ((GrowNode *)node)->color_tone ||
			     ((GrowNode *)node)->color_lightness ||
			     ((GrowNode *)node)->color_intensity ||
			     ((GrowNode *)node)->color_shift ||
			     ((GrowNode *)node)->color_inverse)) ||  
	      ( !colornode && (glow_eDrawTone_No == color_tone ||
			     color_lightness ||
			     color_intensity ||
			     color_shift ||
			     color_inverse))))) {
	set_image_color( om, colornode);
	if ( ctx->gdraw->image_get_width( image) != current_width ||
	     ctx->gdraw->image_get_height( image) != current_height) {
	  ctx->gdraw->image_scale( ur_x - ll_x, ur_y - ll_y, 0,
				   &image, &pixmap, &clip_mask);
	  current_width = ctx->gdraw->image_get_width( image);
	  current_height = ctx->gdraw->image_get_height( image);
	}
	om = 0;
	sts = 1;
      }
      
      if ( flip_vert != current_flip_vertical) {
	ctx->gdraw->image_flip_vertical( &image);
	current_flip_vertical = flip_vert;
	sts = 1;
      }
      if ( flip_horiz != current_flip_horizontal) {
	ctx->gdraw->image_flip_horizontal( &image);
	current_flip_horizontal = flip_horiz;
	sts = 1;
      }

      if ( sts) {
	ctx->gdraw->image_render( ur_x - ll_x, ur_y - ll_y, om,
				  &image, &pixmap, &clip_mask);
	om = 0;
	current_width = ctx->gdraw->image_get_width( image);
	current_height = ctx->gdraw->image_get_height( image);
	if ( colornode) {
	  current_color_tone = ((GrowNode *)colornode)->color_tone;
	  current_color_lightness = ((GrowNode *)colornode)->color_lightness;
	  current_color_intensity = ((GrowNode *)colornode)->color_intensity;
	  current_color_shift = ((GrowNode *)colornode)->color_shift;
	  current_color_inverse = ((GrowNode *)colornode)->color_inverse;
	}
	else {
	  current_color_tone = color_tone;
	  current_color_lightness = color_lightness;
	  current_color_intensity = color_intensity;
	  current_color_shift = color_shift;
	  current_color_inverse = color_inverse;
	}
      }
      
      ctx->gdraw->image( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, 
			 image, pixmap, clip_mask);
    }
    else
      ctx->gdraw->fill_rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, 
			     glow_eDrawType_LineGray);
    if ( highlight) {
      ctx->gdraw->rect( w, ll_x, ll_y, ur_x - ll_x - 1, ur_y - ll_y - 1, 
			glow_eDrawType_LineRed, 0, 0);
    }
    else if ( hot /*  && !((GrowCtx *)ctx)->enable_bg_pixmap */)
      ctx->gdraw->rect( w, ll_x, ll_y, ur_x - ll_x - 1, ur_y - ll_y - 1, 
			glow_eDrawType_LineGray, 0, 0);
  }
}

void GrowImage::erase( GlowWind *w, GlowTransform *t, int hot, void *node)
{
  if ( !(display_level & ctx->display_level))
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
  if ( ctx->enable_bg_pixmap)
    ctx->gdraw->draw_background( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y);
  else
    ctx->gdraw->fill_rect( w, ll_x, ll_y, ur_x - ll_x, ur_y - ll_y, glow_eDrawType_LineErase);
  w->reset_draw_buffer_only();
}

void GrowImage::draw()
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

void GrowImage::get_borders( GlowTransform *t, double *x_right, 
	double *x_left, double *y_high, double *y_low)
{
  double ll_x, ur_x, ll_y, ur_y, x1, x2, y1, y2;

  if ( t)
  {
    x1 = trf.x( t, ll.x, ll.y);
    x2 = trf.x( t, ur.x, ur.y);
    y1 = trf.y( t, ll.x, ll.y);
    y2 = trf.y( t, ur.x, ur.y);
  }
  else
  {
    x1 = trf.x( ll.x, ll.y);
    x2 = trf.x( ur.x, ur.y);
    y1 = trf.y( ll.x, ll.y);
    y2 = trf.y( ur.x, ur.y);
  }

  ll_x = min( x1, x2);
  ur_x = max( x1, x2);
  ll_y = min( y1, y2);
  ur_y = max( y1, y2);

  if ( display_level != glow_mDisplayLevel_1)
    return;
  if ( ll_x < *x_left)
    *x_left = ll_x;
  if ( ur_x > *x_right)
    *x_right = ur_x;
  if ( ll_y < *y_low)
    *y_low = ll_y;
  if ( ur_y > *y_high)
    *y_high = ur_y;
}

void GrowImage::set_transform( GlowTransform *t)
{
  trf = *t * trf;
  get_node_borders();
}


void GrowImage::align( double x, double y, glow_eAlignDirection direction)
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


void GrowImage::export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp)
{
  if ( !(display_level & ctx->display_level))
    return;
  double x1, y1, x2, y2, ll_x, ll_y, ur_x, ur_y;
  double rot;
  int transparent = 0;

  if (!t) {
    x1 = trf.x( ll.x, ll.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y1 = trf.y( ll.x, ll.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
    x2 = trf.x( ur.x, ur.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y2 = trf.y( ur.x, ur.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
    rot = trf.rot();
  }
  else {
    x1 = trf.x( t, ll.x, ll.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y1 = trf.y( t, ll.x, ll.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
    x2 = trf.x( t, ur.x, ur.y) * ctx->mw.zoom_factor_x - ctx->mw.offset_x;
    y2 = trf.y( t, ur.x, ur.y) * ctx->mw.zoom_factor_y - ctx->mw.offset_y;
    rot = trf.rot(t);
  }

  ll_x = min( x1, x2);
  ur_x = max( x1, x2);
  ll_y = min( y1, y2);
  ur_y = max( y1, y2);
  if ( clip_mask)
    transparent = 1;

  ctx->export_jbean->image( ll_x, ll_y, ur_x, ur_y, image_filename,
    	transparent, color_tone, color_lightness,
    	color_intensity, color_shift, rot,
    	pass, shape_cnt, node_cnt, in_nc, fp);
}

int GrowImage::set_image_color( glow_tImImage om, void *n)
{
  GrowNode *node = (GrowNode *) n;

  if ( node) {
    c_color_lightness = node->color_lightness;
    c_color_tone = node->color_tone;
    c_color_inverse = node->color_inverse;
    c_color_shift = node->color_shift;
    c_color_intensity = node->color_intensity;
  }
  else {
    c_color_lightness = color_lightness;
    c_color_tone = color_tone;
    c_color_inverse = color_inverse;
    c_color_shift = color_shift;
    c_color_intensity = color_intensity;
  }


  if ( c_color_intensity > 0)
    factor_intens = 1 + 0.1 * c_color_intensity;
  else
    factor_intens = 1 + 0.1 * c_color_intensity;

  if ( c_color_lightness > 0)
    factor_light = 1 - 0.1 * c_color_lightness;
  else
    factor_light = 1 + 0.1 * c_color_lightness;

  if ( !(c_color_tone == glow_eDrawTone_No || c_color_tone >= glow_eDrawTone__) ||
       c_color_shift || c_color_intensity || c_color_lightness || c_color_inverse) {
    ctx->gdraw->image_pixel_iter( om, &image, pixel_cb, this);
  }
  return 1;
}

void GrowImage::pixel_cb( void *data, unsigned char *rgb)
{
  GrowImage *o = (GrowImage *)data;
  int           m;
  int           value;

  if ( !(o->c_color_tone == glow_eDrawTone_No || o->c_color_tone >= glow_eDrawTone__)) {
    rgb_tone( rgb, rgb+1, rgb+2, o->c_color_tone);
  }
  if ( o->c_color_shift) {
    rgb_shift( rgb, rgb+1, rgb+2, o->c_color_shift);
  }

  for ( int i = 0; i < 3; i++) {
    if ( o->c_color_intensity) {
      value = int( o->factor_intens * *rgb) - o->c_color_intensity * 25;
      if ( value > 255)
	*rgb = 255;
      else if ( value < 0)
	*rgb = 0;
      else
	*rgb = value;
    }

    if ( o->c_color_lightness) {
      if ( o->c_color_lightness > 0) {
	value = int( o->factor_light * *rgb) + o->c_color_lightness * 25;
	if ( value < 0)
	  *rgb = 0;
	else
	  *rgb = value;
      }
      else {
	value = int( o->factor_light * *rgb);
	if ( value > 255)
	  *rgb = 255;
	else
	  *rgb = value;
      }
    }
	
    if ( o->c_color_inverse) {
      //        *rgb = 255 - *rgb;
      if ( i % 3 == 0) {
	m = ((int)(*rgb) + *(rgb+1) + *(rgb+2)) / 3;
	value = 255 - m + ((int)*rgb - m);
	if ( value < 0)
	  value = 0;
	if ( value > 255)
	  value = 255;
	*rgb = (unsigned char) value;
	value = 255 - m + ((int)*(rgb+1) - m);
	if ( value < 0)
	  value = 0;
	if ( value > 255)
	  value = 255;
	*(rgb+1) = (unsigned char) value;
	value = 255 - m + ((int)*(rgb+2) - m);
	if ( value < 0)
	  value = 0;
	if ( value > 255)
	  value = 255;
	*(rgb+2) = (unsigned char) value;
      }
    }
    rgb++;
  }
}


#if defined IMLIB
static int rgb_tone( unsigned char *x0, unsigned char *y0, unsigned char *z0, int tone)
{
  int a1, b2;
  int tmp, m;
  float ka1, kb1;
  int ka2, kb2;

  m = ((int)*x0 + *y0 + *z0) / 3;

  ka1 = 0.6; /* 0.6 */
  ka2 = 120; /* 130 */
  kb1 = 0.4;
  kb2 = 0;  
  if ( m > 75)
    a1 = (int)(m * ka1) + ka2;
  else
    a1 = (int)(m * 1.5);
  if ( a1 > 255)
    a1 = 255;
  b2 = int(kb1 * m) + kb2;
  if ( b2 < 0)
    b2 = 0;


  switch ( tone) {
    case glow_eDrawTone_Gray:
      *x0 = m;
      *y0 = m;
      *z0 = m;
      break;      
    case glow_eDrawTone_YellowGreen:
      *x0 = a1;
      *y0 = a1;
      *z0 = b2;
       break;
    case glow_eDrawTone_Yellow:
      tmp = int( 1.2 * a1);
      if  (tmp <= 255)
        *x0 = tmp;
      else
        *x0 = 255;
      *y0 = (unsigned char)(0.9 * a1);
      *z0 = b2;
       break;
    case glow_eDrawTone_Orange:
      tmp = (int)(1.2 * a1);
      if  (tmp <= 255)
        *x0 = (unsigned char)tmp;
      else
        *x0 = 255;
      *y0 = (unsigned char)(0.6 * a1);
      *z0 = b2;
       break;
    case glow_eDrawTone_Red:
      *x0 = a1;
      *y0 = b2;
      *z0 = b2;
       break;
    case glow_eDrawTone_Magenta:
      *x0 = a1;
      *y0 = b2;
      *z0 = a1;
       break;
    case glow_eDrawTone_Blue:
      *x0 = b2;
      *y0 = b2;
      *z0 = a1;
       break;
    case glow_eDrawTone_Seablue:
      *x0 = b2;
      *y0 = a1;
      *z0 = a1;
       break;
    case glow_eDrawTone_Green:
      *x0 = b2;
      *y0 = a1;
      *z0 = b2;
       break;
    case glow_eDrawTone_DarkGray:
      *x0 = (unsigned char)(m / 1.3);
      *y0 = (unsigned char)(m / 1.3);
      *z0 = (unsigned char)(m / 1.3);
       break;
  }
  return 1;
}
#endif

#if defined IMLIB
static int rgb_shift( unsigned char *x0, unsigned char *y0, unsigned char *z0, int shift)
{
  unsigned char x, y, z;
  int d;
  int step;

  shift = -shift;
  shift %= 8;
  if ( shift < 0)
    shift += 8;

  x = *x0;
  y = *y0;
  z = *z0;

  for( ;;) {
    if ( x == y && y == z)
      break;
    if ( x > y && y >= z && x > z) {
      d = x - z;
      step = 6 * d / 10 * shift;
//      printf("Section 1, d: %d, step: %d, lap: %d\n", d, step, 6 * d);

      if ( step <= z + d - y) {
        y += step;
        break;
      }
      step -= z + d - y;
      y = z + d;
      if ( step <= d) {
        x -= step;
	break;
      }
      x -= d;
      step -= d;
      if ( step <= d) {
        z += step; 
	break;
      }
      z += d;
      step -= d;
      if ( step <= d) {
        y -= step; 
	break;
      }
      y -= d;
      step -= d;
      if ( step <= d) {
        x += step;
	break;
      }
      x += d;
      step -= d;
      if ( step <= d) {
        z -= step;
	break;
      }
      z -= d;
      step -= d;
      if ( step <= d) {
        y += step;
	break;
      }
      y += d;
      printf( "Error, shift larger than one lap\n");
      break;
    }
    else if ( x > z && y >= x && y > z) {
      d = y - z;
      step = 6 * d / 10 * shift;
//      printf("Section 2, d: %d, step: %d\n", d, step);

      if ( step <= x - z) {
        x -= step;
        break;
      }
      step -= x - z;
      x = z;
      if ( step <= d) {
        z += step;
	break;
      }
      z += d;
      step -= d;
      if ( step <= d) {
        y -= step; 
	break;
      }
      y -= d;
      step -= d;
      if ( step <= d) {
        x += step; 
	break;
      }
      x += d;
      step -= d;
      if ( step <= d) {
        z -= step;
	break;
      }
      z -= d;
      step -= d;
      if ( step <= d) {
        y += step;
	break;
      }
      y += d;
      step -= d;
      if ( step <= d) {
        x -= step;
	break;
      }
      x -= d;
      printf( "Error, shift larger than one lap\n");
      break;
    }
    else if ( y > z && z >= x && y > x) {
      d = y - x;
      step = 6 * d / 10 * shift;
//      printf("Section 3, d: %d, step: %d\n", d, step);

      if ( step <= x + d - z) {
        z += step;
        break;
      }
      step -= x + d - z;
      z = x  + d;
      if ( step <= d) {
        y -= step;
	break;
      }
      y -= d;
      step -= d;
      if ( step <= d) {
        x += step; 
	break;
      }
      x += d;
      step -= d;
      if ( step <= d) {
        z -= step; 
	break;
      }
      z -= d;
      step -= d;
      if ( step <= d) {
        y += step;
	break;
      }
      y += d;
      step -= d;
      if ( step <= d) {
        x -= step;
	break;
      }
      x -= d;
      step -= d;
      if ( step <= d) {
        z += step;
	break;
      }
      z += d;
      printf( "Error, shift larger than one lap\n");
      break;
    }
    else if ( z >= y && y > x && z >= y) {
      d = z - x;
      step = 6 * d / 10 * shift;
//      printf("Section 4, d: %d, step: %d\n", d, step);

      if ( step <= y - x) {
        y -= step;
	break;
      }
      step -= y - x;
      y = x;
      if ( step <= d) {
        x += step;
	break;
      }
      x += d;
      step -= d;
      if ( step <= d) {
        z -= step; 
	break;
      }
      z -= d;
      step -= d;
      if ( step <= d) {
        y += step; 
	break;
      }
      y += d;
      step -= d;
      if ( step <= d) {
        x -= step;
	break;
      }
      x -= d;
      step -= d;
      if ( step <= d) {
        z += step;
	break;
      }
      z += d;
      step -= d;
      if ( step <= d) {
        y -= step;
	break;
      }
      y -= d;
      printf( "Error, shift larger than one lap\n");
      break;
    }
    else if ( z > x && x >= y && z > y) {
      d = z - y;
      step = 6 * d / 10 * shift;
//      printf("Section 5, d: %d, step: %d\n", d, step);

      if ( step <= y + d - x) {
        x += step;
        break;
      }
      step -= y + d - x;
      x = y + d;
      if ( step <= d) {
        z -= step;
	break;
      }
      z -= d;
      step -= d;
      if ( step <= d) {
        y += step; 
	break;
      }
      y += d;
      step -= d;
      if ( step <= d) {
        x -= step; 
	break;
      }
      x -= d;
      step -= d;
      if ( step <= d) {
        z += step;
	break;
      }
      z += d;
      step -= d;
      if ( step <= d) {
        y -= step;
	break;
      }
      y -= d;
      step -= d;
      if ( step <= d) {
        x += step;
	break;
      }
      x += d;
      printf( "Error, shift larger than one lap\n");
      break;
    }
    else /* if ( x >= z && z > y && x > y) */ {
      d = x - y;
      step = 6 * d / 8 * shift;
//      printf("Section 6, d: %d, step: %d\n", d, step);
      if ( d < 0)
        printf( "d: %d ( %d, %d, %d)\n", d, x, y, z);

      if ( step <= z - y) {
        z -= step;
        break;
      }
      step -= z - y;
      z = y;
      if ( step <= d) {
        y += step;
	break;
      }
      y += d;
      step -= d;
      if ( step <= d) {
        x -= step; 
	break;
      }
      x -= d;
      step -= d;
      if ( step <= d) {
        z += step; 
	break;
      }
      z += d;
      step -= d;
      if ( step <= d) {
        y -= step;
	break;
      }
      y -= d;
      step -= d;
      if ( step <= d) {
        x += step;
	break;
      }
      x += d;
      step -= d;
      if ( step <= d) {
        z -= step;
	break;
      }
      z -= d;
      printf( "Error, shift larger than one lap\n");
      break;
    }
   
  }
  *x0 = x;
  *y0 = y;
  *z0 = z;
  return 1;
}
#endif

int grow_image_to_pixmap( GrowCtx *ctx, char *imagefile, 
	    int width, int height, glow_tPixmap *pixmap, glow_tImImage *image, int *w, int *h)
{
  int found = 0;
  char imagename[80];
  pwr_tFileName filename;
    char *s;

  // Find file
  if ( strncmp( imagefile, "jpwr/", 5) == 0) {
    if ( (s = strchr( &imagefile[5], '/')))
      strcpy( imagename, s+1);
    else
      strcpy( imagename, imagefile); 
  }
  else
    strcpy( imagename, imagefile);

  strcpy( filename, imagename);
  if ( check_file( filename))
    found = 1;

  if ( !found) {
    // Add some search path
    for ( int i = 0; i < ((GrowCtx *)ctx)->path_cnt; i++) {

      strcpy( filename, ((GrowCtx *)ctx)->path[i]);
      strcat( filename, imagename);
      dcli_translate_filename( filename, filename);
      if ( check_file( filename)) {
        found = 1;
        break;
      }
    }
    if ( !found)
      return 0;
  }

  ctx->gdraw->image_load( filename, image, 0);
  if ( !*image)
    return 0;

  if ( width == 0 || height == 0) {
    width = ctx->gdraw->image_get_width( *image);
    height = ctx->gdraw->image_get_height( *image);
  }
  else {
    ctx->gdraw->image_scale( width, height,
			     0, image, pixmap, 0);
  }
  ctx->gdraw->image_render( width, height,
			    0, image, pixmap, 0);
  *w = width;
  *h = height;

  return 1;
}

void GrowImage::flip( double x0, double y0, glow_eFlipDirection dir)
{
  switch ( dir) {
  case glow_eFlipDirection_Horizontal:
    trf.store();
    set_scale( 1, -1, x0, y0, glow_eScaleType_FixPoint);
    flip_horizontal = !flip_horizontal;
    break;
  case glow_eFlipDirection_Vertical:
    trf.store();
    set_scale( -1, 1, x0, y0, glow_eScaleType_FixPoint);
    flip_vertical = !flip_vertical;
    break;
  }
}
