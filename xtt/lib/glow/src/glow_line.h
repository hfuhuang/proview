#ifndef glow_line_h
#define glow_line_h

#include <iostream.h>
#include <fstream.h>
#include "glow.h"
#include "glow_ctx.h"
#include "glow_point.h"
#include "glow_array_elem.h"


/*! \file glow_line.h
    \brief Contains the GlowLine class. */
/*! \addtogroup Glow */
/*@{*/


//! Base class for a line.
/*! The full implementation of a line is in the GrowLine class. 
  The GlowLine class is still used by GlowCon, when drawing connections.
*/

class GlowLine : public GlowArrayElem {
 public:
  //! Noargs constructor.
  GlowLine() {};

  //! Constuctor
  /*!
    \param glow_ctx 	The glow context.
    \param x1		x coordinate for first point.
    \param y1		y coordinate for first point.
    \param x2		x coordinate for second point.
    \param y2		y coordinate for second point.
    \param d_type 	Color.
    \param line_w	Linewidth.
    \param fix_line_w	Linewidth independent of scale.
  */
  GlowLine( GlowCtx *glow_ctx, double x1 = 0, double y1 = 0, double x2 = 0, 
		double y2 = 0, glow_eDrawType d_type = glow_eDrawType_Line, 
		int line_w = 1, int fix_line_w = 0) : 
	ctx(glow_ctx), p1(glow_ctx,x1,y1), p2(glow_ctx,x2,y2),
	draw_type(d_type), line_width(line_w), fix_line_width(fix_line_w)
      {}

  friend ostream& operator<< ( ostream& o, const GlowLine l);

  //! Adjust pixel coordinates to current zoom factor.
  void zoom();

  //! Adjust pixel coordinates for navigaion window to current zoom factor.
  void nav_zoom();

  void print_zoom();		//!< Not used
  void traverse( int x, int y); //!< Not used

  //! Event handler
  /*!
    \param pos		Position of object. Should be zero.
    \param event	Current event.
    \param x		x coordinate of event.
    \param y		y coordinate of event.
    \param node		Parent node. Can be zero.
    \return		Returns 1 if the object is hit, else 0.

    Detects if the object is hit by the event.
  */
  int	event_handler( void *pos, glow_eEvent event, int x, int y, void *node);

  //! Not implemented
  void conpoint_select( void *pos, int x, int y, double *distance, 
		void **cp) {};

  //! Print postscript. Not used.
  void print( void *pos, void *node);

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

  //! Draw the object.
  /*!
    \param pos		Position of object. Should be zero.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.

    Draw the object, without borders or shadow.
  */
  void draw( void *pos, int highlight, int hot, void *node);

  void draw_border( void *pos, int highlight, int hot, void *node);

  //! Draw border and shadow of the line.
  /*!
    \param border	Draw border.
    \param shadow	Draw shadow.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.

    Draw border and shadow of the object. The border always has color black and linewidth 1 pixel.
    The shadow also always has linewith 1 pixel.
  */
  void draw_shadow( int border, int shadow, int highlight, int hot);

  //! Draw the object in the navigation window.
  /*!
    \param pos		Position of object. Should be zero.
    \param highlight	Draw with highlight colors.
    \param node		Parent node. Can be zero.

    Draw the object, without borders or shadow.
  */
  void nav_draw( void *pos, int highlight, void *node);

  //! Not used
  void draw_inverse( void *pos, int hot, void *node)
	{ erase( pos, hot, node);};

  //! Erase the object.
  /*!
    \param pos		Position of object. Should be zero.
    \param hot		Draw as hot, with larger line width.
    \param node		Parent node. Can be zero.
  */
  void erase( void *pos, int hot, void *node);

  //! Erase the object in the navigation window.
  /*!
    \param pos		Position of object. Should be zero.
    \param node		Parent node. Can be zero.
  */
  void nav_erase( void *pos, void *node);

