/* 
 * Proview   $Id: glow_growaxis.h,v 1.5 2007-01-04 07:57:38 claes Exp $
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

#ifndef glow_growaxis_h
#define glow_growaxis_h

#include "glow_growrect.h"

/*! \file glow_growaxis.h
    \brief Contains the GrowAxis class. */
/*! \addtogroup Glow */
/*@{*/


//! Class for drawing an axis object.
/*! A GrowAxis object is an object that displays an axis or scale.

  The GrowAxis class contains function for drawing the object, and handle events when the 
  object is clicked on, moved etc.
*/
class GrowAxis : public GrowRect {
 public:

  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param name		Name (max 31 char).
    \param x1		x coordinate for first corner.
    \param y1		y coordinate for first corner.
    \param x2		x coordinate for second corner.
    \param y2		y coordinate for second corner.
    \param border_d_type Border color.
    \param line_w	Linewidth of border.
    \param t_size	Text size.
    \param t_drawtype	Drawtype for text.
    \param nodraw	Don't draw the object now.
  */
  GrowAxis( GrowCtx *glow_ctx, char *name, double x1 = 0, double y1 = 0, 
		double x2 = 0, double y2 = 0, 
		glow_eDrawType border_d_type = glow_eDrawType_Line, 
	        int line_w = 1, int t_size = 2, 
                glow_eDrawType t_drawtype = glow_eDrawType_TextHelvetica,
		int nodraw = 0);

  //! Destructor
  /*! Remove the object from context, and erase it from the screen.
   */
  ~GrowAxis();

  //! Save the content of the object to file.
  /*!
    \param fp	Ouput file.
    \param mode	Not used.
  */
  void save( ofstream& fp, glow_eSaveMode mode);

  //! Read the content of the object from file.
  /*!
    \param fp	Input file.
  */
  void open( ifstream& fp);

  //! Erase the object
  void erase( GlowWind *w)
	{ erase( w, (GlowTransform *)NULL, hot, NULL);};

  //! Draw the objects if any part is inside the drawing area.
  /*!
    \param ll_x		Lower left x coordinate of drawing area.
    \param ll_y		Lower left y coordinate of drawing area.
    \param ur_x		Upper right x coordinate of drawing area.
    \param ur_y		Upper right y coordinate of drawing area.
  */
  void draw( GlowWind *w, int ll_x, int ll_y, int ur_x, int ur_y);

  //! Draw the objects if any part is inside the drawing area, and extends the drawing area.
  /*!
    \param ll_x		Lower left x coordinate of drawing area.
    \param ll_y		Lower left y coordinate of drawing area.
    \param ur_x		Upper right x coordinate of drawing area.
    \param ur_y		Upper right y coordinate of drawing area.

    If some part of object is inside the drawing area, and also outside the drawing area,
    the drawingarea is extended so it contains the whole objects.
  */
  void draw( GlowWind *w, int *ll_x, int *ll_y, int *ur_x, int *ur_y);

  //! Set object highlight.
  /*!
    \param on	If 1, set highlight. If 0, reset highlight.
  */
  void set_highlight( int on);

  //! Get the object type
  /*!
    \return The type of the object.
  */
  glow_eObjectType type() { return glow_eObjectType_GrowAxis;};

  int                 	text_size;		//!< Size of text.
  glow_eDrawType      	text_drawtype;		//!< Drawtype for text.
  glow_eDrawType      	text_color_drawtype;	//!< Text color.
  double		max_value;		//!< Max value for the scale.
  double		min_value;		//!< Min value for the scale.
  int			lines;			//!< Number of perpendicular lines.
  int                 	linelength;		//!< Length of perpendicular lines.
  int                 	longquotient;		//!< Quotient of lines that are a little bit longer.
  int                 	valuequotient;		//!< Quotient of lines that displays a value.
  double              	increment;		//!< Value difference between two lines.
  char                	format[20];		//!< Format of displayed values.

  //! Erase the object.
  /*!
    \param t		Transform of parent node.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
  */
  void erase( GlowWind *w, GlowTransform *t, int hot, void *node);

  //! Draw the object.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
    \param colornode	The node that controls the color of the object. Can be zero.

    The object is drawn with border, fill and shadow. If t is not zero, the current tranform is
    multiplied with the parentnodes transform, to give the appropriate coordinates for the drawing.
  */
  void draw( GlowWind *w, GlowTransform *t, int highlight, int hot, void *node, void *colornode);

  //! Redraw the area inside the objects border.
  void draw();

  //! Configure the axis
  /*! Calculate the layout for the axis, length of lines and which values that should be dislayed.
   */
  void configure();

  //! Moves object to alignment line or point.
  /*!
    \param x	x coordinate of alignment point.
    \param y	y coordinate of alignment point.
    \param direction Type of alignment.
  */
  void align( double x, double y, glow_eAlignDirection direction);

  //! Set the range for the scale value
  /*!
    \param min		Min value.
    \param max		Max value.
  */
  void set_range( double min, double max);

  //! Set size of text.
  /*!
    \param size		Text size.
  */
  void set_textsize( int size);

  //! Set text bold.
  /*!
    \param bold		Text is bold.
  */
  void set_textbold( int bold);

  //! Set parameters for the axis.
  /*!
    \param info		Info struct.
  */
  void set_axis_info( glow_sAxisInfo *info);

  //! Export the object as a javabean.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param node		Parent node. Can be zero.
    \param pass		Export pass.
    \param shape_cnt	Current index in a shape vector.
    \param node_cnt	Counter used for javabean name. Not used for this kind of object.
    \param in_nc	Member of a nodeclass. Not used for this kind of object.
    \param fp		Output file.

    The object is transformed to the current zoom factor, and GlowExportJBean is used to generate
    java code for the bean.
  */
  void export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp);

  //! Set configuration values for the axis.
  void set_conf( double max_val, double min_val, int no_of_lines, 
                   int long_quot, int value_quot, double rot, char *format);

  //! Conversion between different versions of Glow
  /*!
    \param version	Version to convert to.
  */
  void convert( glow_eConvert version);

  //! Set original text color.
  /*!
    \param drawtype	Text color.
  */
  void set_original_text_color( glow_eDrawType drawtype) 
	{ text_color_drawtype = drawtype; draw();};

  //! Format axis text.
  /*!
    \param text		Formated text.
    \param fmt		Format specification, c syntax or %1t(h:m:s), %2t (d h:m) for time formats.
    \param value	Value to format.
  */
  void format_text( char *text, char *fmt, double value);
};


/*@}*/
#endif















