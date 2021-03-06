/* 
 * Proview   $Id: glow_growxycurve.h,v 1.2 2008-10-31 12:51:36 claes Exp $
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

#ifndef glow_growxycurve_h
#define glow_growxycurve_h

#include "glow_growtrend.h"


/*! \file glow_growcurve.h
    \brief Contains the GrowXYCurve class. */
/*! \addtogroup Glow */
/*@{*/


//! Class for drawing xy-curves in a trend window.
/*! A GrowXYCurve object is an object that draws a number of curves specified by points
  with x and y coordinates. The curves are drawn filled or unfilled.
*/

class GrowXYCurve : public GrowTrend {
 public:

  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param name		Name (max 31 char).
    \param x		x coordinate for position.
    \param y		y coordinate for position.
    \param w		Width.
    \param h		Height.
    \param border_d_type Border color.
    \param line_w	Linewidth of border.
    \param display_lev	Displaylevel when this object is visible.
    \param fill_rect	Rectangle is filled.
    \param display_border Border is visible.
    \param fill_d_type	Fill color.
    \param nodraw	Don't draw the object now.
  */
  GrowXYCurve( GrowCtx *glow_ctx, const char *name,
                double x = 0, double y = 0, 
		double w = 0, double h = 0, 
		glow_eDrawType border_d_type = glow_eDrawType_Line, 
		int line_w = 1, 
		glow_mDisplayLevel display_lev = glow_mDisplayLevel_1,
		int fill_rect = 0, int display_border = 1, 
	        glow_eDrawType fill_d_type = glow_eDrawType_Line, 
	        int nodraw = 0);

  //! Get the object type
  /*!
    \return The type of the object.
  */
  glow_eObjectType type() { return glow_eObjectType_GrowXYCurve;};

  void save( ofstream& fp, glow_eSaveMode mode);
  void open( ifstream& fp);

  void set_xy_range_x( int curve, double min, double max);
  void set_xy_range_y( int curve, double min, double max);
  void set_xy_noofcurves( int noofcurves);
  void set_xy_curve_color( int curve, glow_eDrawType curve_color,
			   glow_eDrawType fill_color);
  void set_xy_data( double *y_data, double *x_data, int curve_idx, int data_points);
  void export_javabean( GlowTransform *t, void *node,
			glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, 
			ofstream &fp);
  int get_java_name( char *name) { strcpy( name, "jopXYCurve"); return 1;}
};


/*@}*/
#endif