  //! Calculate the border for a set of objects or for a parent node.
  /*!
    \param pos_x        x coordinate for position.
    \param pos_y        y coordinate for position.
    \param x_right	Right limit.
    \param x_left	Left limit.
    \param y_high	High limit.
    \param y_low	Low limit.
    \param node		Parent node. Can be zero.
    
    If the borders of the objects exceeds a limit, the limit is adjusted to the
    border of the object.
  */
  void get_borders( double pos_x, double pos_y, double *x_right, 
		double *x_left, double *y_high, double *y_low, void *node);

  //! Move the line to the specified coordinates.
  /*!
    \param pos		Position. Should be zero.
    \param x1		x coordinate of first point.
    \param y1		y coordinate of first point.
    \param x2		x coordinate of second point.
    \param y2		y coordinate of second point.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.

    Both endpoints are given new coordinates, so the direction and length of the line can 
    be entirely different.
  */
  void move( void *pos, double x1, double y1, double x2, double y2,
	int highlight, int hot);

  //! Move the line to the specified coordinates without erase.
  /*!
    \param pos		Position. Should be zero.
    \param x1		x coordinate of first point.
    \param y1		y coordinate of first point.
    \param x2		x coordinate of second point.
    \param y2		y coordinate of second point.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.

    Both endpoints are given new coordinates, so the direction and length of the line can be entirely different.
  */
  void move_noerase( void *pos, double x1, double y1, double x2, double y2,
	int highlight, int hot);

  //! Move the line.
  /*!
    \param pos		Position. Should be zero.
    \param delta_x	Moved distance in x direction.
    \param delta_y	Moved distance in y direction.
    \param highlight	Draw with highlight colors.
    \param hot		Draw as hot, with larger line width.
  */
  void shift( void *pos, double delta_x, double delta_y, int highlight, int hot);

  int get_conpoint( int num, double *x, double *y, glow_eDirection *dir) 
		{ return 0;};

  //! Get the object type
  /*!
    \return The type of the object.
  */
  glow_eObjectType type() { return glow_eObjectType_Line;};

  //! Set the color.
  /*!
    \param drawtype	Color.
  */
  void set_drawtype( glow_eDrawType drawtype) { draw_type = drawtype;};

  //! Set the linewidth.
  /*!
    \param linewidth	Linewidth in range 0 to 8. 0 gives a linewidth of 1 pixel at original zoom. 1 -> 2 pixel etc.
  */
  void set_linewidth( int linewidth) {line_width = linewidth;};

  //! Export the object as a java shape.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param node		Parent node. Can be zero.
    \param pass		Export pass.
    \param shape_cnt	Current index in a shape vector.
    \param node_cnt	Counter used for javabean name. Not used for this kind of object.
    \param in_nc	Member of a nodeclass. Not used for this kind of object.
    \param fp		Output file.

    The object is transformed to the current zoom factor, and GlowExportJBean is used to generate
    java code for the shape.
  */
  void export_javabean( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp);

  //! Export the shadow of the object as a java shape.
  /*!
    \param t		Transform of parent node. Can be zero.
    \param node		Parent node. Can be zero.
    \param pass		Export pass.
    \param shape_cnt	Current index in a shape vector.
    \param node_cnt	Counter used for javabean name. Not used for this kind of object.
    \param in_nc	Member of a nodeclass. Not used for this kind of object.
    \param fp		Output file.
    \param shadow	Draw the object with shadow.
    \param border	Draw the object with border.

    The object is transformed to the current zoom factor, and GlowExportJBean is used to generate
    java code for the shape.
  */
  void export_javabean_shadow( GlowTransform *t, void *node,
	glow_eExportPass pass, int *shape_cnt, int node_cnt, int in_nc, ofstream &fp, int shadow, int border);

  //! Conversion between different versions of Glow
  /*!
    \param version	Version to convert to.
  */
  void convert( glow_eConvert version);

  GlowCtx *ctx;    	//!< Glow context.
  GlowPoint p1;		//!< First endpoint of line.
  GlowPoint p2;		//!< Second endpoint of line.
  glow_eDrawType draw_type; //!< Line color.
  int	 line_width;	//!< Line width.
  int  fix_line_width;	//!< Line width is independent of zoom factor.
};

/*@}*/
#endif


