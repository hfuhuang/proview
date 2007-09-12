/* 
 * Proview   $Id: glow_growtrend.h,v 1.5 2007-09-12 08:56:37 claes Exp $
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

#ifndef glow_growtrend_h
#define glow_growtrend_h

#include "glow_growrect.h"
#include "glow_growpolyline.h"
#include "glow_tracedata.h"

/*! \file glow_growtrend.h
    \brief Contains the GrowTrend class. */
/*! \addtogroup Glow */
/*@{*/


//! Class for drawing a trend object.
/*! A GrowTrend object is an object that draws a number of trend curves, where the x-axis is the 
  time dimension, and the y-axis displayes the trend value of a parameter. New values are added as time
  passes. The curves are drawn filled or unfilled.

  The GrowTrend class contains function for drawing the object, and handle events when the 
  object is clicked on, moved etc.
*/

class GrowTrend : public GrowRect {
 public:

  GrowTrend( GrowCtx *glow_ctx, char *name, double x = 0, double y = 0, 
		double w = 0, double h = 0, 
		glow_eDrawType border_d_type = glow_eDrawType_Line, 
		int line_w = 1, 
		glow_mDisplayLevel display_lev = glow_mDisplayLevel_1,
		int fill_rect = 0, int display_border = 1, 
		glow_eDrawType fill_d_type = glow_eDrawType_Line, int nodraw = 0);

  ~GrowTrend();

  void save( ofstream& fp, glow_eSaveMode mode);

  void open( ifstream& fp);

  //! Erase the object
  void erase( GlowWind *w)
	{ erase( w, (GlowTransform *)NULL, hot, NULL);};

  void draw( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y);

  void draw( GlowWind *w, int *ll_x, int *ll_y, int *ur_x, int *ur_y);

  void set_highlight( int on);

  //! Get the object type
  /*!
    \return The type of the object.
  */
  glow_eObjectType type() { return glow_eObjectType_GrowTrend;};

  void set_trace_attr( GlowTraceData *attr);	//!< Obsolete
  void get_trace_attr( GlowTraceData **attr);	//!< Obsolete

  //! Set number of verticals and horizontal lines.
  /*!
    \param v	Number of vertical lines.
    \param h	Number of horizontal lines.
  */
  void set_lines( int v, int h) { vertical_lines = v; horizontal_lines = h;};

  double		y_max_value[TREND_MAX_CURVES];	//!< Max y values of the curves.
  double		y_min_value[TREND_MAX_CURVES];	//!< Min y values of the curves.
  double		x_max_value[TREND_MAX_CURVES];	//!< Max x values of the curves.
  double		x_min_value[TREND_MAX_CURVES];	//!< Min x values of the curves.
  int			horizontal_lines;		//!< Number of horizontal lines.
  int			vertical_lines;			//!< Number of vertical lines.
  int			fill_curve;			//!< The curves are filled.
  int			no_of_points;			//!< Number of points in each curve.
  int			curve_width;			//!< Line width of the curves.
  glow_eDrawType	curve_drawtype[TREND_MAX_CURVES]; //!< Color of the curves.
  glow_eDrawType	curve_fill_drawtype[TREND_MAX_CURVES]; //!< Fill color of the curves.
  GlowTraceData		trace;				//!< Obsolete
  GrowPolyLine		*curve[TREND_MAX_CURVES];	//!< The polyline object for the curves.
  int			curve_cnt;			//!< Number of curves.
  double		scan_time;			//!< Scantime. Time interval between two points.
  void 			*user_data;			//!< User data.
  glow_eTrendMode	mode;				//!< Type of curve.

  void draw( GlowWind *w, GlowTransform *t, int highlight, int hot, void *node, void *colornode);

  void erase( GlowWind *w, GlowTransform *t, int hot, void *node);

  void draw();

  void trace_scan();

  int trace_init();

  void trace_close();

  void add_value( double value, int idx);

  void configure_curves();

  void align( double x, double y, glow_eAlignDirection direction);

  void set_scan_time( double time);

  //! Get scantime
  /*!
    \param time		Scantime in seconds.
  */
  void get_scan_time( double *time) { *time = scan_time;};

  void set_range_y( int curve, double min, double max);

  //! Set fill for curves.
  /*!
    \param fill		Curves is drawn with fill.
  */
  void set_fill_curve( int fill) { fill_curve = fill;};

  //! Set user data.
  /*!
    \param data User data.
  */
  void set_user_data( void *data) { user_data = data;};

  //! Get user data.
  /*!
    \param data User data.
  */
  void get_user_data( void **data) { *data = user_data;};

  void set_trend_info( glow_sTrendInfo *info);

  void export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp);

  void convert( glow_eConvert version);

  void set_data( double *data[3], int data_curves, int data_points);
  int get_no_of_points() { return no_of_points;}

  void set_xy_range_x( int curve, double min, double max);
  void set_xy_range_y( int curve, double min, double max);
  void set_xy_noofcurves( int noofcurves);
  void set_xy_curve_color( int curve, glow_eDrawType curve_color,
			   glow_eDrawType fill_color);
  void set_xy_data( double *y_data, double *x_data, int curve_idx, int data_points);
};


/*@}*/
#endif


