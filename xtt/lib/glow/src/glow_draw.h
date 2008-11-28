/* 
 * Proview   $Id: glow_draw.h,v 1.13 2008-11-28 17:13:45 claes Exp $
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

#ifndef glow_draw_h
#define glow_draw_h

#include <stdlib.h>

#if defined OS_LINUX
#define IMLIB 1
#endif

#define DRAW_CLIP_SIZE 10

class GlowCtx;

class GlowDraw {
 public:
  GlowCtx *ctx;
  glow_tImData imlib;

  GlowDraw(): ctx(0), imlib(0) {}
  virtual ~GlowDraw() {};

  // virtual int event_handler( XEvent event) { return 1;}
  virtual void enable_event( glow_eEvent event, 
			     glow_eEventType event_type, 
			      int (*event_cb)(GlowCtx *ctx, glow_tEvent event)) {}
  virtual void clear( GlowWind *w) {}
  virtual void copy_buffer( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y) {}

  virtual void get_window_size( GlowWind *w, int *width, int *height) {}
  virtual void set_window_size( GlowWind *w, int width, int height) {}

  virtual int rect( GlowWind *w, int x, int y, int width, int height, 
	    glow_eDrawType gc_type, int idx, int highlight) {return 1;}
  virtual int rect_erase( GlowWind *w, int x, int y, int width, int height,
		  int idx) {return 1;}
  virtual int arrow( GlowWind *w, int x1, int y1, int x2, int y2, 
	     int x3, int y3,
	     glow_eDrawType gc_type, int idx, int highlight) {return 1;}
  virtual int arrow_erase( GlowWind *w, int x1, int y1, int x2, int y2, 
		   int x3, int y3,
		   int idx) {return 1;}
  virtual int arc( GlowWind *w, int x, int y, int width, int height, 
	   int angle1, int angle2,
	   glow_eDrawType gc_type, int idx, int highlight) {return 1;}
  virtual int fill_arc( GlowWind *w, int x, int y, int width, int height, 
		int angle1, int angle2, glow_eDrawType gc_type, int highlight) {return 1;}
  virtual int arc_erase( GlowWind *w, int x, int y, int width, int height,
		 int angle1, int angle2,
		 int idx) {return 1;}
  virtual int line( GlowWind *w, int x1, int y1, int x2, int y2,
	    glow_eDrawType gc_type, int idx, int highlight) {return 1;}
  virtual int line_dashed( GlowWind *w, int x1, int y1, int x2, int y2,
		   glow_eDrawType gc_type, int idx, int highlight, glow_eLineType line_type) {return 1;}
  virtual int line_erase( GlowWind *w, int x1, int y1, int x2, int y2,
		  int idx) {return 1;}
  virtual int polyline( GlowWind *w, glow_sPointX *points, int point_cnt,
		glow_eDrawType gc_type, int idx, int highlight) {return 1;}
  virtual int fill_polyline( GlowWind *w, glow_sPointX *points, int point_cnt,
		     glow_eDrawType gc_type, int highlight) {return 1;}
  virtual int polyline_erase( GlowWind *w, glow_sPointX *points, int point_cnt,
		      int idx) {return 1;}
  virtual int text( GlowWind *w, int x, int y, char *text, int len,
	    glow_eDrawType gc_type, glow_eDrawType color, int idx, int highlight, 
		    int line, glow_eFont font_idx, double size) {return 1;}
  virtual int text_cursor( GlowWind *w, int x, int y, char *text, int len,
			   glow_eDrawType gc_type, glow_eDrawType color, int idx, 
			   int highlight, int pos, glow_eFont font, double size) {return 1;}
  virtual int text_erase( GlowWind *w, int x, int y, char *text, int len,
			  glow_eDrawType gc_type, int idx, int line, glow_eFont font_idx,
			  double size) {return 1;}
  virtual int fill_rect( GlowWind *w, int x, int y, int width, int height, 
		 glow_eDrawType gc_type) {return 1;}
  virtual int pixmaps_create( GlowWind *w, glow_sPixmapData *pixmap_data,
		      void **pixmaps) {return 1;}
  virtual void pixmaps_delete( GlowWind *w, void *pixmaps) {}
  virtual int pixmap( GlowWind *w, int x, int y, glow_sPixmapData *pixmap_data,
	      void *pixmaps, glow_eDrawType gc_type, int idx, int highlight, int line) {return 1;}
  virtual int pixmap_inverse( GlowWind *w, int x, int y, glow_sPixmapData *pixmap_data,
		      void *pixmaps, glow_eDrawType gc_type, int idx, int line) {return 1;}
  virtual int pixmap_erase( GlowWind *w, int x, int y, glow_sPixmapData *pixmap_data,
		    void *pixmaps, glow_eDrawType gc_type, int idx, int line) {return 1;}
  virtual int image( GlowWind *w, int x, int y, int width, int height,
	     glow_tImImage image, glow_tPixmap pixmap, glow_tPixmap clip_mask) {return 1;}
  
  virtual void set_cursor( GlowWind *w, glow_eDrawCursor cursor) {}
  virtual void set_nav_cursor( glow_eDrawCursor cursor) {}
  virtual int get_text_extent( const char *text, int len,
			       glow_eDrawType gc_type, int idx, glow_eFont font_idx,
			       int *width, int *height, int *descent, double size) {return 1;}
  virtual void copy_area( GlowWind *w, int x, int y) {}
  virtual void clear_area( GlowWind *w, int ll_x, int ur_x, int ll_y, int ur_y) {}
  virtual void set_inputfocus( GlowWind *w) {}
  virtual void set_background(  GlowWind *w, glow_eDrawType drawtype, glow_tPixmap pixmap,
		       glow_tImImage image, int pixmap_width, int pixmap_height) {}
  virtual void reset_background( GlowWind *w) {}
  virtual void set_image_clip_mask( DrawWind *w, glow_tPixmap pixmap, int x, int y) {}
  virtual void reset_image_clip_mask( DrawWind *w) {}
  virtual int set_clip_rectangle( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y) {return 1;}
  virtual void reset_clip_rectangle( GlowWind *w) {}
  virtual int clip_level( GlowWind *w) {return 1;}
  virtual int draw_point( GlowWind *w, int x1, int y1, glow_eDrawType gc_type) {return 1;}
  virtual int draw_points( GlowWind *w, glow_sPointX *points, int point_num, 
	      glow_eDrawType gc_type) {return 1;}
  virtual void set_click_sensitivity( GlowWind *w, int value) {}
  virtual void draw_background( GlowWind *wind, int x, int y, int w, int h) {}
  virtual int create_buffer( GlowWind *w) {return 1;}
  virtual void delete_buffer( GlowWind *w) {}
  virtual void buffer_background( GlowWind *w) {}
  virtual int print( char *filename, double x0, double x1, int end) { return 1;};
  //virtual void set_clip( DrawWind *w, GC gc) {}
  //virtual void reset_clip( DrawWind *w, GC gc) {}
  
  virtual void set_timer( GlowCtx *gctx, int time_ms,
		  void (*callback_func)( GlowCtx *ctx), void **id) {}
  virtual void remove_timer( void *id) {}

  virtual int image_get_width( glow_tImImage image) {return 0;}
  virtual int image_get_height( glow_tImImage image) {return 0;}
  virtual int image_get_rowstride( glow_tImImage image) {return 0;}
  virtual unsigned char *image_get_data( glow_tImImage image) {return 0;}
  virtual void image_rotate( glow_tImImage *image, int to_rotation, int from_rotation) {}
  virtual void image_flip_vertical( glow_tImImage *image) {}
  virtual void image_flip_horizontal( glow_tImImage *image) {}
  virtual void image_scale( int width, int height, glow_tImImage orig_im, glow_tImImage *im, 
			    glow_tPixmap *im_pixmap, glow_tPixmap *im_mask) {}
  virtual int image_load( char *imagefile,
			  glow_tImImage *orig_im, glow_tImImage *im) {return 0;}
  virtual int image_render( int width, int height,
			    glow_tImImage orig_im, glow_tImImage *im,
			    glow_tPixmap *im_pixmap, glow_tPixmap *im_mask) {return 0;}
  virtual void image_free( glow_tImImage image) {}
  virtual void pixmap_free( glow_tPixmap pixmap) {}
  virtual void image_pixel_iter( glow_tImImage orig_image, glow_tImImage *image, 
				 void (* pixel_cb)(void *, unsigned char *), void *userdata) {}
  virtual glow_eGradient gradient_rotate( double rot, glow_eGradient gradient) { return glow_eGradient_No;}
  virtual int gradient_fill_rect( GlowWind *wind, int x, int y, int w, int h, 
				  glow_eDrawType d0, glow_eDrawType d1, glow_eDrawType d2,
				  glow_eGradient gradient) 
    {return fill_rect( wind, x, y, w, h, d0);} 
  virtual int gradient_fill_rectrounded( GlowWind *wind, int x, int y, int w, int h, 
					 int roundamount, glow_eDrawType d0, 
					 glow_eDrawType d1, glow_eDrawType d2, 
					 glow_eGradient gradient) 
    {return fill_rect( wind, x, y, w, h, d0);}
  virtual int gradient_fill_arc( GlowWind *wind, int x, int y, int w, int h, 
				 int angle1, int angle2, glow_eDrawType d0, glow_eDrawType d1, 
				 glow_eDrawType d2, glow_eGradient gradient) 
    {return fill_arc( wind, x, y, w, h, angle1, angle2, d0, 0);}
  virtual int gradient_fill_polyline( GlowWind *wind, glow_sPointX *points, int point_cnt,
				      glow_eDrawType d0, glow_eDrawType d1, glow_eDrawType d2, 
				      glow_eGradient gradient) 
    {return fill_polyline( wind, points, point_cnt, d0, 0);}
};

class DrawWind {
 public:
  DrawWind() : double_buffered(0), double_buffer_on(0), draw_buffer_only(0),
    is_nav(0) {}
  int 		type;
  int 		double_buffered;       	//!< Double buffering is configured.
  int 		double_buffer_on;      	//!< Double buffering is on.
  int 		draw_buffer_only;      	//!< Draw in double buffering buffer only.
  int		is_nav;			//!< Is navigator window.
};

#endif














